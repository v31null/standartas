#include "shell.h"

#include <map>
#include <set>
#include <thread>

namespace
{
    const CLSID ClsidNewMenu = { 0xd969a300, 0xe7ff, 0x11d0, { 0xa9, 0x3b, 0x00, 0xa0, 0xc9, 0x0f, 0x27, 0x19 } };
    const wchar_t Label[] = L"Ípmùk";

    struct Made
    {
        std::wstring name;
        bool dir = false;
    };

    std::wstring Workdir()
    {
        wchar_t temp[MAX_PATH + 1];
        if (!GetTempPathW(ARRAYSIZE(temp), temp)) return {};
        const std::wstring base = std::wstring(temp) + L"v31np";
        CreateDirectoryW(base.c_str(), nullptr);
        GUID g;
        CoCreateGuid(&g);
        wchar_t tag[64];
        StringFromGUID2(g, tag, ARRAYSIZE(tag));
        const std::wstring dir = base + L"\\" + tag;
        return CreateDirectoryW(dir.c_str(), nullptr) ? dir : std::wstring();
    }

    std::vector<Made> Scan(const std::wstring& dir)
    {
        std::vector<Made> out;
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileExW((dir + L"\\*").c_str(), FindExInfoBasic, &fd, FindExSearchNameMatch, nullptr, 0);
        if (h == INVALID_HANDLE_VALUE) return out;
        do
        {
            const std::wstring name = fd.cFileName;
            if (name == L"." || name == L"..") continue;
            out.push_back({ name, (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 });
        } while (FindNextFileW(h, &fd));
        FindClose(h);
        return out;
    }

    void Clear(const std::wstring& work)
    {
        for (const auto& m : Scan(work))
        {
            const std::wstring full = work + L"\\" + m.name;
            if (m.dir)
            {
                Clear(full);
                continue;
            }
            DeleteFileW(full.c_str());
        }
        RemoveDirectoryW(work.c_str());
    }

    std::set<std::wstring> Prepare(const Place& place, const std::wstring& work)
    {
        std::set<std::wstring> before;
        auto a = Load(place.path, place.chain);
        const size_t dir = a ? a->Find(place.sub) : v31np::None;
        if (dir != v31np::None)
            for (size_t k : a->index->tree.nodes[dir].kids)
            {
                const v31np::Node& n = a->index->tree.nodes[k];
                const std::wstring full = work + L"\\" + n.natural;
                if (n.dir) CreateDirectoryW(full.c_str(), nullptr);
                else
                {
                    HANDLE h = CreateFileW(full.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
                    if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
                }
            }
        for (const auto& m : Scan(work)) before.insert(v31np::Fold(m.name));
        return before;
    }

    using Seen = std::map<std::wstring, std::pair<uint64_t, uint64_t>>;

    Seen State(const std::wstring& work, const std::set<std::wstring>& before)
    {
        Seen out;
        for (const auto& m : Scan(work))
        {
            if (before.count(v31np::Fold(m.name))) continue;
            WIN32_FILE_ATTRIBUTE_DATA d{};
            GetFileAttributesExW((work + L"\\" + m.name).c_str(), GetFileExInfoStandard, &d);
            out[m.name] = { ((uint64_t)d.nFileSizeHigh << 32) | d.nFileSizeLow, ((uint64_t)d.ftLastWriteTime.dwHighDateTime << 32) | d.ftLastWriteTime.dwLowDateTime };
        }
        return out;
    }

    bool Wizard(const Seen& seen)
    {
        for (const auto& m : seen)
            if (m.first.size() >= 4 && _wcsicmp(m.first.c_str() + m.first.size() - 4, L".lnk") == 0) return true;
        return false;
    }

    HRESULT Import(const Place& place, const std::wstring& work, const std::set<std::wstring>& before, std::vector<Made>& made, const Seen* hold = nullptr)
    {
        made.clear();
        // Delete not — « New > Shortcut » leaves a placeholder .lnk here and hands it to a wizard that finishes it later.
        // Snatch it early and the wizard vanishes mid-sentence. So a fresh .lnk sits still until the wizard changes,
        // renames or removes it.
        if (hold && State(work, before) == *hold) return S_FALSE;
        for (const auto& m : Scan(work))
            if (!before.count(v31np::Fold(m.name))) made.push_back(m);
        if (made.empty()) return S_FALSE;
        return Mutate(place.path, [&](Edit& e)
        {
            auto a = Load(place.path, place.chain);
            const size_t dir = a ? a->Find(place.sub) : v31np::None;
            if (dir == v31np::None || !a->index->tree.nodes[dir].dir) return E_ACCESSDENIED;
            const std::string prefix = Join(StoredSegments(a->index->tree, dir));
            a.reset();
            if (!e.Part(place.chain)) return E_FAIL;
            for (const auto& m : made)
            {
                const std::string n = v31np::Narrow(m.name);
                v31np::Row row;
                row.key = prefix.empty() ? n : prefix + "/" + n;
                if (m.dir) row.kind = 'D';
                else
                {
                    const std::wstring t = e.Temp();
                    if (t.empty() || !MoveFileExW((work + L"\\" + m.name).c_str(), t.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) return E_FAIL;
                    WIN32_FILE_ATTRIBUTE_DATA d;
                    if (!GetFileAttributesExW(t.c_str(), GetFileExInfoStandard, &d)) return E_FAIL;
                    row.kind = 'T';
                    row.file = t;
                    row.size = ((uint64_t)d.nFileSizeHigh << 32) | d.nFileSizeLow;
                }
                e.Put(place.chain, std::move(row));
            }
            return S_OK;
        });
    }

    void Rename(IUnknown* site, PCIDLIST_ABSOLUTE root, const Place& place, const std::wstring& name)
    {
        auto a = Load(place.path, place.chain);
        const size_t dir = a ? a->Find(place.sub) : v31np::None;
        const size_t k = dir == v31np::None ? v31np::None : a->index->Child(dir, name);
        if (k == v31np::None || !root) return;
        const v31np::Node& n = a->index->tree.nodes[k];
        PITEMID_CHILD id = MakeId(n, n.dir ? 0 : a->index->entries[n.entry].size);
        if (!id) return;
        if (PIDLIST_ABSOLUTE full = ILCombine(root, id))
        {
            SHChangeNotify(n.dir ? SHCNE_MKDIR : SHCNE_CREATE, SHCNF_IDLIST | SHCNF_FLUSH, full, nullptr);
            CoTaskMemFree(full);
        }
        ComPtr<IShellBrowser> browser;
        ComPtr<IShellView> view;
        if (site && SUCCEEDED(IUnknown_QueryService(site, SID_STopLevelBrowser, IID_PPV_ARGS(&browser))) && SUCCEEDED(browser->QueryActiveShellView(&view)))
        {
            const UINT how = SVSI_EDIT | SVSI_SELECT | SVSI_DESELECTOTHERS | SVSI_ENSUREVISIBLE | SVSI_FOCUSED;
            if (FAILED(view->SelectItem(id, how)))
            {
                Changed(root);
                view->SelectItem(id, how);
            }
        }
        CoTaskMemFree(id);
    }

    void Watch(Place place, PIDLIST_ABSOLUTE root, std::wstring work, std::set<std::wstring> before, Seen hold)
    {
        std::thread([place, root, work, before, hold]()
        {
            Lock keep;
            const ULONGLONG end = GetTickCount64() + 10 * 60 * 1000;
            HANDLE change = FindFirstChangeNotificationW(work.c_str(), FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE);
            while (change != INVALID_HANDLE_VALUE)
            {
                const ULONGLONG now = GetTickCount64();
                if (now >= end || WaitForSingleObject(change, (DWORD)(end - now)) != WAIT_OBJECT_0) break;
                Sleep(1000);
                std::vector<Made> made;
                if (Import(place, work, before, made, &hold) == S_OK)
                {
                    Changed(root);
                    break;
                }
                if (!FindNextChangeNotification(change)) break;
            }
            if (change != INVALID_HANDLE_VALUE) FindCloseChangeNotification(change);
            Clear(work);
            CoTaskMemFree(root);
        }).detach();
    }

    class BackMenu final : public IContextMenu3, public IObjectWithSite
    {
        REFS

    public:
        ComPtr<IContextMenu> inner;
        ComPtr<IContextMenu> fresh;
        ComPtr<IUnknown> site;
        Place place;
        PIDLIST_ABSOLUTE root = nullptr;
        std::wstring work;
        UINT base = 0;
        UINT count = 0;

        ~BackMenu()
        {
            fresh.Reset();
            if (!work.empty()) Clear(work);
            CoTaskMemFree(root);
        }

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(BackMenu, IContextMenu3),
                QITABENTMULTI(BackMenu, IContextMenu2, IContextMenu3),
                QITABENTMULTI(BackMenu, IContextMenu, IContextMenu3),
                QITABENT(BackMenu, IObjectWithSite),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        bool Owns(UINT_PTR cmd) const
        {
            return fresh && cmd >= base && cmd < base + count;
        }

        IFACEMETHODIMP QueryContextMenu(HMENU menu, UINT index, UINT first, UINT last, UINT flags) override
        {
            HRESULT r = inner->QueryContextMenu(menu, index, first, last, flags);
            if (FAILED(r) || (flags & (CMF_DEFAULTONLY | CMF_VERBSONLY))) return r;
            const UINT used = HRESULT_CODE(r);
            if (work.empty()) work = Workdir();
            if (work.empty()) return r;
            PIDLIST_ABSOLUTE at = ILCreateFromPathW(work.c_str());
            ComPtr<IContextMenu> m;
            ComPtr<IShellExtInit> init;
            HRESULT h = at ? CoCreateInstance(ClsidNewMenu, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m)) : E_OUTOFMEMORY;
            if (SUCCEEDED(h)) h = m.As(&init);
            if (SUCCEEDED(h)) h = init->Initialize(at, nullptr, nullptr);
            CoTaskMemFree(at);
            if (FAILED(h)) return r;
            if (site) IUnknown_SetSite(m.Get(), site.Get());
            const int had = GetMenuItemCount(menu);
            const UINT pos = had > 0 ? (UINT)had - 1 : 0;
            h = m->QueryContextMenu(menu, pos, first + used, last, flags);
            if (FAILED(h)) return r;
            fresh = m;
            base = used;
            count = HRESULT_CODE(h);
            const int now = GetMenuItemCount(menu);
            for (int i = (int)pos; i < (int)pos + (now - had); i++)
            {
                if (!GetSubMenu(menu, i)) continue;
                MENUITEMINFOW mi = { sizeof(mi) };
                mi.fMask = MIIM_STRING;
                mi.dwTypeData = (LPWSTR)Label;
                SetMenuItemInfoW(menu, i, TRUE, &mi);
                break;
            }
            return MAKE_HRESULT(SEVERITY_SUCCESS, 0, used + count);
        }

        IFACEMETHODIMP InvokeCommand(CMINVOKECOMMANDINFO* info) override
        {
            if (!IS_INTRESOURCE(info->lpVerb) || !Owns(LOWORD(info->lpVerb))) return inner->InvokeCommand(info);
            const UINT off = LOWORD(info->lpVerb) - base;
            const std::set<std::wstring> before = Prepare(place, work);
            CMINVOKECOMMANDINFOEX ex = {};
            memcpy(&ex, info, std::min<size_t>(info->cbSize, sizeof(ex)));
            ex.lpVerb = MAKEINTRESOURCEA(off);
            if (info->cbSize >= sizeof(CMINVOKECOMMANDINFOEX)) ex.lpVerbW = MAKEINTRESOURCEW(off);
            const HRESULT r = fresh->InvokeCommand((CMINVOKECOMMANDINFO*)&ex);
            const Seen seen = State(work, before);
            std::vector<Made> made;
            if (!Wizard(seen) && Import(place, work, before, made) == S_OK)
            {
                Changed(root);
                Rename(site.Get(), root, place, made[0].name);
                return r;
            }
            if (SUCCEEDED(r))
            {
                Watch(place, root ? ILCloneFull(root) : nullptr, work, before, Wizard(seen) ? seen : Seen{});
                work.clear();
            }
            return r;
        }

        IFACEMETHODIMP GetCommandString(UINT_PTR cmd, UINT type, UINT* reserved, LPSTR out, UINT cch) override
        {
            if (Owns(cmd)) return fresh->GetCommandString(cmd - base, type, reserved, out, cch);
            return inner->GetCommandString(cmd, type, reserved, out, cch);
        }

        IFACEMETHODIMP HandleMenuMsg(UINT msg, WPARAM w, LPARAM l) override
        {
            LRESULT ignored = 0;
            return HandleMenuMsg2(msg, w, l, &ignored);
        }

        IFACEMETHODIMP HandleMenuMsg2(UINT msg, WPARAM w, LPARAM l, LRESULT* result) override
        {
            HRESULT r = E_NOTIMPL;
            ComPtr<IContextMenu3> f3;
            ComPtr<IContextMenu2> f2;
            if (fresh && SUCCEEDED(fresh.As(&f3))) r = f3->HandleMenuMsg2(msg, w, l, result);
            else if (fresh && SUCCEEDED(fresh.As(&f2))) r = f2->HandleMenuMsg(msg, w, l);
            ComPtr<IContextMenu3> i3;
            ComPtr<IContextMenu2> i2;
            HRESULT q = E_NOTIMPL;
            if (SUCCEEDED(inner.As(&i3))) q = i3->HandleMenuMsg2(msg, w, l, result);
            else if (SUCCEEDED(inner.As(&i2))) q = i2->HandleMenuMsg(msg, w, l);
            return SUCCEEDED(r) ? r : q;
        }

        IFACEMETHODIMP SetSite(IUnknown* s) override
        {
            site = s;
            IUnknown_SetSite(inner.Get(), s);
            if (fresh) IUnknown_SetSite(fresh.Get(), s);
            return S_OK;
        }

        IFACEMETHODIMP GetSite(REFIID riid, void** ppv) override
        {
            *ppv = nullptr;
            return site ? site->QueryInterface(riid, ppv) : E_FAIL;
        }
    };
}

HRESULT WrapBackMenu(const Place& place, PCIDLIST_ABSOLUTE root, IContextMenu* inner, REFIID riid, void** ppv)
{
    *ppv = nullptr;
    BackMenu* m = new (std::nothrow) BackMenu();
    if (!m) return E_OUTOFMEMORY;
    m->inner = inner;
    m->place = place;
    m->root = root ? ILCloneFull(root) : nullptr;
    HRESULT r = m->QueryInterface(riid, ppv);
    m->Release();
    return r;
}
