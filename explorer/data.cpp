#include "shell.h"

#include <algorithm>

namespace
{
    class EntryStream final : public IStream
    {
        REFS

    public:
        std::shared_ptr<Archive> a;
        uint64_t at = 0;
        uint64_t size = 0;
        uint64_t pos = 0;
        std::wstring name;

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
            if (n && !a->ReadAt(at + pos, pv, n))
            {
                if (pcbRead) *pcbRead = 0;
                return STG_E_READFAULT;
            }
            pos += n;
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
        size_t prefix = 0;
        std::once_flag built;
        v31np::Tree tree;
        std::vector<size_t> list;
        std::vector<std::wstring> rel;
        CLIPFORMAT desc = (CLIPFORMAT)RegisterClipboardFormatW(CFSTR_FILEDESCRIPTORW);
        CLIPFORMAT contents = (CLIPFORMAT)RegisterClipboardFormatW(CFSTR_FILECONTENTS);
        CLIPFORMAT effect = (CLIPFORMAT)RegisterClipboardFormatW(CFSTR_PREFERREDDROPEFFECT);

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
                static_cast<EntryStream*>(s)->name = tree.nodes[list[(size_t)fmt->lindex]].natural;
                med->tymed = TYMED_ISTREAM;
                med->pstm = s;
                return S_OK;
            }
            if (fmt->cfFormat == effect && (fmt->tymed & TYMED_HGLOBAL))
            {
                HGLOBAL g = GlobalAlloc(GHND, sizeof(DWORD));
                if (!g) return E_OUTOFMEMORY;
                *(DWORD*)GlobalLock(g) = DROPEFFECT_COPY;
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

        IFACEMETHODIMP SetData(FORMATETC*, STGMEDIUM*, BOOL) override
        {
            return E_NOTIMPL;
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
                if (prefix > 0) tree.Fit(base, prefix, v31np::LongPaths());
                for (size_t n : nodes) Add(n);
            });
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
    EntryStream* s = new (std::nothrow) EntryStream();
    if (!s) return E_OUTOFMEMORY;
    s->a = a;
    s->at = a->index->entries[n.entry].at;
    s->size = a->index->entries[n.entry].size;
    s->name = n.natural;
    *out = s;
    return S_OK;
}

HRESULT CreateData(const std::shared_ptr<Archive>& a, size_t base, const std::vector<size_t>& nodes, size_t prefix, REFIID riid, void** ppv)
{
    *ppv = nullptr;
    Data* d = new (std::nothrow) Data();
    if (!d) return E_OUTOFMEMORY;
    d->a = a;
    d->base = base;
    d->nodes = nodes;
    d->prefix = prefix;
    HRESULT r = d->QueryInterface(riid, ppv);
    d->Release();
    return r;
}

HRESULT ExtractTo(const std::wstring& file, const std::wstring& dest, HWND)
{
    auto a = Load(file, {});
    if (!a) return HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
    int made = SHCreateDirectoryExW(nullptr, dest.c_str(), nullptr);
    if (made != ERROR_SUCCESS && made != ERROR_ALREADY_EXISTS && made != ERROR_FILE_EXISTS) return HRESULT_FROM_WIN32(made);
    if (a->index->tree.nodes[0].kids.empty()) return S_OK;
    ComPtr<IDataObject> data;
    HRESULT r = CreateData(a, 0, a->index->tree.nodes[0].kids, dest.size() + 1, IID_PPV_ARGS(&data));
    if (FAILED(r)) return r;
    ComPtr<IShellItem> to;
    r = SHCreateItemFromParsingName(dest.c_str(), nullptr, IID_PPV_ARGS(&to));
    if (FAILED(r)) return r;
    ComPtr<IDropTarget> target;
    r = to->BindToHandler(nullptr, BHID_SFUIObject, IID_PPV_ARGS(&target));
    if (FAILED(r)) return r;
    DWORD effect = DROPEFFECT_COPY;
    POINTL pt = { 0, 0 };
    r = target->DragEnter(data.Get(), MK_LBUTTON, pt, &effect);
    if (FAILED(r)) return r;
    if (!(effect & DROPEFFECT_COPY))
    {
        target->DragLeave();
        return E_FAIL;
    }
    effect = DROPEFFECT_COPY;
    return target->Drop(data.Get(), MK_LBUTTON, pt, &effect);
}
