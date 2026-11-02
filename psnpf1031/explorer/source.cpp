#include "shell.h"

#include <algorithm>
#include <set>
#include <thread>
#include <sherrors.h>

namespace
{
    std::mutex markGuard;
    std::set<std::wstring> marked;

    std::wstring MarkOf(const Place& place, const std::wstring& name)
    {
        std::wstring k = ArchiveId(place.path, place.chain);
        for (const auto& s : place.sub) k += L'\x1' + s;
        return k + L'\x1' + name;
    }

    bool Full(const Place& place, const std::wstring& name)
    {
        auto a = Load(place.path, place.chain);
        const size_t dir = a ? a->Find(place.sub) : v31np::None;
        const size_t k = dir == v31np::None ? v31np::None : a->index->Child(dir, name);
        return k != v31np::None && a->index->tree.nodes[k].dir && !a->index->tree.nodes[k].kids.empty();
    }

    void Sweep(Place place)
    {
        while (!place.sub.empty())
        {
            const std::wstring name = place.sub.back();
            place.sub.pop_back();
            {
                std::lock_guard<std::mutex> g(markGuard);
                auto it = marked.find(MarkOf(place, name));
                if (it == marked.end() || Full(place, name)) return;
                marked.erase(it);
            }
            RemoveNodes(place, { name });
        }
    }

    struct Found
    {
        PIDLIST_ABSOLUTE full = nullptr;
        Place place;
        std::wstring natural;

        ~Found()
        {
            CoTaskMemFree(full);
        }
    };

    HRESULT Find(IShellItem* item, Found& f)
    {
        if (!item) return E_INVALIDARG;
        HRESULT r = SHGetIDListFromObject(item, &f.full);
        if (FAILED(r)) return r;
        return Locate(f.full, f.place, f.natural);
    }

    size_t Deepest(const v31np::Tree& t, size_t x)
    {
        size_t most = 0;
        for (size_t k : t.nodes[x].kids) most = std::max(most, Deepest(t, k) + 1);
        return t.nodes[x].natural.size() + most;
    }

    void NotifyParent(PCIDLIST_ABSOLUTE full)
    {
        PIDLIST_ABSOLUTE up = ILCloneFull(full);
        if (!up) return;
        ILRemoveLastID(up);
        Changed(up);
        CoTaskMemFree(up);
    }

    class Source final : public ITransferSource
    {
        REFS

    public:
        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(Source, ITransferSource),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP Advise(ITransferAdviseSink*, DWORD* cookie) override
        {
            *cookie = 1;
            return S_OK;
        }

        IFACEMETHODIMP Unadvise(DWORD) override
        {
            return S_OK;
        }

        IFACEMETHODIMP SetProperties(IPropertyChangeArray*) override
        {
            return S_OK;
        }

        IFACEMETHODIMP OpenItem(IShellItem* item, TRANSFER_SOURCE_FLAGS, REFIID, void** ppv) override
        {
            *ppv = nullptr;
            Found f;
            HRESULT r = Find(item, f);
            if (FAILED(r)) return r;
            auto a = Load(f.place.path, f.place.chain);
            if (!a) return HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
            const size_t dir = a->Find(f.place.sub);
            if (dir == v31np::None) return HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
            const size_t k = a->index->Child(dir, f.natural);
            if (k == v31np::None) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            // Delete not — S_FALSE = « help yourself, the fridge is open ». The copy engine then walks into folders and
            // reads files as plain streams, exactly like it does for Windows' own zip folder. The « proper » way
            // (resources) needs a content ID Microsoft never wrote down; trying it got us lovely, empty, 0-byte files.
            return S_FALSE;
        }

        IFACEMETHODIMP MoveItem(IShellItem*, IShellItem*, LPCWSTR, TRANSFER_SOURCE_FLAGS, IShellItem** out) override
        {
            if (out) *out = nullptr;
            return E_NOTIMPL;
        }

        // Delete not — no Recycle Bin inside an archive, so « recycle » just means delete. Explorer's own Delete key never
        // comes here (it rides our menu, no questions asked). Only programs driving the copy engine do, and the engine
        // asks « are you sure? » before it even knocks. Windows gets its little question; we stopped arguing.
        IFACEMETHODIMP RecycleItem(IShellItem* item, IShellItem*, TRANSFER_SOURCE_FLAGS flags, IShellItem** out) override
        {
            if (out) *out = nullptr;
            return RemoveItem(item, flags);
        }

        IFACEMETHODIMP RemoveItem(IShellItem* item, TRANSFER_SOURCE_FLAGS flags) override
        {
            Found f;
            HRESULT r = Find(item, f);
            // delete not — If Windows asks you to delete something that already doesn't exist, smile and tell it good job.
            // It cries if it didn't get to push the button itself. « Wow Windows! Look at you! You did it! You deleted the
            // folder all by yourself! Who's a big boy?! »
            if (r == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) return S_OK;
            if (FAILED(r)) return r;
            // Delete not — the copy engine asks to remove a folder BEFORE it touches what is inside. On a delete we wipe it
            // all and answer « don't process the children », or it goes looking for the kids we just buried (0x80070002).
            // On a move it has not copied the kids yet, so we only nod, keep everything, and let the folder go once its
            // last child has moved out. Lying « not empty » just made it give up.
            if (Full(f.place, f.natural))
            {
                if (flags & TSF_MOVE_AS_COPY_DELETE)
                {
                    std::lock_guard<std::mutex> g(markGuard);
                    marked.insert(MarkOf(f.place, f.natural));
                    return S_OK;
                }
                r = RemoveNodes(f.place, { f.natural });
                if (FAILED(r)) return r;
                NotifyParent(f.full);
                return COPYENGINE_S_DONT_PROCESS_CHILDREN;
            }
            r = RemoveNodes(f.place, { f.natural });
            if (FAILED(r)) return r;
            Sweep(f.place);
            NotifyParent(f.full);
            return r;
        }

        IFACEMETHODIMP RenameItem(IShellItem* item, LPCWSTR name, TRANSFER_SOURCE_FLAGS, IShellItem** out) override
        {
            if (out) *out = nullptr;
            if (!name) return E_INVALIDARG;
            Found f;
            HRESULT r = Find(item, f);
            if (FAILED(r)) return r;
            r = RenameNode(f.place, f.natural, name);
            if (FAILED(r)) return r;
            NotifyParent(f.full);
            if (!out) return S_OK;
            auto a = Load(f.place.path, f.place.chain);
            if (!a) return S_OK;
            const size_t dir = a->Find(f.place.sub);
            if (dir == v31np::None) return S_OK;
            for (size_t k : a->index->tree.nodes[dir].kids)
            {
                const v31np::Node& n = a->index->tree.nodes[k];
                if (n.stored != name) continue;
                PITEMID_CHILD id = MakeId(n, n.dir ? 0 : a->index->entries[n.entry].size);
                PIDLIST_ABSOLUTE up = ILCloneFull(f.full);
                if (id && up)
                {
                    ILRemoveLastID(up);
                    if (PIDLIST_ABSOLUTE now = ILCombine(up, id))
                    {
                        SHCreateItemFromIDList(now, IID_PPV_ARGS(out));
                        CoTaskMemFree(now);
                    }
                }
                CoTaskMemFree(id);
                CoTaskMemFree(up);
                break;
            }
            return S_OK;
        }

        IFACEMETHODIMP LinkItem(IShellItem*, IShellItem*, LPCWSTR, TRANSFER_SOURCE_FLAGS, IShellItem** out) override
        {
            if (out) *out = nullptr;
            return E_NOTIMPL;
        }

        IFACEMETHODIMP ApplyPropertiesToItem(IShellItem*, IShellItem** out) override
        {
            if (out) *out = nullptr;
            return E_NOTIMPL;
        }

        IFACEMETHODIMP GetDefaultDestinationName(IShellItem* item, IShellItem* parent, LPWSTR* out) override
        {
            *out = nullptr;
            Found f;
            HRESULT r = Find(item, f);
            if (FAILED(r)) return r;
            std::wstring name = f.natural;
            PWSTR where = nullptr;
            if (parent && SUCCEEDED(parent->GetDisplayName(SIGDN_FILESYSPATH, &where)) && where)
            {
                std::wstring dir = where;
                CoTaskMemFree(where);
                if (!dir.empty() && dir.back() != L'\\') dir += L'\\';
                auto a = Load(f.place.path, f.place.chain);
                const size_t up = a ? a->Find(f.place.sub) : v31np::None;
                const size_t node = up == v31np::None ? v31np::None : a->index->Child(up, f.natural);
                const bool folder = node != v31np::None && a->index->tree.nodes[node].dir;
                const bool fit = node != v31np::None && !v31np::LongPaths();
                auto fitted = [&](size_t extra)
                {
                    if (!fit || dir.size() + extra + Deepest(a->index->tree, node) <= v31np::PathMax) return f.natural;
                    v31np::Tree t = a->index->tree;
                    t.Fit(up, dir.size() + extra, false);
                    return t.nodes[node].natural;
                };
                auto clash = [&](const std::wstring& n)
                {
                    const DWORD at = GetFileAttributesW((dir + n).c_str());
                    if (at == INVALID_FILE_ATTRIBUTES) return false;
                    return !(folder && (at & FILE_ATTRIBUTE_DIRECTORY));
                };
                std::wstring base = fitted(0);
                size_t extra = 0;
                name = base;
                for (size_t k = 1; clash(name); k++)
                {
                    const size_t need = v31np::Suffix(L"", k).size();
                    if (need != extra)
                    {
                        extra = need;
                        base = fitted(extra);
                    }
                    name = v31np::Suffix(base, k);
                }
            }
            return SHStrDupW(name.c_str(), out);
        }

        IFACEMETHODIMP EnterFolder(IShellItem*) override
        {
            return S_OK;
        }

        IFACEMETHODIMP LeaveFolder(IShellItem*) override
        {
            return S_OK;
        }
    };

    class ViewCallback final : public IShellFolderViewCB
    {
        REFS

    public:
        std::wstring path;
        HANDLE stop = nullptr;

        ~ViewCallback()
        {
            if (stop)
            {
                SetEvent(stop);
                CloseHandle(stop);
            }
            Leave(path);
        }

        void Watch(PCIDLIST_ABSOLUTE root)
        {
            if (!root || !v31np::SharedRoad(path)) return;
            HANDLE mine = CreateEventW(nullptr, TRUE, FALSE, nullptr);
            if (!mine) return;
            HANDLE theirs = nullptr;
            if (!DuplicateHandle(GetCurrentProcess(), mine, GetCurrentProcess(), &theirs, 0, FALSE, DUPLICATE_SAME_ACCESS))
            {
                CloseHandle(mine);
                return;
            }
            PIDLIST_ABSOLUTE pidl = ILCloneFull(root);
            if (!pidl)
            {
                CloseHandle(mine);
                CloseHandle(theirs);
                return;
            }
            stop = mine;
            const std::wstring file = path;
            std::thread([file, pidl, theirs]()
            {
                Lock keep;
                auto look = [&file](uint64_t& size, uint64_t& time)
                {
                    WIN32_FILE_ATTRIBUTE_DATA d;
                    if (!GetFileAttributesExW(file.c_str(), GetFileExInfoStandard, &d)) return false;
                    size = ((uint64_t)d.nFileSizeHigh << 32) | d.nFileSizeLow;
                    time = ((uint64_t)d.ftLastWriteTime.dwHighDateTime << 32) | d.ftLastWriteTime.dwLowDateTime;
                    return true;
                };
                const size_t cut = file.find_last_of(L"\\/");
                const std::wstring dir = cut == std::wstring::npos ? L"." : file.substr(0, cut);
                HANDLE change = FindFirstChangeNotificationW(dir.c_str(), FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE);
                uint64_t size = 0;
                uint64_t time = 0;
                bool known = look(size, time);
                for (;;)
                {
                    HANDLE wait[2] = { theirs, change };
                    const DWORD n = change == INVALID_HANDLE_VALUE ? 1 : 2;
                    const DWORD w = WaitForMultipleObjects(n, wait, FALSE, 2000);
                    if (w == WAIT_OBJECT_0 || w == WAIT_FAILED) break;
                    if (w == WAIT_OBJECT_0 + 1 && !FindNextChangeNotification(change))
                    {
                        FindCloseChangeNotification(change);
                        change = INVALID_HANDLE_VALUE;
                    }
                    uint64_t s = 0;
                    uint64_t t = 0;
                    const bool now = look(s, t);
                    if (now == known && s == size && t == time) continue;
                    known = now;
                    size = s;
                    time = t;
                    Changed(pidl);
                }
                if (change != INVALID_HANDLE_VALUE) FindCloseChangeNotification(change);
                CloseHandle(theirs);
                CoTaskMemFree(pidl);
            }).detach();
        }

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(ViewCallback, IShellFolderViewCB),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP MessageSFVCB(UINT, WPARAM, LPARAM) override
        {
            return E_NOTIMPL;
        }
    };
}

HRESULT CreateSource(const Place&, PCIDLIST_ABSOLUTE, REFIID riid, void** ppv)
{
    *ppv = nullptr;
    Source* s = new (std::nothrow) Source();
    if (!s) return E_OUTOFMEMORY;
    HRESULT r = s->QueryInterface(riid, ppv);
    s->Release();
    return r;
}

HRESULT CreateViewCallback(const std::wstring& path, PCIDLIST_ABSOLUTE root, IShellFolderViewCB** out)
{
    *out = nullptr;
    ViewCallback* v = new (std::nothrow) ViewCallback();
    if (!v) return E_OUTOFMEMORY;
    v->path = path;
    Enter(path);
    v->Watch(root);
    *out = v;
    return S_OK;
}
