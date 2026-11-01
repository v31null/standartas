#include "shell.h"

#include <algorithm>

const IID IID_Store = { 0x5b0e7a43, 0x2f61, 0x4d8c, { 0x9e, 0x0b, 0x31, 0x6a, 0xc4, 0x52, 0x87, 0xd9 } };

namespace
{
    bool BadName(const std::wstring& name)
    {
        return name.empty() || name.find_first_of(L"/\\") != std::wstring::npos;
    }

    uint64_t FileSize(const std::wstring& file)
    {
        WIN32_FILE_ATTRIBUTE_DATA d;
        if (!GetFileAttributesExW(file.c_str(), GetFileExInfoStandard, &d)) return 0;
        return ((uint64_t)d.nFileSizeHigh << 32) | d.nFileSizeLow;
    }

    void Notify(PCIDLIST_ABSOLUTE pidl)
    {
        if (pidl) SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, pidl, nullptr);
    }

    std::string Child(const std::string& prefix, const std::wstring& name)
    {
        const std::string n = v31np::Narrow(name);
        return prefix.empty() ? n : prefix + "/" + n;
    }

    class Held final : public IStream
    {
        REFS

    public:
        ComPtr<IStream> inner;
        std::wstring path;
        std::vector<std::string> chain;
        PIDLIST_ABSOLUTE pidl = nullptr;
        std::string key;
        std::wstring file;
        std::wstring name;

        ~Held()
        {
            inner.Reset();
            if (!key.empty())
            {
                const uint64_t size = FileSize(file);
                Mutate(path, [&](Edit& e)
                {
                    auto rows = e.Part(chain);
                    if (!rows) return E_FAIL;
                    for (auto& r : *rows)
                        if (r.kind == 'T' && r.key == key && r.file == file) r.size = size;
                    return S_OK;
                });
                Notify(pidl);
            }
            CoTaskMemFree(pidl);
        }

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(Held, IStream),
                QITABENT(Held, ISequentialStream),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP Read(void* pv, ULONG cb, ULONG* got) override { return inner->Read(pv, cb, got); }
        IFACEMETHODIMP Write(const void* pv, ULONG cb, ULONG* put) override { return inner->Write(pv, cb, put); }
        IFACEMETHODIMP Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER* pos) override { return inner->Seek(move, origin, pos); }
        IFACEMETHODIMP SetSize(ULARGE_INTEGER size) override { return inner->SetSize(size); }
        IFACEMETHODIMP CopyTo(IStream* to, ULARGE_INTEGER cb, ULARGE_INTEGER* read, ULARGE_INTEGER* written) override { return inner->CopyTo(to, cb, read, written); }
        IFACEMETHODIMP Commit(DWORD flags) override { return inner->Commit(flags); }
        IFACEMETHODIMP Revert() override { return inner->Revert(); }
        IFACEMETHODIMP LockRegion(ULARGE_INTEGER at, ULARGE_INTEGER cb, DWORD type) override { return inner->LockRegion(at, cb, type); }
        IFACEMETHODIMP UnlockRegion(ULARGE_INTEGER at, ULARGE_INTEGER cb, DWORD type) override { return inner->UnlockRegion(at, cb, type); }
        IFACEMETHODIMP Stat(STATSTG* st, DWORD flag) override
        {
            HRESULT r = inner->Stat(st, flag | STATFLAG_NONAME);
            if (FAILED(r) || (flag & STATFLAG_NONAME)) return r;
            return SHStrDupW(name.c_str(), &st->pwcsName);
        }

        IFACEMETHODIMP Clone(IStream**) override { return E_NOTIMPL; }
    };

    class Elements final : public IEnumSTATSTG
    {
        REFS

    public:
        std::vector<STATSTG> list;
        std::vector<std::wstring> names;
        size_t at = 0;

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(Elements, IEnumSTATSTG),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP Next(ULONG celt, STATSTG* out, ULONG* fetched) override
        {
            ULONG n = 0;
            while (n < celt && at < list.size())
            {
                out[n] = list[at];
                if (FAILED(SHStrDupW(names[at].c_str(), &out[n].pwcsName))) break;
                n++;
                at++;
            }
            if (fetched) *fetched = n;
            return n == celt ? S_OK : S_FALSE;
        }

        IFACEMETHODIMP Skip(ULONG celt) override
        {
            at = std::min<size_t>(list.size(), at + celt);
            return at < list.size() ? S_OK : S_FALSE;
        }

        IFACEMETHODIMP Reset() override
        {
            at = 0;
            return S_OK;
        }

        IFACEMETHODIMP Clone(IEnumSTATSTG** out) override
        {
            Elements* e = new (std::nothrow) Elements();
            if (!e) return E_OUTOFMEMORY;
            e->list = list;
            e->names = names;
            e->at = at;
            *out = e;
            return S_OK;
        }
    };

    HRESULT OpenHeld(const std::wstring& path, const std::vector<std::string>& chain, PCIDLIST_ABSOLUTE pidl, const std::string& key, const std::wstring& shown, IStream* prefill, IStream** out)
    {
        *out = nullptr;
        Held* h = new (std::nothrow) Held();
        if (!h) return E_OUTOFMEMORY;
        h->path = path;
        h->chain = chain;
        h->name = shown;
        h->file = TempFile();
        HRESULT r = h->file.empty() ? E_FAIL : SHCreateStreamOnFileEx(h->file.c_str(), STGM_CREATE | STGM_READWRITE | STGM_SHARE_DENY_WRITE, FILE_ATTRIBUTE_TEMPORARY, TRUE, nullptr, &h->inner);
        if (SUCCEEDED(r) && prefill)
        {
            ULARGE_INTEGER all;
            all.QuadPart = ~0ull;
            r = prefill->CopyTo(h->inner.Get(), all, nullptr, nullptr);
            LARGE_INTEGER zero{};
            if (SUCCEEDED(r)) r = h->inner->Seek(zero, STREAM_SEEK_SET, nullptr);
        }
        if (SUCCEEDED(r))
        {
            const std::wstring file = h->file;
            const uint64_t size = FileSize(file);
            r = Mutate(path, [&](Edit& e)
            {
                if (!e.Part(chain)) return E_FAIL;
                v31np::Row row;
                row.kind = 'T';
                row.key = key;
                row.size = size;
                row.file = file;
                e.Put(chain, std::move(row));
                return S_OK;
            });
        }
        if (FAILED(r))
        {
            h->inner.Reset();
            if (!h->file.empty()) DeleteFileW(h->file.c_str());
            delete h;
            return r;
        }
        h->key = key;
        h->pidl = pidl ? ILCloneFull(pidl) : nullptr;
        Notify(pidl);
        *out = h;
        return S_OK;
    }

    class Store final : public IStorage
    {
        REFS

    public:
        std::wstring path;
        std::vector<std::string> chain;
        std::vector<std::wstring> segs;
        std::wstring name;
        PIDLIST_ABSOLUTE pidl = nullptr;

        ~Store()
        {
            CoTaskMemFree(pidl);
        }

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            if (riid == IID_Store)
            {
                *ppv = this;
                AddRef();
                return S_OK;
            }
            static const QITAB qit[] = {
                QITABENT(Store, IStorage),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        std::string Prefix() const
        {
            return Join(segs);
        }

        std::shared_ptr<Archive> Open(size_t& dir) const
        {
            auto a = Load(path, chain);
            dir = v31np::None;
            if (!a) return nullptr;
            const auto& t = a->index->tree;
            size_t cur = 0;
            for (const auto& s : segs)
            {
                size_t next = v31np::None;
                for (size_t k : t.nodes[cur].kids)
                    if (t.nodes[k].dir && t.nodes[k].stored == s) next = k;
                if (next == v31np::None) return a;
                cur = next;
            }
            dir = cur;
            return a;
        }

        bool Here(Place& p) const
        {
            size_t dir;
            auto a = Open(dir);
            if (!a || dir == v31np::None) return false;
            p.path = path;
            p.chain = chain;
            p.sub.clear();
            for (size_t x = dir; x != 0 && x != v31np::None; x = a->index->tree.nodes[x].parent) p.sub.push_back(a->index->tree.nodes[x].natural);
            std::reverse(p.sub.begin(), p.sub.end());
            return true;
        }

        static size_t Find(const std::shared_ptr<Archive>& a, size_t dir, const std::wstring& name)
        {
            if (dir == v31np::None) return v31np::None;
            const auto& t = a->index->tree;
            for (size_t k : t.nodes[dir].kids)
                if (t.nodes[k].stored == name) return k;
            return a->index->Child(dir, name);
        }

        HRESULT Sub(const std::wstring& stored, const std::wstring& shown, IStorage** out) const
        {
            Store* s = new (std::nothrow) Store();
            if (!s) return E_OUTOFMEMORY;
            s->path = path;
            s->chain = chain;
            s->segs = segs;
            s->segs.push_back(stored);
            s->name = shown;
            *out = s;
            return S_OK;
        }

        IFACEMETHODIMP CreateStream(LPCOLESTR pwcsName, DWORD grfMode, DWORD, DWORD, IStream** out) override
        {
            *out = nullptr;
            if (!pwcsName || BadName(pwcsName)) return STG_E_INVALIDNAME;
            size_t dir;
            auto a = Open(dir);
            if (!a) return STG_E_FILENOTFOUND;
            if (dir == v31np::None) return STG_E_PATHNOTFOUND;
            std::wstring stored = pwcsName;
            const size_t k = Find(a, dir, pwcsName);
            if (k != v31np::None)
            {
                if (a->index->tree.nodes[k].dir || !(grfMode & STGM_CREATE)) return STG_E_FILEALREADYEXISTS;
                stored = a->index->tree.nodes[k].stored;
            }
            a.reset();
            return OpenHeld(path, chain, pidl, Child(Prefix(), stored), pwcsName, nullptr, out);
        }

        IFACEMETHODIMP OpenStream(LPCOLESTR pwcsName, void*, DWORD grfMode, DWORD, IStream** out) override
        {
            *out = nullptr;
            if (!pwcsName) return STG_E_INVALIDNAME;
            size_t dir;
            auto a = Open(dir);
            if (!a) return STG_E_FILENOTFOUND;
            const size_t k = Find(a, dir, pwcsName);
            if (k == v31np::None || a->index->tree.nodes[k].dir) return STG_E_FILENOTFOUND;
            ComPtr<IStream> s;
            HRESULT r = CreateEntryStream(a, k, &s);
            if (FAILED(r)) return r;
            if (!(grfMode & (STGM_WRITE | STGM_READWRITE)))
            {
                *out = s.Detach();
                return S_OK;
            }
            const std::string key = Child(Prefix(), a->index->tree.nodes[k].stored);
            const std::wstring shown = a->index->tree.nodes[k].natural;
            a.reset();
            return OpenHeld(path, chain, pidl, key, shown, s.Get(), out);
        }

        IFACEMETHODIMP CreateStorage(LPCOLESTR pwcsName, DWORD, DWORD, DWORD, IStorage** out) override
        {
            *out = nullptr;
            if (!pwcsName || BadName(pwcsName)) return STG_E_INVALIDNAME;
            size_t dir;
            auto a = Open(dir);
            if (!a) return STG_E_FILENOTFOUND;
            if (dir == v31np::None) return STG_E_PATHNOTFOUND;
            const size_t k = Find(a, dir, pwcsName);
            if (k != v31np::None)
            {
                if (!a->index->tree.nodes[k].dir) return STG_E_FILEALREADYEXISTS;
                return Sub(a->index->tree.nodes[k].stored, a->index->tree.nodes[k].natural, out);
            }
            a.reset();
            const std::string key = Child(Prefix(), pwcsName);
            HRESULT r = Mutate(path, [&](Edit& e)
            {
                if (!e.Part(chain)) return E_FAIL;
                v31np::Row row;
                row.kind = 'D';
                row.key = key;
                e.Put(chain, std::move(row));
                return S_OK;
            });
            if (FAILED(r)) return r;
            Notify(pidl);
            return Sub(pwcsName, pwcsName, out);
        }

        IFACEMETHODIMP OpenStorage(LPCOLESTR pwcsName, IStorage*, DWORD, SNB, DWORD, IStorage** out) override
        {
            *out = nullptr;
            if (!pwcsName) return STG_E_INVALIDNAME;
            size_t dir;
            auto a = Open(dir);
            if (!a) return STG_E_FILENOTFOUND;
            const size_t k = Find(a, dir, pwcsName);
            if (k == v31np::None || !a->index->tree.nodes[k].dir) return STG_E_FILENOTFOUND;
            return Sub(a->index->tree.nodes[k].stored, a->index->tree.nodes[k].natural, out);
        }

        IFACEMETHODIMP CopyTo(DWORD, const IID*, SNB, IStorage*) override
        {
            return E_NOTIMPL;
        }

        IFACEMETHODIMP MoveElementTo(LPCOLESTR pwcsName, IStorage* dest, LPCOLESTR newName, DWORD flags) override
        {
            if (!pwcsName || !dest || !newName || BadName(newName)) return STG_E_INVALIDNAME;
            ComPtr<IUnknown> raw;
            if (FAILED(dest->QueryInterface(IID_Store, (void**)raw.GetAddressOf()))) return STG_E_INVALIDFUNCTION;
            const Store* d = static_cast<Store*>(static_cast<void*>(raw.Get()));
            Place from;
            Place to;
            if (!Here(from) || !d->Here(to)) return STG_E_PATHNOTFOUND;
            size_t dir;
            auto a = Open(dir);
            if (!a) return STG_E_FILENOTFOUND;
            const size_t k = Find(a, dir, pwcsName);
            if (k == v31np::None) return STG_E_FILENOTFOUND;
            const std::wstring natural = a->index->tree.nodes[k].natural;
            const std::wstring stored = a->index->tree.nodes[k].stored;
            a.reset();
            HRESULT r = Transfer(from, { natural }, to, flags == STGMOVE_MOVE);
            if (FAILED(r)) return r;
            if (stored != newName)
            {
                size_t there;
                auto b = d->Open(there);
                if (!b) return STG_E_FILENOTFOUND;
                const size_t m = Find(b, there, stored);
                if (m == v31np::None) return STG_E_FILENOTFOUND;
                const std::wstring now = b->index->tree.nodes[m].natural;
                b.reset();
                r = RenameNode(to, now, newName);
                if (FAILED(r)) return r;
            }
            Notify(pidl);
            Notify(d->pidl);
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

        IFACEMETHODIMP EnumElements(DWORD, void*, DWORD, IEnumSTATSTG** out) override
        {
            *out = nullptr;
            size_t dir;
            auto a = Open(dir);
            if (!a) return STG_E_FILENOTFOUND;
            if (dir == v31np::None) return STG_E_PATHNOTFOUND;
            Elements* e = new (std::nothrow) Elements();
            if (!e) return E_OUTOFMEMORY;
            for (size_t k : a->index->tree.nodes[dir].kids)
            {
                const v31np::Node& n = a->index->tree.nodes[k];
                STATSTG st = {};
                st.type = n.dir ? STGTY_STORAGE : STGTY_STREAM;
                if (!n.dir) st.cbSize.QuadPart = a->index->entries[n.entry].size;
                e->list.push_back(st);
                e->names.push_back(n.natural);
            }
            *out = e;
            return S_OK;
        }

        IFACEMETHODIMP DestroyElement(LPCOLESTR pwcsName) override
        {
            if (!pwcsName) return STG_E_INVALIDNAME;
            Place here;
            if (!Here(here)) return STG_E_PATHNOTFOUND;
            size_t dir;
            auto a = Open(dir);
            if (!a) return STG_E_FILENOTFOUND;
            const size_t k = Find(a, dir, pwcsName);
            if (k == v31np::None) return S_OK;
            const std::wstring natural = a->index->tree.nodes[k].natural;
            a.reset();
            HRESULT r = RemoveNodes(here, { natural });
            if (SUCCEEDED(r)) Notify(pidl);
            return r;
        }

        IFACEMETHODIMP RenameElement(LPCOLESTR oldName, LPCOLESTR newName) override
        {
            if (!oldName || !newName || BadName(newName)) return STG_E_INVALIDNAME;
            Place here;
            if (!Here(here)) return STG_E_PATHNOTFOUND;
            size_t dir;
            auto a = Open(dir);
            if (!a) return STG_E_FILENOTFOUND;
            const size_t k = Find(a, dir, oldName);
            if (k == v31np::None) return STG_E_FILENOTFOUND;
            const size_t clash = Find(a, dir, newName);
            if (clash != v31np::None && clash != k) return STG_E_FILEALREADYEXISTS;
            const std::wstring natural = a->index->tree.nodes[k].natural;
            a.reset();
            HRESULT r = RenameNode(here, natural, newName);
            if (SUCCEEDED(r)) Notify(pidl);
            return r;
        }

        IFACEMETHODIMP SetElementTimes(LPCOLESTR, const FILETIME*, const FILETIME*, const FILETIME*) override
        {
            return S_OK;
        }

        IFACEMETHODIMP SetClass(REFCLSID) override
        {
            return S_OK;
        }

        IFACEMETHODIMP SetStateBits(DWORD, DWORD) override
        {
            return S_OK;
        }

        IFACEMETHODIMP Stat(STATSTG* st, DWORD flag) override
        {
            ZeroMemory(st, sizeof(*st));
            st->type = STGTY_STORAGE;
            st->grfMode = STGM_READWRITE;
            if (!(flag & STATFLAG_NONAME)) return SHStrDupW(name.c_str(), &st->pwcsName);
            return S_OK;
        }
    };
}

HRESULT CreateStore(const Place& place, PCIDLIST_ABSOLUTE pidl, IStorage** out)
{
    *out = nullptr;
    auto a = Load(place.path, place.chain);
    if (!a) return STG_E_FILENOTFOUND;
    const size_t here = a->Find(place.sub);
    if (here == v31np::None || !a->index->tree.nodes[here].dir) return STG_E_FILENOTFOUND;
    Store* s = new (std::nothrow) Store();
    if (!s) return E_OUTOFMEMORY;
    s->path = place.path;
    s->chain = place.chain;
    s->pidl = pidl ? ILCloneFull(pidl) : nullptr;
    s->segs = StoredSegments(a->index->tree, here);
    if (!place.sub.empty()) s->name = place.sub.back();
    else
    {
        const size_t cut = place.path.find_last_of(L"\\/");
        s->name = cut == std::wstring::npos ? place.path : place.path.substr(cut + 1);
    }
    *out = s;
    return S_OK;
}

HRESULT CreateWriteStream(const Place& place, PCIDLIST_ABSOLUTE pidl, const std::wstring& natural, bool fresh, IStream** out)
{
    *out = nullptr;
    auto a = Load(place.path, place.chain);
    if (!a) return STG_E_FILENOTFOUND;
    const size_t here = a->Find(place.sub);
    if (here == v31np::None) return STG_E_PATHNOTFOUND;
    const size_t k = a->index->Child(here, natural);
    if (k == v31np::None || a->index->tree.nodes[k].dir) return STG_E_FILENOTFOUND;
    ComPtr<IStream> old;
    if (!fresh)
    {
        HRESULT r = CreateEntryStream(a, k, &old);
        if (FAILED(r)) return r;
    }
    const std::string key = Join(StoredSegments(a->index->tree, k));
    a.reset();
    return OpenHeld(place.path, place.chain, pidl, key, natural, old.Get(), out);
}
