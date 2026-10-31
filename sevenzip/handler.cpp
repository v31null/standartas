#include "7zip/CPP/Common/MyInitGuid.h"
#include "7zip/CPP/Common/MyCom.h"
#include "7zip/CPP/7zip/Archive/IArchive.h"
#include "7zip/CPP/7zip/PropID.h"

#include "../lib/order.h"
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

        HRESULT DoOpen(IInStream* in);
        HRESULT DoExtract(const UInt32* indices, UInt32 numItems, Int32 testMode, IArchiveExtractCallback* cb);
        HRESULT DoUpdate(ISequentialOutStream* out, UInt32 numItems, IArchiveUpdateCallback* cb);
    };

    HRESULT Handler::DoOpen(IInStream* in)
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
        entries = std::move(parsed);
        total = size;
        tree.Build(entries);
        Arrange();
        return S_OK;
    }

    Z7_COM7F_IMF(Handler::Open(IInStream* in, const UInt64*, IArchiveOpenCallback*))
    {
        Close();
        try
        {
            return DoOpen(in);
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
                const v31np::Entry& e = entries[n->entry];
                if (e.size > 0 && buf.empty()) buf.resize(Block);
                UInt64 pos = e.at;
                for (UInt64 left = e.size; left > 0;)
                {
                    const UInt32 len = (UInt32)std::min<UInt64>(Block, left);
                    if (!ReadAt(pos, buf.data(), len))
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
