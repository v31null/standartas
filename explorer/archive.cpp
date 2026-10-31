#include "shell.h"

#include <algorithm>
#include <unordered_set>

namespace
{
    std::mutex cacheGuard;
    std::map<std::wstring, std::weak_ptr<Archive>> cache;
    struct Known
    {
        uint64_t size = 0;
        uint64_t stamp = 0;
        std::shared_ptr<const Index> index;
    };
    std::map<std::wstring, Known> known;
    constexpr DWORD Magic = 0x706E3133;

    uint64_t Join64(DWORD hi, DWORD lo)
    {
        return ((uint64_t)hi << 32) | lo;
    }

    std::shared_ptr<const Index> Indexed(const std::wstring& key, uint64_t size, uint64_t stamp, const Archive* raw)
    {
        {
            std::lock_guard<std::mutex> g(cacheGuard);
            auto it = known.find(key);
            if (it != known.end() && it->second.size == size && it->second.stamp == stamp) return it->second.index;
        }
        auto x = std::make_shared<Index>();
        if (!v31np::Parse([raw](uint64_t pos, void* buf, uint32_t len) { return raw->ReadAt(pos, buf, len); }, raw->total, x->entries)) return nullptr;
        x->tree.Build(x->entries);
        x->lookup.resize(x->tree.nodes.size());
        for (size_t d = 0; d < x->tree.nodes.size(); d++)
            for (size_t k : x->tree.nodes[d].kids) x->lookup[d].emplace(v31np::Fold(x->tree.nodes[k].natural), k);
        x->blank = std::make_unique<std::atomic<bool>[]>(x->entries.size());
        std::lock_guard<std::mutex> g(cacheGuard);
        known[key] = { size, stamp, x };
        return x;
    }

    std::shared_ptr<Archive> OpenFile(const std::wstring& path)
    {
        WIN32_FILE_ATTRIBUTE_DATA d;
        if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &d) || (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) return nullptr;
        const uint64_t size = Join64(d.nFileSizeHigh, d.nFileSizeLow);
        const uint64_t time = Join64(d.ftLastWriteTime.dwHighDateTime, d.ftLastWriteTime.dwLowDateTime);
        const std::wstring key = v31np::Fold(path);
        {
            std::lock_guard<std::mutex> g(cacheGuard);
            auto it = cache.find(key);
            if (it != cache.end())
                if (auto a = it->second.lock())
                    if (a->total == size && a->stamp == time) return a;
        }
        auto a = std::make_shared<Archive>();
        a->path = path;
        a->file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (a->file == INVALID_HANDLE_VALUE) return nullptr;
        LARGE_INTEGER li;
        FILETIME ft;
        if (!GetFileSizeEx(a->file, &li) || !GetFileTime(a->file, nullptr, nullptr, &ft)) return nullptr;
        a->total = (uint64_t)li.QuadPart;
        a->stamp = Join64(ft.dwHighDateTime, ft.dwLowDateTime);
        a->index = Indexed(key, a->total, a->stamp, a.get());
        if (!a->index) return nullptr;
        std::lock_guard<std::mutex> g(cacheGuard);
        cache[key] = a;
        return a;
    }

    struct Handle
    {
        HANDLE h = INVALID_HANDLE_VALUE;
        ~Handle()
        {
            if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
        }
    };

    v31np::Item FromFile(const std::string& key, const std::wstring& file, uint64_t size)
    {
        auto s = std::make_shared<Handle>();
        return { key, size, [s, file](void* buf, uint32_t len)
        {
            if (s->h == INVALID_HANDLE_VALUE)
            {
                s->h = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
                if (s->h == INVALID_HANDLE_VALUE) return false;
            }
            BYTE* p = (BYTE*)buf;
            while (len > 0)
            {
                DWORD got = 0;
                if (!ReadFile(s->h, p, len, &got, nullptr) || got == 0) return false;
                p += got;
                len -= got;
            }
            return true;
        } };
    }

    std::wstring NameOf(const std::wstring& path)
    {
        std::wstring p = path;
        while (!p.empty() && (p.back() == L'\\' || p.back() == L'/')) p.pop_back();
        size_t cut = p.find_last_of(L"\\/");
        return cut == std::wstring::npos ? p : p.substr(cut + 1);
    }

    void Walk(const std::wstring& dir, std::vector<std::wstring> segs, std::vector<v31np::Item>& out)
    {
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileExW((dir + L"\\*").c_str(), FindExInfoBasic, &fd, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
        if (h == INVALID_HANDLE_VALUE) return;
        do
        {
            std::wstring name = fd.cFileName;
            if (name == L"." || name == L"..") continue;
            std::wstring full = dir + L"\\" + name;
            segs.push_back(name);
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) Walk(full, segs, out);
            }
            else out.push_back(FromFile(Join(segs), full, Join64(fd.nFileSizeHigh, fd.nFileSizeLow)));
            segs.pop_back();
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }

    void Gather(const std::vector<std::wstring>& base, const std::vector<std::wstring>& paths, std::vector<v31np::Item>& out)
    {
        for (const auto& p : paths)
        {
            WIN32_FILE_ATTRIBUTE_DATA d;
            if (!GetFileAttributesExW(p.c_str(), GetFileExInfoStandard, &d)) continue;
            std::vector<std::wstring> segs = base;
            segs.push_back(NameOf(p));
            if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!(d.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) Walk(p, segs, out);
            }
            else out.push_back(FromFile(Join(segs), p, Join64(d.nFileSizeHigh, d.nFileSizeLow)));
        }
    }

    std::shared_ptr<Archive> Writable(const Place& place, size_t& dir)
    {
        auto a = Load(place.path, place.chain);
        if (!a) return nullptr;
        dir = a->Find(place.sub);
        if (dir == v31np::None || !a->index->tree.nodes[dir].dir) return nullptr;
        return a;
    }

}

size_t Index::Child(size_t dir, const std::wstring& natural) const
{
    auto it = lookup[dir].find(v31np::Fold(natural));
    return it == lookup[dir].end() ? v31np::None : it->second;
}

Archive::~Archive()
{
    if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
}

bool Archive::ReadAt(uint64_t pos, void* buf, uint32_t len) const
{
    if (parent) return pos <= total && len <= total - pos && parent->ReadAt(base + pos, buf, len);
    BYTE* p = (BYTE*)buf;
    while (len > 0)
    {
        OVERLAPPED o{};
        o.Offset = (DWORD)pos;
        o.OffsetHigh = (DWORD)(pos >> 32);
        DWORD got = 0;
        if (!ReadFile(file, p, len, &got, &o) || got == 0) return false;
        p += got;
        pos += got;
        len -= got;
    }
    return true;
}

size_t Archive::Find(const std::vector<std::wstring>& sub) const
{
    size_t cur = 0;
    for (const auto& name : sub)
    {
        cur = index->Child(cur, name);
        if (cur == v31np::None) return v31np::None;
    }
    return cur;
}

std::shared_ptr<Archive> Load(const std::wstring& path, const std::vector<std::string>& chain)
{
    auto a = OpenFile(path);
    const auto root = a;
    std::wstring id = v31np::Fold(path);
    for (const auto& key : chain)
    {
        if (!a) return nullptr;
        id += L'\0';
        id += v31np::Wide(key);
        std::lock_guard<std::mutex> g(a->guard);
        auto it = a->nested.find(key);
        if (it != a->nested.end())
        {
            if (auto n = it->second.lock())
            {
                a = n;
                continue;
            }
        }
        auto e = std::find_if(a->index->entries.begin(), a->index->entries.end(), [&](const v31np::Entry& x) { return x.key == key; });
        if (e == a->index->entries.end()) return nullptr;
        auto n = std::make_shared<Archive>();
        n->path = path;
        n->parent = a;
        n->key = key;
        n->base = e->at;
        n->total = e->size;
        n->index = Indexed(id, root->total, root->stamp, n.get());
        if (!n->index) return nullptr;
        a->nested[key] = n;
        a = n;
    }
    return a;
}

HRESULT Commit(const std::shared_ptr<Archive>& a, const std::vector<v31np::Item>& items)
{
    if (!a) return E_ACCESSDENIED;
    wchar_t tag[48];
    swprintf_s(tag, L".%08lx%08llx%p.tmp", GetCurrentProcessId(), (unsigned long long)GetTickCount64(), (void*)a.get());
    const std::wstring tmp = a->path + tag;
    if (a->parent)
    {
        HANDLE h = CreateFileW(tmp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, nullptr);
        if (h == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());
        uint64_t size = 0;
        bool ok = v31np::Write(items, [h, &size](const void* b, uint32_t n)
        {
            DWORD put = 0;
            size += n;
            return WriteFile(h, b, n, &put, nullptr) && put == n;
        });
        CloseHandle(h);
        HRESULT r = E_FAIL;
        if (ok)
        {
            const auto& up = a->parent;
            std::vector<v31np::Item> outer;
            for (size_t i = 0; i < up->index->entries.size(); i++)
                outer.push_back(up->index->entries[i].key == a->key ? FromFile(a->key, tmp, size) : Keep(up, i, up->index->entries[i].key));
            r = Commit(up, outer);
        }
        DeleteFileW(tmp.c_str());
        return r;
    }
    HANDLE h = CreateFileW(tmp.c_str(), GENERIC_WRITE | DELETE, FILE_SHARE_DELETE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());
    bool ok = v31np::Write(items, [h](const void* b, uint32_t n)
    {
        DWORD put = 0;
        return WriteFile(h, b, n, &put, nullptr) && put == n;
    });
    ok = FlushFileBuffers(h) && ok;
    if (!ok)
    {
        CloseHandle(h);
        DeleteFileW(tmp.c_str());
        return E_FAIL;
    }
    const size_t bytes = sizeof(FILE_RENAME_INFO) + a->path.size() * sizeof(wchar_t);
    std::vector<BYTE> buf(bytes, 0);
    FILE_RENAME_INFO* info = (FILE_RENAME_INFO*)buf.data();
    info->Flags = FILE_RENAME_FLAG_REPLACE_IF_EXISTS | FILE_RENAME_FLAG_POSIX_SEMANTICS;
    info->FileNameLength = (DWORD)(a->path.size() * sizeof(wchar_t));
    memcpy(info->FileName, a->path.c_str(), info->FileNameLength);
    const bool moved = SetFileInformationByHandle(h, FileRenameInfoEx, info, (DWORD)bytes) != FALSE;
    CloseHandle(h);
    if (moved) return S_OK;
    if (a->file != INVALID_HANDLE_VALUE)
    {
        CloseHandle(a->file);
        a->file = INVALID_HANDLE_VALUE;
    }
    if (!MoveFileExW(tmp.c_str(), a->path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        HRESULT r = HRESULT_FROM_WIN32(GetLastError());
        DeleteFileW(tmp.c_str());
        return r;
    }
    return S_OK;
}

v31np::Item Keep(const std::shared_ptr<Archive>& a, size_t entry, const std::string& key)
{
    const v31np::Entry& e = a->index->entries[entry];
    auto pos = std::make_shared<uint64_t>(e.at);
    return { key, e.size, [a, pos](void* buf, uint32_t len)
    {
        if (!a->ReadAt(*pos, buf, len)) return false;
        *pos += len;
        return true;
    } };
}

std::vector<std::wstring> StoredSegments(const v31np::Tree& t, size_t node)
{
    std::vector<std::wstring> out;
    for (size_t x = node; x != 0 && x != v31np::None; x = t.nodes[x].parent) out.push_back(t.nodes[x].stored);
    std::reverse(out.begin(), out.end());
    return out;
}

uint64_t CacheId(const std::shared_ptr<Archive>& a, size_t entry)
{
    uint64_t h = 14695981039346656037ull;
    auto mix = [&h](const void* p, size_t n)
    {
        for (size_t i = 0; i < n; i++)
        {
            h ^= ((const BYTE*)p)[i];
            h *= 1099511628211ull;
        }
    };
    const std::string& key = a->index->entries[entry].key;
    mix(key.data(), key.size() + 1);
    const Archive* r = a.get();
    for (; r->parent; r = r->parent.get()) mix(r->key.data(), r->key.size() + 1);
    const std::wstring path = v31np::Fold(r->path);
    mix(path.data(), path.size() * sizeof(wchar_t));
    mix(&r->total, sizeof(r->total));
    mix(&r->stamp, sizeof(r->stamp));
    return h;
}

std::string Join(const std::vector<std::wstring>& segs)
{
    std::wstring r;
    for (size_t i = 0; i < segs.size(); i++)
    {
        if (i) r += L'/';
        r += segs[i];
    }
    return v31np::Narrow(r);
}

HRESULT AddPaths(const Place& place, const std::vector<std::wstring>& paths)
{
    size_t dir;
    auto a = Writable(place, dir);
    if (!a) return E_ACCESSDENIED;
    std::vector<v31np::Item> adds;
    Gather(StoredSegments(a->index->tree, dir), paths, adds);
    if (adds.empty()) return E_INVALIDARG;
    std::unordered_set<std::string> replaced;
    for (const auto& it : adds) replaced.insert(it.key);
    std::vector<v31np::Item> items;
    for (size_t i = 0; i < a->index->entries.size(); i++)
        if (!replaced.count(a->index->entries[i].key)) items.push_back(Keep(a, i, a->index->entries[i].key));
    for (auto& it : adds) items.push_back(std::move(it));
    return Commit(a, items);
}

HRESULT RemoveNodes(const Place& place, const std::vector<std::wstring>& names)
{
    size_t dir;
    auto a = Writable(place, dir);
    if (!a) return E_ACCESSDENIED;
    std::vector<bool> gone(a->index->entries.size(), false);
    for (const auto& n : names)
    {
        size_t node = a->index->Child(dir, n);
        if (node == v31np::None) continue;
        std::vector<size_t> files;
        a->index->tree.Files(node, files);
        for (size_t f : files) gone[a->index->tree.nodes[f].entry] = true;
    }
    std::vector<v31np::Item> items;
    for (size_t i = 0; i < a->index->entries.size(); i++)
        if (!gone[i]) items.push_back(Keep(a, i, a->index->entries[i].key));
    return Commit(a, items);
}

HRESULT RenameNode(const Place& place, const std::wstring& natural, const std::wstring& stored)
{
    if (stored.empty() || stored.find(L'/') != std::wstring::npos) return E_INVALIDARG;
    size_t dir;
    auto a = Writable(place, dir);
    if (!a) return E_ACCESSDENIED;
    size_t node = a->index->Child(dir, natural);
    if (node == v31np::None) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    for (size_t k : a->index->tree.nodes[dir].kids)
        if (k != node && a->index->tree.nodes[k].stored == stored) return HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
    const size_t depth = StoredSegments(a->index->tree, node).size() - 1;
    std::vector<size_t> files;
    a->index->tree.Files(node, files);
    std::vector<std::string> keys(a->index->entries.size());
    for (size_t i = 0; i < a->index->entries.size(); i++) keys[i] = a->index->entries[i].key;
    for (size_t f : files)
    {
        auto segs = StoredSegments(a->index->tree, f);
        segs[depth] = stored;
        keys[a->index->tree.nodes[f].entry] = Join(segs);
    }
    std::vector<v31np::Item> items;
    for (size_t i = 0; i < a->index->entries.size(); i++) items.push_back(Keep(a, i, keys[i]));
    return Commit(a, items);
}

HRESULT CreateFromPaths(const std::wstring& target, const std::vector<std::wstring>& paths)
{
    std::vector<v31np::Item> items;
    Gather({}, paths, items);
    if (items.empty()) return E_INVALIDARG;
    HANDLE h = CreateFileW(target.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());
    bool ok = v31np::Write(items, [h](const void* b, uint32_t n)
    {
        DWORD put = 0;
        return WriteFile(h, b, n, &put, nullptr) && put == n;
    });
    CloseHandle(h);
    if (!ok)
    {
        DeleteFileW(target.c_str());
        return E_FAIL;
    }
    return S_OK;
}

std::vector<std::wstring> DropPaths(IDataObject* data)
{
    std::vector<std::wstring> out;
    if (!data) return out;
    FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM med;
    if (FAILED(data->GetData(&fmt, &med))) return out;
    HDROP drop = (HDROP)GlobalLock(med.hGlobal);
    if (drop)
    {
        UINT n = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);
        for (UINT i = 0; i < n; i++)
        {
            UINT len = DragQueryFileW(drop, i, nullptr, 0);
            std::wstring s(len, L'\0');
            DragQueryFileW(drop, i, s.data(), len + 1);
            out.push_back(s);
        }
        GlobalUnlock(med.hGlobal);
    }
    ReleaseStgMedium(&med);
    return out;
}

bool IsPack(const std::wstring& name)
{
    return name.size() > 6 && _wcsicmp(name.c_str() + name.size() - 6, L".v31np") == 0;
}

PITEMID_CHILD MakeId(const v31np::Node& n, uint64_t size)
{
    const size_t s = std::min<size_t>(n.stored.size(), 15000);
    const size_t t = std::min<size_t>(n.natural.size(), 15000);
    const size_t bytes = sizeof(Id) + (s + t) * sizeof(wchar_t);
    BYTE* p = (BYTE*)CoTaskMemAlloc(bytes + sizeof(USHORT));
    if (!p) return nullptr;
    Id* id = (Id*)p;
    id->cb = (USHORT)bytes;
    id->magic = Magic;
    id->dir = n.dir ? 1 : 0;
    id->size = size;
    id->stored = (USHORT)s;
    id->natural = (USHORT)t;
    wchar_t* text = (wchar_t*)(p + sizeof(Id));
    memcpy(text, n.stored.data(), s * sizeof(wchar_t));
    memcpy(text + s, n.natural.data(), t * sizeof(wchar_t));
    *(USHORT*)(p + bytes) = 0;
    return (PITEMID_CHILD)p;
}

const Id* AsId(PCUIDLIST_RELATIVE pidl)
{
    if (!pidl || pidl->mkid.cb < sizeof(Id)) return nullptr;
    const Id* id = (const Id*)pidl;
    if (id->magic != Magic || sizeof(Id) + ((size_t)id->stored + id->natural) * sizeof(wchar_t) != id->cb) return nullptr;
    return id;
}

std::wstring IdStored(const Id* id)
{
    const wchar_t* text = (const wchar_t*)((const BYTE*)id + sizeof(Id));
    return std::wstring(text, id->stored);
}

std::wstring IdNatural(const Id* id)
{
    const wchar_t* text = (const wchar_t*)((const BYTE*)id + sizeof(Id));
    return std::wstring(text + id->stored, id->natural);
}

void Changed(PCIDLIST_ABSOLUTE pidl)
{
    if (pidl) SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST | SHCNF_FLUSH, pidl, nullptr);
}
