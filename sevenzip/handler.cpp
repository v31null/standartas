#include "7zip/CPP/Common/MyInitGuid.h"
#include "7zip/CPP/Common/MyCom.h"
#include "7zip/CPP/7zip/Archive/IArchive.h"
#include "7zip/CPP/7zip/PropID.h"

#include "../lib/order.h"
#include "../lib/shadow.h"
#include "../lib/tree.h"
#include "../lib/v31np.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <new>
#include <string>
#include <vector>

namespace
{
    const GUID ClassId = { 0x6d3b83f6, 0x80ca, 0x4c28, { 0xa8, 0xad, 0x4a, 0xa7, 0x59, 0x4d, 0x3d, 0xd9 } };
    const unsigned char Signature[] = { 'V', '3', '1', 'N', 'S', 'P', 'A', 'C', 'K', 'F', 'O', 'R', 'M', 'A', 'T', 0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3, 0x13, 0x19, 0x8A, 0x2E };
    const wchar_t FormatName[] = L"v31np";
    constexpr UInt32 Block = 1u << 20;
    constexpr UInt32 NoIndex = (UInt32)(Int32)-1;

    const PROPID ItemProps[] = { kpidPath, kpidIsDir, kpidSize };
    const VARTYPE ItemTypes[] = { VT_BSTR, VT_BOOL, VT_UI8 };

    HRESULT SetString(PROPVARIANT* v, const std::wstring& s)
    {
        v->bstrVal = SysAllocStringLen(s.data(), (UINT)s.size());
        if (!v->bstrVal) return E_OUTOFMEMORY;
        v->vt = VT_BSTR;
        return S_OK;
    }

    HRESULT SetBytes(PROPVARIANT* v, const void* p, UINT n)
    {
        v->bstrVal = SysAllocStringByteLen((const char*)p, n);
        if (!v->bstrVal) return E_OUTOFMEMORY;
        v->vt = VT_BSTR;
        return S_OK;
    }

    void SetBool(PROPVARIANT* v, bool b)
    {
        v->vt = VT_BOOL;
        v->boolVal = b ? VARIANT_TRUE : VARIANT_FALSE;
    }

    void SetU64(PROPVARIANT* v, UInt64 x)
    {
        v->vt = VT_UI8;
        v->uhVal.QuadPart = x;
    }

    HRESULT ReadFull(ISequentialInStream* s, void* buf, UInt32 len)
    {
        Byte* p = (Byte*)buf;
        while (len > 0)
        {
            UInt32 got = 0;
            RINOK(s->Read(p, len, &got))
            if (got == 0) return S_FALSE;
            p += got;
            len -= got;
        }
        return S_OK;
    }

    HRESULT WriteFull(ISequentialOutStream* s, const void* buf, UInt32 len)
    {
        const Byte* p = (const Byte*)buf;
        while (len > 0)
        {
            UInt32 put = 0;
            RINOK(s->Write(p, len, &put))
            if (put == 0) return E_FAIL;
            p += put;
            len -= put;
        }
        return S_OK;
    }

    struct Link
    {
        char kind = 'A';
        UInt64 raw = 0;
        std::wstring file;
        bool pending = false;
    };

    using Raw = std::function<bool(UInt64 pos, void* buf, UInt32 len)>;

    bool ReadFileAt(const std::wstring& file, UInt64 pos, void* buf, UInt32 len)
    {
        HANDLE h = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) return false;
        Byte* p = (Byte*)buf;
        bool ok = true;
        while (ok && len > 0)
        {
            OVERLAPPED o{};
            o.Offset = (DWORD)pos;
            o.OffsetHigh = (DWORD)(pos >> 32);
            DWORD got = 0;
            ok = ReadFile(h, p, len, &got, &o) && got > 0;
            p += got;
            pos += got;
            len -= got;
        }
        CloseHandle(h);
        return ok;
    }

    bool CurrentSize(const std::wstring& file, UInt64& size)
    {
        WIN32_FILE_ATTRIBUTE_DATA d;
        if (!GetFileAttributesExW(file.c_str(), GetFileExInfoStandard, &d) || (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) return false;
        size = ((UInt64)d.nFileSizeHigh << 32) | d.nFileSizeLow;
        return true;
    }

    std::wstring TempPath()
    {
        wchar_t dir[MAX_PATH + 1];
        wchar_t name[MAX_PATH + 1];
        if (!GetTempPathW(MAX_PATH, dir) || !GetTempFileNameW(dir, L"v31", 0, name)) return {};
        return name;
    }

    v31np::Item Pulled(const std::string& key, UInt64 size, Raw raw)
    {
        auto pos = std::make_shared<UInt64>(0);
        return { key, size, [raw, pos](void* buf, uint32_t len)
        {
            if (!raw(*pos, buf, len)) return false;
            *pos += len;
            return true;
        } };
    }

    bool Synth(const v31np::Shadow& s, const v31np::Chain& chain, const Raw& raw, const v31np::Push& push)
    {
        auto it = s.parts.find(chain);
        if (it == s.parts.end()) return false;
        std::vector<v31np::Item> items;
        std::vector<std::wstring> temps;
        bool ok = true;
        for (const auto& r : it->second)
        {
            if (!ok) break;
            if (!v31np::Stored(r)) continue;
            Raw own;
            if (r.kind == 'A')
            {
                const UInt64 at = r.at;
                own = [raw, at](UInt64 pos, void* buf, UInt32 len) { return raw(at + pos, buf, len); };
            }
            else
            {
                const std::wstring f = r.file;
                own = [f](UInt64 pos, void* buf, UInt32 len) { return ReadFileAt(f, pos, buf, len); };
            }
            v31np::Chain deeper = chain;
            deeper.push_back(r.key);
            if (s.parts.count(deeper))
            {
                const std::wstring t = TempPath();
                HANDLE h = t.empty() ? INVALID_HANDLE_VALUE : CreateFileW(t.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
                if (h == INVALID_HANDLE_VALUE)
                {
                    ok = false;
                    break;
                }
                temps.push_back(t);
                UInt64 size = 0;
                ok = Synth(s, deeper, own, [h, &size](const void* b, uint32_t n)
                {
                    DWORD put = 0;
                    size += n;
                    return WriteFile(h, b, n, &put, nullptr) && put == n;
                });
                CloseHandle(h);
                items.push_back(Pulled(r.key, size, [t](UInt64 pos, void* buf, UInt32 len) { return ReadFileAt(t, pos, buf, len); }));
                continue;
            }
            UInt64 size = r.size;
            if (r.kind != 'A' && !CurrentSize(r.file, size)) continue;
            items.push_back(Pulled(r.key, size, own));
        }
        if (ok) ok = v31np::Write(items, push);
        items.clear();
        for (const auto& t : temps) DeleteFileW(t.c_str());
        return ok;
    }

    std::string KeyOf(const std::wstring& path)
    {
        std::wstring k = path;
        std::replace(k.begin(), k.end(), L'\\', L'/');
        return v31np::Narrow(k);
    }

    Z7_CLASS_IMP_COM_3(Handler, IInArchive, IArchiveGetRawProps, IOutArchive)

        CMyComPtr<IInStream> stream;
        UInt64 total = 0;
        std::vector<v31np::Entry> entries;
        std::vector<Link> links;
        std::vector<std::string> extra;
        v31np::Shadow shadow;
        bool journal = false;
        v31np::Tree tree;
        std::vector<size_t> order;
        std::vector<UInt32> slot;
        bool extracting = false;

        const v31np::Node* NodeOf(UInt32 index) const
        {
            return index < order.size() ? &tree.nodes[order[index]] : nullptr;
        }

        void Arrange()
        {
            order.clear();
            slot.assign(tree.nodes.size(), NoIndex);
            std::function<void(size_t)> walk = [&](size_t d)
            {
                std::vector<size_t> kids = tree.nodes[d].kids;
                std::stable_sort(kids.begin(), kids.end(), [&](size_t x, size_t y)
                {
                    if (tree.nodes[x].dir != tree.nodes[y].dir) return tree.nodes[x].dir;
                    return v31np::Order(tree.nodes[x].stored, tree.nodes[y].stored) < 0;
                });
                for (size_t k : kids)
                {
                    slot[k] = (UInt32)order.size();
                    order.push_back(k);
                    if (tree.nodes[k].dir) walk(k);
                }
            };
            walk(0);
        }

        bool ReadAt(UInt64 pos, void* buf, UInt32 len)
        {
            if (!stream || stream->Seek((Int64)pos, STREAM_SEEK_SET, nullptr) != S_OK) return false;
            return ReadFull(stream, buf, len) == S_OK;
        }

        bool ReadEntry(size_t i, UInt64 pos, void* buf, UInt32 len)
        {
            const Link& l = links[i];
            if (pos > l.raw || len > l.raw - pos) return false;
            if (l.kind == 'A') return ReadAt(entries[i].at + pos, buf, len);
            return ReadFileAt(l.file, pos, buf, len);
        }

        void Journal(IArchiveOpenCallback* cb, UInt64 size)
        {
            journal = false;
            shadow = v31np::Shadow{};
            if (!cb) return;
            CMyComPtr<IArchiveOpenVolumeCallback> vol;
            if (cb->QueryInterface(IID_IArchiveOpenVolumeCallback, (void**)&vol) != S_OK || !vol) return;
            PROPVARIANT name;
            PropVariantInit(&name);
            PROPVARIANT time;
            PropVariantInit(&time);
            std::wstring file;
            UInt64 stamp = 0;
            bool have = false;
            if (vol->GetProperty(kpidName, &name) == S_OK && name.vt == VT_BSTR && vol->GetProperty(kpidMTime, &time) == S_OK && time.vt == VT_FILETIME)
            {
                file.assign(name.bstrVal, SysStringLen(name.bstrVal));
                stamp = ((UInt64)time.filetime.dwHighDateTime << 32) | time.filetime.dwLowDateTime;
                have = true;
            }
            PropVariantClear(&name);
            PropVariantClear(&time);
            if (!have) return;
            const size_t cut = file.find_last_of(L"\\/");
            if (cut != std::wstring::npos) file = file.substr(cut + 1);
            const std::wstring jn = v31np::JournalName(file);
            Text plain = Load(vol, jn);
            Text dot = Load(vol, L"." + jn);
            const Text* use = &plain;
            if (plain.found && dot.found && !(plain.read && dot.read && plain.text == dot.text))
            {
                v31np::Shadow probe;
                const bool rp = plain.read && v31np::ReadJournal(plain.text, probe);
                const bool rd = dot.read && v31np::ReadJournal(dot.text, probe);
                if (rp != rd) use = rd ? &dot : &plain;
                else if (dot.time > plain.time) use = &dot;
            }
            else if (!plain.found) use = &dot;
            if (!use->read) return;
            v31np::Shadow s;
            if (!v31np::ReadJournal(use->text, s) || s.size != size || s.stamp != stamp) return;
            shadow = std::move(s);
            journal = true;
        }

        struct Text
        {
            bool found = false;
            bool read = false;
            UInt64 time = 0;
            std::string text;
        };

        static Text Load(IArchiveOpenVolumeCallback* vol, const std::wstring& name)
        {
            Text t;
            CMyComPtr<IInStream> js;
            if (vol->GetStream(name.c_str(), &js) != S_OK || !js) return t;
            t.found = true;
            CMyComPtr<IStreamGetProps> props;
            if (js->QueryInterface(IID_IStreamGetProps, (void**)&props) == S_OK && props)
            {
                FILETIME m{};
                if (props->GetProps(nullptr, nullptr, nullptr, &m, nullptr) == S_OK) t.time = ((UInt64)m.dwHighDateTime << 32) | m.dwLowDateTime;
            }
            UInt64 len = 0;
            if (js->Seek(0, STREAM_SEEK_END, &len) != S_OK || len >= (1u << 31) || js->Seek(0, STREAM_SEEK_SET, nullptr) != S_OK) return t;
            t.text.assign((size_t)len, '\0');
            if (len && ReadFull(js, t.text.data(), (UInt32)len) != S_OK) return t;
            t.read = true;
            return t;
        }

        HRESULT DoOpen(IInStream* in, IArchiveOpenCallback* cb);
        HRESULT DoExtract(const UInt32* indices, UInt32 numItems, Int32 testMode, IArchiveExtractCallback* cb);
        HRESULT DoUpdate(ISequentialOutStream* out, UInt32 numItems, IArchiveUpdateCallback* cb);
    };

    HRESULT Handler::DoOpen(IInStream* in, IArchiveOpenCallback* cb)
    {
        UInt64 size = 0;
        RINOK(in->Seek(0, STREAM_SEEK_END, &size))
        stream = in;
        std::vector<v31np::Entry> parsed;
        if (!v31np::Parse([this](uint64_t pos, void* buf, uint32_t len) { return ReadAt(pos, buf, len); }, size, parsed))
        {
            stream.Release();
            return S_FALSE;
        }
        total = size;
        Journal(cb, size);
        links.clear();
        extra.clear();
        auto root = journal ? shadow.parts.find({}) : shadow.parts.end();
        if (journal && root != shadow.parts.end())
        {
            entries.clear();
            for (const auto& r : root->second)
            {
                if (r.kind == 'D') extra.push_back(r.key);
                if (!v31np::Stored(r)) continue;
                entries.push_back({ r.key, r.kind == 'A' ? r.at : 0, v31np::Logical(shadow, {}, r) });
                links.push_back({ r.kind, r.size, r.file, shadow.parts.count({ r.key }) > 0 });
            }
        }
        else
        {
            entries = std::move(parsed);
            links.resize(entries.size());
            for (size_t i = 0; i < entries.size(); i++)
            {
                links[i].raw = entries[i].size;
                links[i].pending = journal && shadow.parts.count({ entries[i].key }) > 0;
                if (links[i].pending) entries[i].size = v31np::Logical(shadow, {}, { 'A', entries[i].key, entries[i].size, entries[i].at, {} });
            }
        }
        tree.Build(entries, extra);
        Arrange();
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::Open(IInStream* in, const UInt64*, IArchiveOpenCallback* cb))
    {
        Close();
        try
        {
            return DoOpen(in, cb);
        }
        catch (...)
        {
            Close();
            return E_OUTOFMEMORY;
        }
    }

    Z7_COM7F_IMF(Handler::Close())
    {
        stream.Release();
        entries.clear();
        links.clear();
        extra.clear();
        shadow = v31np::Shadow{};
        journal = false;
        tree.nodes.clear();
        order.clear();
        slot.clear();
        total = 0;
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::GetNumberOfItems(UInt32* numItems))
    {
        *numItems = (UInt32)order.size();
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::GetProperty(UInt32 index, PROPID propID, PROPVARIANT* value))
    {
        const v31np::Node* n = NodeOf(index);
        if (!n) return E_INVALIDARG;
        try
        {
            switch (propID)
            {
            case kpidPath:
                return SetString(value, tree.Path(order[index], extracting, L'\\'));
            case kpidName:
                return SetString(value, extracting ? n->natural : n->stored);
            case kpidIsDir:
                SetBool(value, n->dir);
                break;
            case kpidSize:
            case kpidPackSize:
                if (!n->dir) SetU64(value, entries[n->entry].size);
                break;
            }
        }
        catch (...)
        {
            return E_OUTOFMEMORY;
        }
        return S_OK;
    }

    HRESULT Handler::DoExtract(const UInt32* indices, UInt32 numItems, Int32 testMode, IArchiveExtractCallback* cb)
    {
        const bool all = numItems == NoIndex;
        const UInt32 count = all ? (UInt32)order.size() : numItems;
        UInt64 sum = 0;
        for (UInt32 i = 0; i < count; i++)
        {
            const v31np::Node* n = NodeOf(all ? i : indices[i]);
            if (!n) return E_INVALIDARG;
            if (!n->dir) sum += entries[n->entry].size;
        }
        RINOK(cb->SetTotal(testMode ? total : sum))
        UInt64 done = 0;
        Int32 tested = NArchive::NExtract::NOperationResult::kOK;
        if (testMode && count > 0)
        {
            HRESULT hr = S_OK;
            const bool sound = v31np::Verify([&](uint64_t pos, void* b, uint32_t len)
            {
                if (!ReadAt(pos, b, len)) return false;
                done = pos + len;
                hr = cb->SetCompleted(&done);
                return hr == S_OK;
            }, total);
            RINOK(hr)
            if (!sound) tested = NArchive::NExtract::NOperationResult::kCRCError;
        }
        std::vector<Byte> buf;
        const Int32 ask = testMode ? NArchive::NExtract::NAskMode::kTest : NArchive::NExtract::NAskMode::kExtract;
        for (UInt32 i = 0; i < count; i++)
        {
            const UInt32 index = all ? i : indices[i];
            const v31np::Node* n = NodeOf(index);
            RINOK(cb->SetCompleted(&done))
            CMyComPtr<ISequentialOutStream> out;
            RINOK(cb->GetStream(index, &out, ask))
            if (!testMode && !out) continue;
            RINOK(cb->PrepareOperation(ask))
            Int32 result = NArchive::NExtract::NOperationResult::kOK;
            if (!n->dir && testMode) result = tested;
            else if (!n->dir)
            {
                const size_t at = n->entry;
                const Link& l = links[at];
                if (l.pending)
                {
                    HRESULT failure = S_OK;
                    const bool ok = Synth(shadow, { entries[at].key }, [this, at](UInt64 pos, void* b, UInt32 len) { return ReadEntry(at, pos, b, len); }, [&](const void* b, uint32_t len)
                    {
                        HRESULT r = out ? WriteFull(out, b, len) : S_OK;
                        if (r == S_OK)
                        {
                            done += len;
                            r = cb->SetCompleted(&done);
                        }
                        if (r != S_OK) failure = r;
                        return r == S_OK;
                    });
                    RINOK(failure)
                    if (!ok) result = NArchive::NExtract::NOperationResult::kUnexpectedEnd;
                }
                else
                {
                    if (l.raw > 0 && buf.empty()) buf.resize(Block);
                    UInt64 pos = 0;
                    for (UInt64 left = l.raw; left > 0;)
                    {
                        const UInt32 len = (UInt32)std::min<UInt64>(Block, left);
                        if (!ReadEntry(at, pos, buf.data(), len))
                        {
                            result = NArchive::NExtract::NOperationResult::kUnexpectedEnd;
                            break;
                        }
                        if (out) RINOK(WriteFull(out, buf.data(), len))
                        pos += len;
                        left -= len;
                        done += len;
                        RINOK(cb->SetCompleted(&done))
                    }
                }
            }
            out.Release();
            RINOK(cb->SetOperationResult(result))
        }
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::Extract(const UInt32* indices, UInt32 numItems, Int32 testMode, IArchiveExtractCallback* cb))
    {
        extracting = true;
        HRESULT r;
        try
        {
            r = DoExtract(indices, numItems, testMode, cb);
        }
        catch (...)
        {
            r = E_OUTOFMEMORY;
        }
        extracting = false;
        return r;
    }

    Z7_COM7F_IMF(Handler::GetArchiveProperty(PROPID propID, PROPVARIANT* value))
    {
        switch (propID)
        {
        case kpidIsTree:
            SetBool(value, true);
            break;
        case kpidPhySize:
            SetU64(value, total);
            break;
        }
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::GetNumberOfProperties(UInt32* numProps))
    {
        *numProps = (UInt32)(sizeof(ItemProps) / sizeof(ItemProps[0]));
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::GetPropertyInfo(UInt32 index, BSTR* name, PROPID* propID, VARTYPE* varType))
    {
        if (index >= sizeof(ItemProps) / sizeof(ItemProps[0])) return E_INVALIDARG;
        *name = nullptr;
        *propID = ItemProps[index];
        *varType = ItemTypes[index];
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::GetNumberOfArchiveProperties(UInt32* numProps))
    {
        *numProps = 0;
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::GetArchivePropertyInfo(UInt32, BSTR*, PROPID*, VARTYPE*))
    {
        return E_INVALIDARG;
    }

    Z7_COM7F_IMF(Handler::GetParent(UInt32 index, UInt32* parent, UInt32* parentType))
    {
        *parentType = NParentType::kDir;
        *parent = NoIndex;
        const v31np::Node* n = NodeOf(index);
        if (n && n->parent != 0) *parent = slot[n->parent];
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::GetRawProp(UInt32 index, PROPID propID, const void** data, UInt32* dataSize, UInt32* propType))
    {
        *data = nullptr;
        *dataSize = 0;
        *propType = 0;
        const v31np::Node* n = NodeOf(index);
        if (!n || propID != kpidName) return S_OK;
        const std::wstring& s = extracting ? n->natural : n->stored;
        *data = s.c_str();
        *dataSize = (UInt32)((s.size() + 1) * sizeof(wchar_t));
        *propType = NPropDataType::kUtf16z;
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::GetNumRawProps(UInt32* numProps))
    {
        *numProps = 0;
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::GetRawPropInfo(UInt32, BSTR* name, PROPID* propID))
    {
        *name = nullptr;
        *propID = kpidNoProperty;
        return E_INVALIDARG;
    }

    Z7_COM7F_IMF(Handler::GetFileTimeType(UInt32* type))
    {
        *type = (UInt32)NFileTimeType::kWindows;
        return S_OK;
    }

    HRESULT Handler::DoUpdate(ISequentialOutStream* out, UInt32 numItems, IArchiveUpdateCallback* cb)
    {
        struct Plan
        {
            std::string key;
            UInt64 size = 0;
            UInt64 at = 0;
            bool fresh = false;
            UInt32 index = 0;
        };
        std::vector<Plan> plans;
        std::vector<std::string> dirs;
        UInt64 sum = 0;
        for (UInt32 i = 0; i < numItems; i++)
        {
            Int32 newData = 0;
            Int32 newProps = 0;
            UInt32 indexInArchive = NoIndex;
            RINOK(cb->GetUpdateItemInfo(i, &newData, &newProps, &indexInArchive))
            const v31np::Node* old = indexInArchive != NoIndex ? NodeOf(indexInArchive) : nullptr;
            bool isDir = false;
            std::wstring path;
            if (newProps)
            {
                PROPVARIANT p;
                PropVariantInit(&p);
                RINOK(cb->GetProperty(i, kpidIsDir, &p))
                if (p.vt == VT_BOOL) isDir = p.boolVal != VARIANT_FALSE;
                PropVariantClear(&p);
                RINOK(cb->GetProperty(i, kpidPath, &p))
                if (p.vt != VT_BSTR)
                {
                    PropVariantClear(&p);
                    return E_INVALIDARG;
                }
                path.assign(p.bstrVal, SysStringLen(p.bstrVal));
                PropVariantClear(&p);
            }
            else
            {
                if (!old) return E_INVALIDARG;
                isDir = old->dir;
            }
            if (isDir)
            {
                if (newProps) dirs.push_back(KeyOf(path));
                continue;
            }
            Plan plan;
            plan.index = i;
            if (newData)
            {
                PROPVARIANT p;
                PropVariantInit(&p);
                RINOK(cb->GetProperty(i, kpidSize, &p))
                if (p.vt != VT_UI8) return E_INVALIDARG;
                plan.size = p.uhVal.QuadPart;
                plan.fresh = true;
            }
            else
            {
                if (!old || old->dir) return E_INVALIDARG;
                plan.size = entries[old->entry].size;
                plan.at = entries[old->entry].at;
            }
            plan.key = newProps ? KeyOf(path) : entries[old->entry].key;
            sum += plan.size;
            plans.push_back(std::move(plan));
        }

        for (const auto& d : dirs)
        {
            const std::string prefix = d + "/";
            bool used = false;
            for (const auto& p : plans)
                if (p.key.compare(0, prefix.size(), prefix) == 0) used = true;
            if (!used) return E_INVALIDARG;
        }

        RINOK(cb->SetTotal(sum))
        UInt64 done = 0;
        HRESULT failure = S_OK;
        std::vector<v31np::Item> items;
        items.reserve(plans.size());
        for (const auto& plan : plans)
        {
            v31np::Item item;
            item.key = plan.key;
            item.size = plan.size;
            if (plan.fresh)
            {
                if (plan.size == 0)
                {
                    CMyComPtr<ISequentialInStream> in;
                    RINOK(cb->GetStream(plan.index, &in))
                    in.Release();
                    RINOK(cb->SetOperationResult(NArchive::NUpdate::NOperationResult::kOK))
                }
                auto in = std::make_shared<CMyComPtr<ISequentialInStream>>();
                auto left = std::make_shared<UInt64>(plan.size);
                const UInt32 index = plan.index;
                item.pull = [cb, in, left, index, &failure](void* buf, uint32_t len)
                {
                    if (!*in)
                    {
                        CMyComPtr<ISequentialInStream>& slot = *in;
                        HRESULT r = cb->GetStream(index, &slot);
                        if (r != S_OK || !*in)
                        {
                            failure = r == S_OK ? E_FAIL : r;
                            return false;
                        }
                    }
                    HRESULT r = ReadFull(*in, buf, len);
                    if (r != S_OK)
                    {
                        failure = r == S_FALSE ? E_FAIL : r;
                        return false;
                    }
                    *left -= len;
                    if (*left == 0)
                    {
                        in->Release();
                        r = cb->SetOperationResult(NArchive::NUpdate::NOperationResult::kOK);
                        if (r != S_OK)
                        {
                            failure = r;
                            return false;
                        }
                    }
                    return true;
                };
            }
            else
            {
                auto pos = std::make_shared<UInt64>(plan.at);
                item.pull = [this, pos](void* buf, uint32_t len)
                {
                    if (!ReadAt(*pos, buf, len)) return false;
                    *pos += len;
                    return true;
                };
            }
            items.push_back(std::move(item));
        }

        const bool ok = v31np::Write(items, [out, cb, &done, &failure](const void* buf, uint32_t len)
        {
            HRESULT r = WriteFull(out, buf, len);
            if (r == S_OK)
            {
                done += len;
                r = cb->SetCompleted(&done);
            }
            if (r != S_OK)
            {
                failure = r;
                return false;
            }
            return true;
        });
        if (ok) return S_OK;
        return failure != S_OK ? failure : E_FAIL;
    }

    Z7_COM7F_IMF(Handler::UpdateItems(ISequentialOutStream* out, UInt32 numItems, IArchiveUpdateCallback* cb))
    {
        // Delete not — 7-Zip knows one way to edit: rewrite the whole archive, every time, with feeling. That cannot go
        // into the shadow, so edits are refused while a journal exists. Reading pending changes is fine.
        if (journal) return E_NOTIMPL;
        try
        {
            return DoUpdate(out, numItems, cb);
        }
        catch (...)
        {
            return E_OUTOFMEMORY;
        }
    }
}

STDAPI CreateObject(const GUID* clsid, const GUID* iid, void** outObject)
{
    *outObject = nullptr;
    if (*clsid != ClassId) return CLASS_E_CLASSNOTAVAILABLE;
    Handler* h = new (std::nothrow) Handler();
    if (!h) return E_OUTOFMEMORY;
    IInArchive* unknown = h;
    unknown->AddRef();
    HRESULT r = unknown->QueryInterface(*iid, outObject);
    unknown->Release();
    return r;
}

STDAPI GetNumberOfFormats(UInt32* numFormats)
{
    *numFormats = 1;
    return S_OK;
}

STDAPI GetHandlerProperty2(UInt32 formatIndex, PROPID propID, PROPVARIANT* value)
{
    if (formatIndex != 0) return E_INVALIDARG;
    switch (propID)
    {
    case NArchive::NHandlerPropID::kName:
        return SetString(value, FormatName);
    case NArchive::NHandlerPropID::kClassID:
        return SetBytes(value, &ClassId, sizeof(ClassId));
    case NArchive::NHandlerPropID::kExtension:
        return SetString(value, FormatName);
    case NArchive::NHandlerPropID::kUpdate:
        SetBool(value, true);
        break;
    case NArchive::NHandlerPropID::kSignature:
        return SetBytes(value, Signature, sizeof(Signature));
    }
    return S_OK;
}

STDAPI GetHandlerProperty(PROPID propID, PROPVARIANT* value)
{
    return GetHandlerProperty2(0, propID, value);
}
