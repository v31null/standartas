#include "shell.h"

#include <algorithm>
#include <functional>
#include <map>

namespace
{
    struct Reads
    {
        std::mutex guard;
        std::vector<bool> done;

        void Mark(size_t slot)
        {
            std::lock_guard<std::mutex> g(guard);
            if (slot < done.size()) done[slot] = true;
        }
    };

    class EntryStream final : public IStream
    {
        REFS

    public:
        std::shared_ptr<Archive> a;
        size_t entry = 0;
        HANDLE own = INVALID_HANDLE_VALUE;
        uint64_t size = 0;
        uint64_t pos = 0;
        std::wstring name;
        std::shared_ptr<Reads> reads;
        size_t slot = 0;

        ~EntryStream()
        {
            if (own != INVALID_HANDLE_VALUE) CloseHandle(own);
        }

        bool Fetch(uint64_t from, void* pv, ULONG n)
        {
            if (own == INVALID_HANDLE_VALUE) return a->ReadEntry(entry, from, pv, n);
            BYTE* p = (BYTE*)pv;
            while (n > 0)
            {
                OVERLAPPED o{};
                o.Offset = (DWORD)from;
                o.OffsetHigh = (DWORD)(from >> 32);
                DWORD got = 0;
                if (!ReadFile(own, p, n, &got, &o) || got == 0) return false;
                p += got;
                from += got;
                n -= got;
            }
            return true;
        }

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(EntryStream, IStream),
                QITABENT(EntryStream, ISequentialStream),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead) override
        {
            ULONG n = (ULONG)std::min<uint64_t>(cb, size - std::min(pos, size));
            if (n && !Fetch(pos, pv, n))
            {
                if (pcbRead) *pcbRead = 0;
                return STG_E_READFAULT;
            }
            pos += n;
            if (reads && pos >= size) reads->Mark(slot);
            if (pcbRead) *pcbRead = n;
            return n < cb ? S_FALSE : S_OK;
        }

        IFACEMETHODIMP Write(const void*, ULONG, ULONG*) override
        {
            return STG_E_ACCESSDENIED;
        }

        IFACEMETHODIMP Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER* newPos) override
        {
            int64_t from = origin == STREAM_SEEK_SET ? 0 : origin == STREAM_SEEK_CUR ? (int64_t)pos : origin == STREAM_SEEK_END ? (int64_t)size : -1;
            if (from < 0) return STG_E_INVALIDFUNCTION;
            int64_t to = from + move.QuadPart;
            if (to < 0) return STG_E_INVALIDFUNCTION;
            pos = (uint64_t)to;
            if (newPos) newPos->QuadPart = pos;
            return S_OK;
        }

        IFACEMETHODIMP SetSize(ULARGE_INTEGER) override
        {
            return STG_E_ACCESSDENIED;
        }

        IFACEMETHODIMP CopyTo(IStream* target, ULARGE_INTEGER cb, ULARGE_INTEGER* read, ULARGE_INTEGER* written) override
        {
            std::vector<BYTE> buf(1 << 20);
            uint64_t left = cb.QuadPart;
            uint64_t moved = 0;
            while (left > 0)
            {
                ULONG got = 0;
                HRESULT r = Read(buf.data(), (ULONG)std::min<uint64_t>(buf.size(), left), &got);
                if (FAILED(r)) return r;
                if (got == 0) break;
                ULONG put = 0;
                r = target->Write(buf.data(), got, &put);
                if (FAILED(r)) return r;
                moved += got;
                left -= got;
            }
            if (read) read->QuadPart = moved;
            if (written) written->QuadPart = moved;
            return S_OK;
        }

        IFACEMETHODIMP Commit(DWORD) override
        {
            return S_OK;
        }

        IFACEMETHODIMP Revert() override
        {
            return S_OK;
        }

        IFACEMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override
        {
            return STG_E_INVALIDFUNCTION;
        }

        IFACEMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override
        {
            return STG_E_INVALIDFUNCTION;
        }

        IFACEMETHODIMP Stat(STATSTG* st, DWORD flags) override
        {
            ZeroMemory(st, sizeof(*st));
            st->type = STGTY_STREAM;
            st->cbSize.QuadPart = size;
            st->grfMode = STGM_READ;
            if (!(flags & STATFLAG_NONAME)) return SHStrDupW(name.c_str(), &st->pwcsName);
            return S_OK;
        }

        IFACEMETHODIMP Clone(IStream**) override
        {
            return E_NOTIMPL;
        }
    };

    class Data final : public IDataObject
    {
        REFS

    public:
        std::shared_ptr<Archive> a;
        size_t base = 0;
        std::vector<size_t> nodes;
        Place place;
        PIDLIST_ABSOLUTE root = nullptr;
        std::once_flag built;
        v31np::Tree tree;
        std::vector<size_t> list;
        std::vector<std::wstring> rel;
        std::shared_ptr<Reads> reads = std::make_shared<Reads>();
        std::atomic<bool> moved{ false };
        CLIPFORMAT desc = (CLIPFORMAT)RegisterClipboardFormatW(CFSTR_FILEDESCRIPTORW);
        CLIPFORMAT contents = (CLIPFORMAT)RegisterClipboardFormatW(CFSTR_FILECONTENTS);
        CLIPFORMAT effect = (CLIPFORMAT)RegisterClipboardFormatW(CFSTR_PREFERREDDROPEFFECT);
        CLIPFORMAT performed = (CLIPFORMAT)RegisterClipboardFormatW(CFSTR_PERFORMEDDROPEFFECT);
        DWORD preferred = DROPEFFECT_COPY;

        ~Data()
        {
            CoTaskMemFree(root);
        }

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(Data, IDataObject),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP GetData(FORMATETC* fmt, STGMEDIUM* med) override
        {
            ZeroMemory(med, sizeof(*med));
            if ((fmt->cfFormat == desc && (fmt->tymed & TYMED_HGLOBAL)) || (fmt->cfFormat == contents && (fmt->tymed & TYMED_ISTREAM))) Build();
            if (fmt->cfFormat == desc && (fmt->tymed & TYMED_HGLOBAL))
            {
                const size_t bytes = sizeof(FILEGROUPDESCRIPTORW) + (list.empty() ? 0 : (list.size() - 1) * sizeof(FILEDESCRIPTORW));
                HGLOBAL g = GlobalAlloc(GHND, bytes);
                if (!g) return E_OUTOFMEMORY;
                FILEGROUPDESCRIPTORW* fg = (FILEGROUPDESCRIPTORW*)GlobalLock(g);
                fg->cItems = (UINT)list.size();
                for (size_t i = 0; i < list.size(); i++)
                {
                    FILEDESCRIPTORW& fd = fg->fgd[i];
                    const v31np::Node& n = tree.nodes[list[i]];
                    fd.dwFlags = (DWORD)(FD_ATTRIBUTES | FD_PROGRESSUI | FD_UNICODE);
                    fd.dwFileAttributes = n.dir ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
                    if (!n.dir)
                    {
                        const uint64_t size = a->index->entries[n.entry].size;
                        fd.dwFlags |= FD_FILESIZE;
                        fd.nFileSizeLow = (DWORD)size;
                        fd.nFileSizeHigh = (DWORD)(size >> 32);
                    }
                    wcsncpy_s(fd.cFileName, rel[i].c_str(), _TRUNCATE);
                }
                GlobalUnlock(g);
                med->tymed = TYMED_HGLOBAL;
                med->hGlobal = g;
                return S_OK;
            }
            if (fmt->cfFormat == contents && (fmt->tymed & TYMED_ISTREAM))
            {
                if (fmt->lindex < 0 || (size_t)fmt->lindex >= list.size() || tree.nodes[list[(size_t)fmt->lindex]].dir) return DV_E_LINDEX;
                IStream* s = nullptr;
                HRESULT r = CreateEntryStream(a, list[(size_t)fmt->lindex], &s);
                if (FAILED(r)) return r;
                EntryStream* es = static_cast<EntryStream*>(s);
                es->name = tree.nodes[list[(size_t)fmt->lindex]].natural;
                es->reads = reads;
                es->slot = (size_t)fmt->lindex;
                if (es->size == 0) reads->Mark(es->slot);
                med->tymed = TYMED_ISTREAM;
                med->pstm = s;
                return S_OK;
            }
            if (fmt->cfFormat == effect && (fmt->tymed & TYMED_HGLOBAL))
            {
                HGLOBAL g = GlobalAlloc(GHND, sizeof(DWORD));
                if (!g) return E_OUTOFMEMORY;
                *(DWORD*)GlobalLock(g) = preferred;
                GlobalUnlock(g);
                med->tymed = TYMED_HGLOBAL;
                med->hGlobal = g;
                return S_OK;
            }
            return DV_E_FORMATETC;
        }

        IFACEMETHODIMP GetDataHere(FORMATETC*, STGMEDIUM*) override
        {
            return E_NOTIMPL;
        }

        IFACEMETHODIMP QueryGetData(FORMATETC* fmt) override
        {
            if (fmt->cfFormat == desc && (fmt->tymed & TYMED_HGLOBAL)) return S_OK;
            if (fmt->cfFormat == contents && (fmt->tymed & TYMED_ISTREAM)) return S_OK;
            if (fmt->cfFormat == effect && (fmt->tymed & TYMED_HGLOBAL)) return S_OK;
            return DV_E_FORMATETC;
        }

        IFACEMETHODIMP GetCanonicalFormatEtc(FORMATETC*, FORMATETC* out) override
        {
            out->ptd = nullptr;
            return E_NOTIMPL;
        }

        IFACEMETHODIMP SetData(FORMATETC* fmt, STGMEDIUM* med, BOOL release) override
        {
            if (fmt->cfFormat == performed && med->tymed == TYMED_HGLOBAL)
            {
                DWORD v = DROPEFFECT_NONE;
                if (const DWORD* p = (const DWORD*)GlobalLock(med->hGlobal))
                {
                    v = *p;
                    GlobalUnlock(med->hGlobal);
                }
                if (v == DROPEFFECT_MOVE && !moved.exchange(true)) Harvest();
                return E_NOTIMPL;
            }
            if (fmt->cfFormat != effect || med->tymed != TYMED_HGLOBAL) return E_NOTIMPL;
            if (const DWORD* p = (const DWORD*)GlobalLock(med->hGlobal))
            {
                preferred = *p;
                GlobalUnlock(med->hGlobal);
            }
            if (release) ReleaseStgMedium(med);
            return S_OK;
        }

        IFACEMETHODIMP EnumFormatEtc(DWORD dir, IEnumFORMATETC** out) override
        {
            if (dir != DATADIR_GET) return E_NOTIMPL;
            FORMATETC fmts[] = {
                { desc, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
                { contents, nullptr, DVASPECT_CONTENT, -1, TYMED_ISTREAM },
                { effect, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            };
            return SHCreateStdEnumFmtEtc(ARRAYSIZE(fmts), fmts, out);
        }

        IFACEMETHODIMP DAdvise(FORMATETC*, DWORD, IAdviseSink*, DWORD*) override
        {
            return OLE_E_ADVISENOTSUPPORTED;
        }

        IFACEMETHODIMP DUnadvise(DWORD) override
        {
            return OLE_E_ADVISENOTSUPPORTED;
        }

        IFACEMETHODIMP EnumDAdvise(IEnumSTATDATA**) override
        {
            return OLE_E_ADVISENOTSUPPORTED;
        }

        void Build()
        {
            std::call_once(built, [this]
            {
                tree = a->index->tree;
                tree.Fit(base, 0, false);
                for (size_t n : nodes) Add(n);
                std::lock_guard<std::mutex> g(reads->guard);
                reads->done.assign(list.size(), false);
            });
        }

        void Harvest()
        {
            Build();
            std::vector<bool> done;
            {
                std::lock_guard<std::mutex> g(reads->guard);
                done = reads->done;
            }
            const v31np::Tree& t = a->index->tree;
            std::map<size_t, size_t> slot;
            for (size_t i = 0; i < list.size(); i++) slot[list[i]] = i;
            std::map<size_t, bool> known;
            std::function<bool(size_t)> full = [&](size_t x) -> bool
            {
                auto k = known.find(x);
                if (k != known.end()) return k->second;
                bool all = true;
                if (!t.nodes[x].dir)
                {
                    auto s = slot.find(x);
                    all = s != slot.end() && done[s->second];
                }
                else
                    for (size_t c : t.nodes[x].kids) all = full(c) && all;
                known[x] = all;
                return all;
            };
            std::map<std::vector<std::wstring>, std::vector<std::wstring>> plan;
            std::function<void(size_t, std::vector<std::wstring>)> take = [&](size_t x, std::vector<std::wstring> sub)
            {
                if (full(x))
                {
                    plan[sub].push_back(t.nodes[x].natural);
                    return;
                }
                if (!t.nodes[x].dir) return;
                sub.push_back(t.nodes[x].natural);
                for (size_t c : t.nodes[x].kids) take(c, sub);
            };
            for (size_t n : nodes) take(n, place.sub);
            for (const auto& [sub, names] : plan)
            {
                Place p = place;
                p.sub = sub;
                RemoveNodes(p, names);
            }
            if (!plan.empty() && root) Changed(root);
        }

        void Add(size_t node)
        {
            list.push_back(node);
            std::wstring r;
            for (size_t x = node; x != base && x != v31np::None; x = tree.nodes[x].parent)
                r = r.empty() ? tree.nodes[x].natural : tree.nodes[x].natural + L"\\" + r;
            rel.push_back(r);
            for (size_t k : tree.nodes[node].kids) Add(k);
        }
    };
}

HRESULT CreateEntryStream(const std::shared_ptr<Archive>& a, size_t node, IStream** out)
{
    *out = nullptr;
    const v31np::Node& n = a->index->tree.nodes[node];
    if (n.dir) return E_INVALIDARG;
    if (a->index->links[n.entry].pending)
    {
        std::wstring file;
        uint64_t size = 0;
        HRESULT r = Materialise(a, n.entry, file, size);
        if (FAILED(r)) return r;
        return CreateFileStream(file, true, n.natural, out);
    }
    EntryStream* s = new (std::nothrow) EntryStream();
    if (!s) return E_OUTOFMEMORY;
    s->a = a;
    s->entry = n.entry;
    s->size = a->index->links[n.entry].raw;
    s->name = n.natural;
    *out = s;
    return S_OK;
}

HRESULT CreateFileStream(const std::wstring& file, bool temporary, const std::wstring& name, IStream** out)
{
    *out = nullptr;
    HANDLE h = CreateFileW(file.c_str(), GENERIC_READ | (temporary ? DELETE : 0), FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, temporary ? FILE_FLAG_DELETE_ON_CLOSE : FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());
    LARGE_INTEGER li;
    if (!GetFileSizeEx(h, &li))
    {
        CloseHandle(h);
        return E_FAIL;
    }
    EntryStream* s = new (std::nothrow) EntryStream();
    if (!s)
    {
        CloseHandle(h);
        return E_OUTOFMEMORY;
    }
    s->own = h;
    s->size = (uint64_t)li.QuadPart;
    s->name = name;
    *out = s;
    return S_OK;
}

HRESULT CreateData(const std::shared_ptr<Archive>& a, size_t base, const std::vector<size_t>& nodes, const Place& place, PCIDLIST_ABSOLUTE root, REFIID riid, void** ppv)
{
    *ppv = nullptr;
    Data* d = new (std::nothrow) Data();
    if (!d) return E_OUTOFMEMORY;
    d->a = a;
    d->base = base;
    d->nodes = nodes;
    d->place = place;
    d->root = root ? ILCloneFull(root) : nullptr;
    HRESULT r = d->QueryInterface(riid, ppv);
    d->Release();
    return r;
}

HRESULT ExtractTo(const std::wstring& file, const std::wstring& dest, HWND hwnd)
{
    auto a = Load(file, {});
    if (!a) return HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
    const DWORD at = GetFileAttributesW(dest.c_str());
    if (at == INVALID_FILE_ATTRIBUTES || !(at & FILE_ATTRIBUTE_DIRECTORY))
    {
        int made = SHCreateDirectoryExW(nullptr, dest.c_str(), nullptr);
        if (made != ERROR_SUCCESS && made != ERROR_ALREADY_EXISTS) return HRESULT_FROM_WIN32(made);
    }
    const auto kids = a->index->tree.nodes[0].kids;
    if (kids.empty()) return S_OK;
    PIDLIST_ABSOLUTE root = nullptr;
    HRESULT r = SHParseDisplayName(file.c_str(), nullptr, &root, 0, nullptr);
    if (FAILED(r)) return r;
    std::vector<PIDLIST_ABSOLUTE> items;
    for (size_t k : kids)
    {
        const v31np::Node& n = a->index->tree.nodes[k];
        PITEMID_CHILD id = MakeId(n, n.dir ? 0 : a->index->entries[n.entry].size);
        if (!id) continue;
        if (PIDLIST_ABSOLUTE full = ILCombine(root, id)) items.push_back(full);
        CoTaskMemFree(id);
    }
    CoTaskMemFree(root);
    a.reset();
    ComPtr<IShellItemArray> from;
    ComPtr<IShellItem> to;
    ComPtr<IFileOperation> op;
    r = items.empty() ? E_FAIL : SHCreateShellItemArrayFromIDLists((UINT)items.size(), (PCIDLIST_ABSOLUTE_ARRAY)items.data(), &from);
    if (SUCCEEDED(r)) r = SHCreateItemFromParsingName(dest.c_str(), nullptr, IID_PPV_ARGS(&to));
    if (SUCCEEDED(r)) r = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&op));
    if (SUCCEEDED(r) && hwnd) op->SetOwnerWindow(hwnd);
    if (SUCCEEDED(r)) r = op->CopyItems(from.Get(), to.Get());
    if (SUCCEEDED(r)) r = op->PerformOperations();
    for (auto p : items) CoTaskMemFree(p);
    return r;
}
