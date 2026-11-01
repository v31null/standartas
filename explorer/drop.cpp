#include "shell.h"

#include <thread>

namespace
{
    CLIPFORMAT Format(const wchar_t* name)
    {
        return (CLIPFORMAT)RegisterClipboardFormatW(name);
    }

    bool Has(IDataObject* data, CLIPFORMAT cf)
    {
        FORMATETC fmt = { cf, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        return data && data->QueryGetData(&fmt) == S_OK;
    }

    bool HasFiles(IDataObject* data)
    {
        return Has(data, CF_HDROP);
    }

    bool HasItems(IDataObject* data)
    {
        return Has(data, Format(CFSTR_SHELLIDLIST));
    }

    bool HasStreams(IDataObject* data)
    {
        return Has(data, Format(CFSTR_FILEDESCRIPTORW));
    }

    void Tell(IDataObject* data, const wchar_t* name, DWORD effect)
    {
        FORMATETC fmt = { Format(name), nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM med = { TYMED_HGLOBAL };
        med.hGlobal = GlobalAlloc(GHND, sizeof(DWORD));
        if (!med.hGlobal) return;
        *(DWORD*)GlobalLock(med.hGlobal) = effect;
        GlobalUnlock(med.hGlobal);
        if (FAILED(data->SetData(&fmt, &med, TRUE))) GlobalFree(med.hGlobal);
    }

    struct Source
    {
        Place place;
        PIDLIST_ABSOLUTE parent = nullptr;
        std::vector<std::wstring> names;
    };

    struct Sources
    {
        std::vector<Source> list;
        std::vector<PIDLIST_ABSOLUTE> pidls;

        ~Sources()
        {
            for (auto& s : list) CoTaskMemFree(s.parent);
            for (auto p : pidls) CoTaskMemFree(p);
        }
    };

    void Collect(IDataObject* data, Sources& out, bool& ours)
    {
        ours = false;
        FORMATETC fmt = { Format(CFSTR_SHELLIDLIST), nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM med;
        if (!data || FAILED(data->GetData(&fmt, &med))) return;
        bool all = true;
        if (const CIDA* c = (const CIDA*)GlobalLock(med.hGlobal))
        {
            PCIDLIST_ABSOLUTE parent = (PCIDLIST_ABSOLUTE)((const BYTE*)c + c->aoffset[0]);
            for (UINT i = 1; i <= c->cidl; i++)
            {
                PIDLIST_ABSOLUTE p = ILCombine(parent, (PCUIDLIST_RELATIVE)((const BYTE*)c + c->aoffset[i]));
                if (!p) continue;
                out.pidls.push_back(p);
                Place where;
                std::wstring name;
                if (FAILED(Locate(p, where, name)))
                {
                    all = false;
                    continue;
                }
                Source* s = nullptr;
                for (auto& x : out.list)
                    if (ArchiveId(x.place.path, x.place.chain) == ArchiveId(where.path, where.chain) && x.place.sub == where.sub) s = &x;
                if (!s)
                {
                    out.list.push_back({ where, ILCloneFull(parent), {} });
                    s = &out.list.back();
                }
                s->names.push_back(name);
            }
            GlobalUnlock(med.hGlobal);
        }
        ReleaseStgMedium(&med);
        ours = all && !out.list.empty();
    }

    bool SameFile(const Sources& s, const std::wstring& path)
    {
        for (const auto& x : s.list)
            if (v31np::Fold(x.place.path) != v31np::Fold(path)) return false;
        return !s.list.empty();
    }

    HRESULT AddStreams(const Place& place, IDataObject* data)
    {
        FORMATETC fd = { Format(CFSTR_FILEDESCRIPTORW), nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM med;
        HRESULT r = data->GetData(&fd, &med);
        if (FAILED(r)) return r;
        struct Got
        {
            std::wstring rel;
            bool dir = false;
            std::wstring file;
            uint64_t size = 0;
        };
        std::vector<Got> got;
        if (const FILEGROUPDESCRIPTORW* g = (const FILEGROUPDESCRIPTORW*)GlobalLock(med.hGlobal))
        {
            for (UINT i = 0; i < g->cItems; i++)
            {
                const FILEDESCRIPTORW& d = g->fgd[i];
                got.push_back({ d.cFileName, (d.dwFlags & FD_ATTRIBUTES) && (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), {}, 0 });
            }
            GlobalUnlock(med.hGlobal);
        }
        ReleaseStgMedium(&med);
        const CLIPFORMAT contents = Format(CFSTR_FILECONTENTS);
        r = S_OK;
        for (size_t i = 0; i < got.size() && SUCCEEDED(r); i++)
        {
            if (got[i].dir) continue;
            FORMATETC fc = { contents, nullptr, DVASPECT_CONTENT, (LONG)i, TYMED_ISTREAM | TYMED_HGLOBAL };
            STGMEDIUM m;
            r = data->GetData(&fc, &m);
            if (FAILED(r)) break;
            got[i].file = TempFile();
            HANDLE h = got[i].file.empty() ? INVALID_HANDLE_VALUE : CreateFileW(got[i].file.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
            if (h == INVALID_HANDLE_VALUE) r = E_FAIL;
            else
            {
                std::vector<BYTE> buf(1 << 20);
                if (m.tymed == TYMED_ISTREAM)
                {
                    for (;;)
                    {
                        ULONG n = 0;
                        HRESULT rr = m.pstm->Read(buf.data(), (ULONG)buf.size(), &n);
                        if (FAILED(rr))
                        {
                            r = rr;
                            break;
                        }
                        if (n == 0) break;
                        DWORD put = 0;
                        if (!WriteFile(h, buf.data(), n, &put, nullptr) || put != n)
                        {
                            r = E_FAIL;
                            break;
                        }
                        got[i].size += n;
                    }
                }
                else if (m.tymed == TYMED_HGLOBAL)
                {
                    const SIZE_T n = GlobalSize(m.hGlobal);
                    if (const void* p = GlobalLock(m.hGlobal))
                    {
                        DWORD put = 0;
                        if (!WriteFile(h, p, (DWORD)n, &put, nullptr) || put != n) r = E_FAIL;
                        got[i].size = n;
                        GlobalUnlock(m.hGlobal);
                    }
                }
                else r = DV_E_TYMED;
                CloseHandle(h);
            }
            ReleaseStgMedium(&m);
        }
        if (SUCCEEDED(r))
        {
            r = Mutate(place.path, [&](Edit& e)
            {
                auto a = Load(place.path, place.chain);
                if (!a) return E_ACCESSDENIED;
                const size_t dir = a->Find(place.sub);
                if (dir == v31np::None) return E_ACCESSDENIED;
                const std::vector<std::wstring> base = StoredSegments(a->index->tree, dir);
                a.reset();
                if (!e.Part(place.chain)) return E_FAIL;
                for (const auto& g : got)
                {
                    std::vector<std::wstring> segs = base;
                    size_t at = 0;
                    while (at <= g.rel.size())
                    {
                        size_t cut = g.rel.find_first_of(L"\\/", at);
                        if (cut == std::wstring::npos) cut = g.rel.size();
                        if (cut > at) segs.push_back(g.rel.substr(at, cut - at));
                        at = cut + 1;
                    }
                    if (segs.size() == base.size()) continue;
                    v31np::Row row;
                    row.key = e.Fresh(place.chain, Join(segs), Join(base), false, g.dir);
                    if (g.dir) row.kind = 'D';
                    else
                    {
                        row.kind = 'T';
                        row.file = g.file;
                        row.size = g.size;
                        e.made.push_back(g.file);
                    }
                    e.Put(place.chain, std::move(row));
                }
                return S_OK;
            });
        }
        if (FAILED(r))
            for (const auto& g : got)
                if (!g.file.empty()) DeleteFileW(g.file.c_str());
        return r;
    }

    HRESULT MoveItems(const Sources& s, const Place& place, bool move)
    {
        HRESULT r = S_OK;
        for (const auto& x : s.list)
        {
            HRESULT one = Transfer(x.place, x.names, place, move);
            if (FAILED(one)) r = one;
            Changed(x.parent);
        }
        return r;
    }

    void TransferAsync(std::vector<PIDLIST_ABSOLUTE> items, PIDLIST_ABSOLUTE dest, bool move, HWND hwnd)
    {
        std::thread([items, dest, move, hwnd]()
        {
            Lock hold;
            if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
            {
                ComPtr<IShellItemArray> from;
                ComPtr<IShellItem> to;
                ComPtr<IFileOperation> op;
                if (SUCCEEDED(SHCreateShellItemArrayFromIDLists((UINT)items.size(), (PCIDLIST_ABSOLUTE_ARRAY)items.data(), &from))
                    && SUCCEEDED(SHCreateItemFromIDList(dest, IID_PPV_ARGS(&to)))
                    && SUCCEEDED(CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&op))))
                {
                    if (hwnd) op->SetOwnerWindow(hwnd);
                    if (SUCCEEDED(move ? op->MoveItems(from.Get(), to.Get()) : op->CopyItems(from.Get(), to.Get()))) op->PerformOperations();
                }
                from.Reset();
                to.Reset();
                op.Reset();
                CoUninitialize();
            }
            for (auto p : items) CoTaskMemFree(p);
            CoTaskMemFree(dest);
        }).detach();
    }

    std::vector<PIDLIST_ABSOLUTE> Clone(const std::vector<PIDLIST_ABSOLUTE>& list)
    {
        std::vector<PIDLIST_ABSOLUTE> out;
        for (auto p : list)
            if (PIDLIST_ABSOLUTE c = ILCloneFull(p)) out.push_back(c);
        return out;
    }

    std::wstring Target(const std::vector<std::wstring>& paths)
    {
        std::wstring first = paths[0];
        while (!first.empty() && (first.back() == L'\\' || first.back() == L'/')) first.pop_back();
        size_t slash = first.find_last_of(L"\\/");
        const std::wstring dir = slash == std::wstring::npos ? L"." : first.substr(0, slash);
        std::wstring name = slash == std::wstring::npos ? first : first.substr(slash + 1);
        DWORD attrs = GetFileAttributesW(first.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY))
        {
            size_t dot = name.rfind(L'.');
            if (dot != std::wstring::npos && dot > 0) name.resize(dot);
        }
        const std::wstring base = name + L".v31np";
        std::wstring candidate = base;
        for (size_t k = 1; GetFileAttributesW((dir + L"\\" + candidate).c_str()) != INVALID_FILE_ATTRIBUTES; k++) candidate = v31np::Suffix(base, k);
        return dir + L"\\" + candidate;
    }

    enum class Kind
    {
        None,
        Files,
        Ours,
        Streams,
        Other,
    };

    Kind KindOf(IDataObject* data, bool& same, const std::wstring& path)
    {
        same = false;
        if (HasFiles(data)) return Kind::Files;
        if (!HasItems(data)) return HasStreams(data) ? Kind::Streams : Kind::None;
        Sources s;
        bool ours = false;
        Collect(data, s, ours);
        if (ours)
        {
            same = SameFile(s, path);
            return Kind::Ours;
        }
        return HasStreams(data) ? Kind::Streams : Kind::Other;
    }

    // Delete not — IFileOperation into the archive ROOT dies with 0x8007007B before we are even asked: Windows sees a
    // file and refuses to believe it is a folder. Its own zip folder gets the same treatment. Real drops and pastes
    // come through this door instead, so they work.
    class DropTarget final : public IDropTarget
    {
        REFS

    public:
        Place place;
        PIDLIST_ABSOLUTE pidl = nullptr;
        Kind kind = Kind::None;
        bool same = false;

        ~DropTarget()
        {
            CoTaskMemFree(pidl);
        }

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(DropTarget, IDropTarget),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        DWORD Pick(DWORD keys, DWORD offered)
        {
            if (kind == Kind::None) return DROPEFFECT_NONE;
            DWORD want = (keys & MK_CONTROL) ? DROPEFFECT_COPY : (keys & MK_SHIFT) ? DROPEFFECT_MOVE : same ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
            if (!(offered & want)) want = (offered & DROPEFFECT_COPY) ? DROPEFFECT_COPY : (offered & DROPEFFECT_MOVE);
            return want;
        }

        IFACEMETHODIMP DragEnter(IDataObject* data, DWORD keys, POINTL, DWORD* effect) override
        {
            kind = KindOf(data, same, place.path);
            *effect = Pick(keys, *effect);
            return S_OK;
        }

        IFACEMETHODIMP DragOver(DWORD keys, POINTL, DWORD* effect) override
        {
            *effect = Pick(keys, *effect);
            return S_OK;
        }

        IFACEMETHODIMP DragLeave() override
        {
            return S_OK;
        }

        IFACEMETHODIMP Drop(IDataObject* data, DWORD keys, POINTL, DWORD* effect) override
        {
            kind = KindOf(data, same, place.path);
            const DWORD chosen = Pick(keys, *effect);
            const bool move = chosen == DROPEFFECT_MOVE;
            *effect = DROPEFFECT_NONE;
            if (!chosen) return S_OK;
            if (kind == Kind::Files)
            {
                auto paths = DropPaths(data);
                if (paths.empty()) return S_OK;
                // Delete not — a move from the PC is an « optimized move »: we tell the source « relax, we've got it »
                // (performed effect NONE), so it keeps its files. They only leave the PC at the repack.
                if (move) Tell(data, CFSTR_PERFORMEDDROPEFFECT, DROPEFFECT_NONE);
                else *effect = DROPEFFECT_COPY;
                Place p = place;
                PIDLIST_ABSOLUTE at = pidl ? ILCloneFull(pidl) : nullptr;
                std::thread([p, paths, at, move]()
                {
                    Lock hold;
                    if (SUCCEEDED(AddPaths(p, paths, move))) Changed(at);
                    CoTaskMemFree(at);
                }).detach();
                return S_OK;
            }
            if (kind == Kind::Ours)
            {
                Sources s;
                bool ours = false;
                Collect(data, s, ours);
                if (!ours) return S_OK;
                if (move) Tell(data, CFSTR_PERFORMEDDROPEFFECT, DROPEFFECT_NONE);
                else *effect = DROPEFFECT_COPY;
                MoveItems(s, place, move);
                Changed(pidl);
                return S_OK;
            }
            if (kind == Kind::Streams)
            {
                if (FAILED(AddStreams(place, data))) return S_OK;
                Changed(pidl);
                *effect = chosen;
                if (move) Tell(data, CFSTR_PERFORMEDDROPEFFECT, DROPEFFECT_MOVE);
                return S_OK;
            }
            if (kind == Kind::Other)
            {
                Sources s;
                bool ours = false;
                Collect(data, s, ours);
                PIDLIST_ABSOLUTE at = pidl ? ILCloneFull(pidl) : nullptr;
                auto list = Clone(s.pidls);
                if (list.empty() || !at)
                {
                    for (auto p : list) CoTaskMemFree(p);
                    CoTaskMemFree(at);
                    return S_OK;
                }
                *effect = chosen;
                TransferAsync(std::move(list), at, move, nullptr);
            }
            return S_OK;
        }
    };

    class Send final : public IDropTarget, public IPersistFile
    {
        REFS

    public:
        DWORD allowed = DROPEFFECT_NONE;

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(Send, IDropTarget),
                QITABENT(Send, IPersistFile),
                QITABENTMULTI(Send, IPersist, IPersistFile),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP GetClassID(CLSID* clsid) override
        {
            *clsid = ClsidSend;
            return S_OK;
        }

        IFACEMETHODIMP IsDirty() override
        {
            return S_FALSE;
        }

        IFACEMETHODIMP Load(LPCOLESTR, DWORD) override
        {
            return S_OK;
        }

        IFACEMETHODIMP Save(LPCOLESTR, BOOL) override
        {
            return E_NOTIMPL;
        }

        IFACEMETHODIMP SaveCompleted(LPCOLESTR) override
        {
            return E_NOTIMPL;
        }

        IFACEMETHODIMP GetCurFile(LPOLESTR*) override
        {
            return E_NOTIMPL;
        }

        IFACEMETHODIMP DragEnter(IDataObject* data, DWORD, POINTL, DWORD* effect) override
        {
            allowed = DROPEFFECT_NONE;
            if (HasFiles(data)) allowed = DROPEFFECT_COPY;
            else if (HasItems(data))
            {
                Sources s;
                bool ours = false;
                Collect(data, s, ours);
                if (ours) allowed = DROPEFFECT_COPY;
            }
            *effect &= allowed;
            return S_OK;
        }

        IFACEMETHODIMP DragOver(DWORD, POINTL, DWORD* effect) override
        {
            *effect &= allowed;
            return S_OK;
        }

        IFACEMETHODIMP DragLeave() override
        {
            return S_OK;
        }

        IFACEMETHODIMP Drop(IDataObject* data, DWORD, POINTL, DWORD* effect) override
        {
            auto paths = DropPaths(data);
            if (!paths.empty())
            {
                *effect = DROPEFFECT_COPY;
                PackAsync(paths);
                return S_OK;
            }
            *effect = DROPEFFECT_NONE;
            Sources s;
            bool ours = false;
            Collect(data, s, ours);
            if (!ours) return S_OK;
            for (const auto& x : s.list)
                if (SUCCEEDED(PackNodes(x.place, x.names)))
                {
                    *effect = DROPEFFECT_COPY;
                    Changed(x.parent);
                }
            return S_OK;
        }
    };
}

bool PasteItems(IDataObject* clip, const Place& place, PCIDLIST_ABSOLUTE dest, HWND hwnd)
{
    if (!clip) return false;
    DWORD preferred = DROPEFFECT_COPY;
    FORMATETC fmt = { Format(CFSTR_PREFERREDDROPEFFECT), nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM med;
    if (SUCCEEDED(clip->GetData(&fmt, &med)))
    {
        if (const DWORD* p = (const DWORD*)GlobalLock(med.hGlobal))
        {
            preferred = *p;
            GlobalUnlock(med.hGlobal);
        }
        ReleaseStgMedium(&med);
    }
    const bool move = (preferred & DROPEFFECT_MOVE) && !(preferred & DROPEFFECT_COPY);
    bool same = false;
    const Kind kind = KindOf(clip, same, place.path);
    if (kind == Kind::Files)
    {
        HRESULT r = AddPaths(place, DropPaths(clip), move);
        if (FAILED(r)) return false;
        // Delete not — never say « paste succeeded, move » here: Windows' own clipboard object takes that as its cue to
        // shred the originals on the spot, and the repack then packs thin air. « We did it ourselves » (performed
        // effect NONE) keeps them on the PC until the repack.
        if (move) Tell(clip, CFSTR_PERFORMEDDROPEFFECT, DROPEFFECT_NONE);
        Changed(dest);
        return true;
    }
    if (kind == Kind::Ours)
    {
        Sources s;
        bool ours = false;
        Collect(clip, s, ours);
        if (!ours || FAILED(MoveItems(s, place, move))) return false;
        if (move) Tell(clip, CFSTR_PERFORMEDDROPEFFECT, DROPEFFECT_NONE);
        Changed(dest);
        return true;
    }
    if (kind == Kind::Streams)
    {
        if (FAILED(AddStreams(place, clip))) return false;
        if (move)
        {
            Tell(clip, CFSTR_PERFORMEDDROPEFFECT, DROPEFFECT_MOVE);
            Tell(clip, CFSTR_PASTESUCCEEDED, DROPEFFECT_MOVE);
        }
        Changed(dest);
        return true;
    }
    if (kind == Kind::Other && dest)
    {
        Sources s;
        bool ours = false;
        Collect(clip, s, ours);
        auto list = Clone(s.pidls);
        PIDLIST_ABSOLUTE at = ILCloneFull(dest);
        if (list.empty() || !at)
        {
            for (auto p : list) CoTaskMemFree(p);
            CoTaskMemFree(at);
            return false;
        }
        TransferAsync(std::move(list), at, move, hwnd);
        return true;
    }
    return false;
}

void PackAsync(const std::vector<std::wstring>& paths)
{
    std::thread([paths]()
    {
        Lock hold;
        const std::wstring target = Target(paths);
        if (SUCCEEDED(CreateFromPaths(target, paths))) SHChangeNotify(SHCNE_CREATE, SHCNF_PATHW | SHCNF_FLUSH, target.c_str(), nullptr);
    }).detach();
}

HRESULT CreateDrop(const Place& place, PCIDLIST_ABSOLUTE pidl, REFIID riid, void** ppv)
{
    *ppv = nullptr;
    DropTarget* d = new (std::nothrow) DropTarget();
    if (!d) return E_OUTOFMEMORY;
    d->place = place;
    d->pidl = pidl ? ILCloneFull(pidl) : nullptr;
    HRESULT r = d->QueryInterface(riid, ppv);
    d->Release();
    return r;
}

HRESULT CreateSend(REFIID riid, void** ppv)
{
    *ppv = nullptr;
    Send* s = new (std::nothrow) Send();
    if (!s) return E_OUTOFMEMORY;
    HRESULT r = s->QueryInterface(riid, ppv);
    s->Release();
    return r;
}
