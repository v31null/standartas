#include "shell.h"

#include <algorithm>
#include <mutex>
#include <set>
#include <thread>

#include <strsafe.h>
#include <wincodec.h>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace
{
    const wchar_t* const Verbs[] = { L"v31np.name", L"v31np.here", L"v31np.there", L"v31np.therename", L"v31np.allhere", L"v31np.allthere" };

    std::wstring Bare(const std::wstring& file)
    {
        size_t slash = file.find_last_of(L"\\/");
        std::wstring name = slash == std::wstring::npos ? file : file.substr(slash + 1);
        if (IsPack(name)) name.resize(name.size() - 6);
        return name;
    }

    std::wstring Dir(const std::wstring& file)
    {
        size_t slash = file.find_last_of(L"\\/");
        if (slash == std::wstring::npos) return L".";
        return slash == 2 && file[1] == L':' ? file.substr(0, 3) : file.substr(0, slash);
    }

    std::wstring Within(const std::wstring& dir, const std::wstring& name)
    {
        return !dir.empty() && (dir.back() == L'\\' || dir.back() == L'/') ? dir + name : dir + L"\\" + name;
    }

    std::wstring PickFolder(HWND hwnd)
    {
        ComPtr<IFileOpenDialog> dlg;
        if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dlg)))) return {};
        FILEOPENDIALOGOPTIONS opts = 0;
        dlg->GetOptions(&opts);
        dlg->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        if (dlg->Show(hwnd) != S_OK) return {};
        ComPtr<IShellItem> item;
        if (FAILED(dlg->GetResult(&item))) return {};
        PWSTR path = nullptr;
        if (FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) return {};
        std::wstring r = path;
        CoTaskMemFree(path);
        return r;
    }

    FILETIME MonthAgo()
    {
        SYSTEMTIME t;
        GetSystemTime(&t);
        if (t.wMonth == 1)
        {
            t.wMonth = 12;
            t.wYear--;
        }
        else t.wMonth--;
        static const WORD days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        WORD last = days[t.wMonth - 1];
        if (t.wMonth == 2 && ((t.wYear % 4 == 0 && t.wYear % 100 != 0) || t.wYear % 400 == 0)) last = 29;
        if (t.wDay > last) t.wDay = last;
        t.wDayOfWeek = 0;
        FILETIME f{};
        SystemTimeToFileTime(&t, &f);
        return f;
    }

    std::mutex watchGuard;
    std::set<std::wstring> watched;

    struct Copy
    {
        std::wstring file;
        std::string key;
        uint64_t basis = 0;
        uint64_t size = 0;
        uint64_t time = 0;
    };

    bool Stat(const std::wstring& file, uint64_t& size, uint64_t& time)
    {
        WIN32_FILE_ATTRIBUTE_DATA d;
        if (!GetFileAttributesExW(file.c_str(), GetFileExInfoStandard, &d)) return false;
        size = ((uint64_t)d.nFileSizeHigh << 32) | d.nFileSizeLow;
        time = ((uint64_t)d.ftLastWriteTime.dwHighDateTime << 32) | d.ftLastWriteTime.dwLowDateTime;
        return true;
    }

    void Mix(uint64_t& h, const BYTE* p, size_t n)
    {
        for (size_t i = 0; i < n; i++)
        {
            h ^= p[i];
            h *= 1099511628211ull;
        }
    }

    bool HashFile(const std::wstring& file, uint64_t& out)
    {
        HANDLE h = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (h == INVALID_HANDLE_VALUE) return false;
        uint64_t x = 14695981039346656037ull;
        std::vector<BYTE> buf(1 << 20);
        bool ok = true;
        for (;;)
        {
            DWORD got = 0;
            if (!ReadFile(h, buf.data(), (DWORD)buf.size(), &got, nullptr))
            {
                ok = false;
                break;
            }
            if (got == 0) break;
            Mix(x, buf.data(), got);
        }
        CloseHandle(h);
        out = x;
        return ok;
    }

    bool HashEntry(const std::shared_ptr<Archive>& a, size_t node, uint64_t& out)
    {
        ComPtr<IStream> s;
        if (FAILED(CreateEntryStream(a, node, &s))) return false;
        uint64_t x = 14695981039346656037ull;
        std::vector<BYTE> buf(1 << 20);
        for (;;)
        {
            ULONG got = 0;
            if (FAILED(s->Read(buf.data(), (ULONG)buf.size(), &got))) return false;
            if (got == 0) break;
            Mix(x, buf.data(), got);
        }
        out = x;
        return true;
    }

    HRESULT SaveBack(const Place& place, Copy& c)
    {
        uint64_t written = 0;
        uint64_t size = 0;
        uint64_t time = 0;
        if (!Stat(c.file, size, time) || !HashFile(c.file, written)) return HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION);
        std::string key = c.key;
        HRESULT r = Mutate(place.path, [&](Edit& e)
        {
            auto a = Load(place.path, place.chain);
            if (!a) return E_FAIL;
            const v31np::Tree& t = a->index->tree;
            size_t node = v31np::None;
            for (size_t k = 0; k < t.nodes.size(); k++)
                if (!t.nodes[k].dir && Join(StoredSegments(t, k)) == key)
                {
                    node = k;
                    break;
                }
            uint64_t now = 0;
            const bool clash = node != v31np::None && (!HashEntry(a, node, now) || (now != c.basis && now != written));
            a.reset();
            if (!e.Part(place.chain)) return E_FAIL;
            if (clash)
            {
                const size_t cut = key.find_last_of('/');
                key = e.Fresh(place.chain, key, cut == std::string::npos ? std::string() : key.substr(0, cut), false, false);
            }
            const std::wstring tmp = e.Temp();
            if (tmp.empty() || !CopyFileW(c.file.c_str(), tmp.c_str(), FALSE)) return E_FAIL;
            v31np::Row row;
            row.kind = 'T';
            row.key = key;
            row.size = size;
            row.file = tmp;
            e.Put(place.chain, std::move(row));
            return S_OK;
        }, nullptr, true);
        if (FAILED(r)) return r;
        c.key = key;
        c.basis = written;
        c.size = size;
        c.time = time;
        return S_OK;
    }

    void Follow(Place place, std::wstring dir, std::vector<Copy> copies, PIDLIST_ABSOLUTE root)
    {
        {
            std::lock_guard<std::mutex> g(watchGuard);
            watched.insert(v31np::Fold(dir));
        }
        std::thread([place = std::move(place), dir, copies = std::move(copies), root]() mutable
        {
            Lock keep;
            HANDLE change = FindFirstChangeNotificationW(dir.c_str(), FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE);
            bool waiting = false;
            while (change != INVALID_HANDLE_VALUE)
            {
                const DWORD w = WaitForSingleObject(change, waiting ? 2000 : INFINITE);
                if (w != WAIT_OBJECT_0 && w != WAIT_TIMEOUT) break;
                if (GetFileAttributesW(dir.c_str()) == INVALID_FILE_ATTRIBUTES) break;
                if (w == WAIT_OBJECT_0) Sleep(1000);
                waiting = false;
                std::vector<std::string> landed;
                for (auto& c : copies)
                {
                    uint64_t size = 0;
                    uint64_t time = 0;
                    if (!Stat(c.file, size, time) || (size == c.size && time == c.time)) continue;
                    if (SUCCEEDED(SaveBack(place, c))) landed.push_back(c.key);
                    else waiting = true;
                }
                if (!landed.empty())
                {
                    Changed(root);
                    if (auto a = root ? Load(place.path, place.chain) : nullptr)
                    {
                        const v31np::Tree& t = a->index->tree;
                        for (size_t k = 0; k < t.nodes.size(); k++)
                        {
                            if (t.nodes[k].dir || std::find(landed.begin(), landed.end(), Join(StoredSegments(t, k))) == landed.end()) continue;
                            PITEMID_CHILD id = MakeId(t.nodes[k], a->index->entries[t.nodes[k].entry].size);
                            if (!id) continue;
                            if (PIDLIST_ABSOLUTE full = ILCombine(root, id))
                            {
                                SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_IDLIST | SHCNF_FLUSH, full, nullptr);
                                CoTaskMemFree(full);
                            }
                            CoTaskMemFree(id);
                        }
                    }
                }
                if (w == WAIT_OBJECT_0 && !FindNextChangeNotification(change)) break;
            }
            if (change != INVALID_HANDLE_VALUE) FindCloseChangeNotification(change);
            {
                std::lock_guard<std::mutex> g(watchGuard);
                watched.erase(v31np::Fold(dir));
            }
            CoTaskMemFree(root);
        }).detach();
    }

    void Sweep(const std::wstring& base)
    {
        const FILETIME cut = MonthAgo();
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileExW((base + L"\\*").c_str(), FindExInfoBasic, &fd, FindExSearchLimitToDirectories, nullptr, 0);
        if (h == INVALID_HANDLE_VALUE) return;
        do
        {
            GUID g;
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) continue;
            if (wcslen(fd.cFileName) != 38 || FAILED(IIDFromString(fd.cFileName, &g))) continue;
            if (CompareFileTime(&fd.ftCreationTime, &cut) >= 0) continue;
            const std::wstring dir = base + L"\\" + fd.cFileName;
            {
                std::lock_guard<std::mutex> w(watchGuard);
                if (watched.count(v31np::Fold(dir))) continue;
            }
            WIN32_FIND_DATAW in;
            HANDLE k = FindFirstFileExW((dir + L"\\*").c_str(), FindExInfoBasic, &in, FindExSearchNameMatch, nullptr, 0);
            if (k != INVALID_HANDLE_VALUE)
            {
                do
                    if (!(in.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) DeleteFileW((dir + L"\\" + in.cFileName).c_str());
                while (FindNextFileW(k, &in));
                FindClose(k);
            }
            RemoveDirectoryW(dir.c_str());
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }

    void Run(std::vector<std::pair<std::wstring, std::wstring>> jobs, HWND hwnd)
    {
        std::thread([jobs = std::move(jobs), hwnd]()
        {
            Lock hold;
            if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return;
            for (const auto& j : jobs) ExtractTo(j.first, j.second, hwnd);
            CoUninitialize();
        }).detach();
    }

    class FileMenu final : public IShellExtInit, public IContextMenu
    {
        REFS

    public:
        std::vector<std::wstring> files;

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(FileMenu, IShellExtInit),
                QITABENT(FileMenu, IContextMenu),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP Initialize(PCIDLIST_ABSOLUTE, IDataObject* data, HKEY) override
        {
            files.clear();
            for (const auto& p : DropPaths(data))
                if (IsPack(p)) files.push_back(p);
            return files.empty() ? E_FAIL : S_OK;
        }

        std::vector<int> Shown() const
        {
            if (files.size() > 1) return { 4, 5 };
            return { 0, 1, 2, 3 };
        }

        IFACEMETHODIMP QueryContextMenu(HMENU menu, UINT index, UINT first, UINT, UINT flags) override
        {
            if (flags & CMF_DEFAULTONLY) return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
            const std::wstring name = Bare(files[0]);
            const std::wstring labels[] = {
                L"Anfas alùz e íülíntù " + name,
                L"Anfas alùz híre",
                L"Anfas alùz țùre",
                L"Anfas alùz țùre e íülíntù " + name,
                L"Anfas teí̈nt híre",
                L"Anfas teí̈nt țùre",
            };
            const auto shown = Shown();
            for (UINT i = 0; i < shown.size(); i++) InsertMenuW(menu, index + i, MF_BYPOSITION | MF_STRING, first + i, labels[shown[i]].c_str());
            return MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)shown.size());
        }

        IFACEMETHODIMP InvokeCommand(CMINVOKECOMMANDINFO* info) override
        {
            const auto shown = Shown();
            int kind = -1;
            if (IS_INTRESOURCE(info->lpVerb))
            {
                const UINT at = LOWORD(info->lpVerb);
                if (at < shown.size()) kind = shown[at];
            }
            else
            {
                const CMINVOKECOMMANDINFOEX* ex = (const CMINVOKECOMMANDINFOEX*)info;
                const bool wide = info->cbSize >= sizeof(CMINVOKECOMMANDINFOEX) && (info->fMask & CMIC_MASK_UNICODE) && ex->lpVerbW;
                for (int k : shown)
                    if (wide ? _wcsicmp(ex->lpVerbW, Verbs[k]) == 0 : _stricmp(info->lpVerb, v31np::Narrow(Verbs[k]).c_str()) == 0) kind = k;
            }
            if (kind < 0) return E_INVALIDARG;
            const bool there = kind == 2 || kind == 3 || kind == 5;
            std::wstring picked;
            if (there)
            {
                picked = PickFolder(info->hwnd);
                if (picked.empty()) return S_OK;
            }
            std::vector<std::pair<std::wstring, std::wstring>> jobs;
            if (kind >= 4)
            {
                const std::wstring at = there ? picked : Dir(files[0]);
                const std::wstring base = L"baíbíz e alùz";
                std::wstring folder = base;
                for (size_t k = 1; GetFileAttributesW(Within(at, folder).c_str()) != INVALID_FILE_ATTRIBUTES; k++) folder = v31np::Suffix(base, k);
                for (const auto& f : files) jobs.emplace_back(f, Within(Within(at, folder), Bare(f)));
            }
            else
            {
                for (const auto& f : files)
                {
                    const std::wstring at = there ? picked : Dir(f);
                    jobs.emplace_back(f, (kind == 0 || kind == 3) ? Within(at, Bare(f)) : at);
                }
            }
            Run(std::move(jobs), info->hwnd);
            return S_OK;
        }

        IFACEMETHODIMP GetCommandString(UINT_PTR cmd, UINT type, UINT*, LPSTR out, UINT cch) override
        {
            const auto shown = Shown();
            if (cmd >= shown.size()) return E_INVALIDARG;
            const wchar_t* verb = Verbs[shown[cmd]];
            if (type == GCS_VERBW) return StringCchCopyW((LPWSTR)out, cch, verb);
            if (type == GCS_VERBA) return StringCchCopyA(out, cch, v31np::Narrow(verb).c_str());
            return E_NOTIMPL;
        }
    };

    HBITMAP MenuBitmap()
    {
        static HBITMAP made = []() -> HBITMAP
        {
            const int cx = GetSystemMetrics(SM_CXSMICON);
            const int cy = GetSystemMetrics(SM_CYSMICON);
            HICON icon = (HICON)LoadImageW(nullptr, IconPath().c_str(), IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
            if (!icon) return nullptr;
            HBITMAP out = nullptr;
            ComPtr<IWICImagingFactory> wic;
            ComPtr<IWICBitmap> src;
            ComPtr<IWICFormatConverter> conv;
            if (SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wic)))
                && SUCCEEDED(wic->CreateBitmapFromHICON(icon, &src))
                && SUCCEEDED(wic->CreateFormatConverter(&conv))
                && SUCCEEDED(conv->Initialize(src.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)))
            {
                BITMAPINFO bi = {};
                bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
                bi.bmiHeader.biWidth = cx;
                bi.bmiHeader.biHeight = -cy;
                bi.bmiHeader.biPlanes = 1;
                bi.bmiHeader.biBitCount = 32;
                bi.bmiHeader.biCompression = BI_RGB;
                void* bits = nullptr;
                out = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
                if (out && FAILED(conv->CopyPixels(nullptr, (UINT)cx * 4, (UINT)(cx * cy * 4), (BYTE*)bits)))
                {
                    DeleteObject(out);
                    out = nullptr;
                }
            }
            DestroyIcon(icon);
            return out;
        }();
        return made;
    }

    void AddItem(HMENU menu, UINT at, UINT id, const std::wstring& label)
    {
        MENUITEMINFOW mii = { sizeof(mii) };
        mii.fMask = MIIM_STRING | MIIM_ID | MIIM_BITMAP;
        mii.wID = id;
        mii.dwTypeData = (LPWSTR)label.c_str();
        mii.hbmpItem = MenuBitmap();
        InsertMenuItemW(menu, at, TRUE, &mii);
    }

    class PackMenu final : public IShellExtInit, public IContextMenu
    {
        REFS

    public:
        std::vector<std::wstring> paths;

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(PackMenu, IShellExtInit),
                QITABENT(PackMenu, IContextMenu),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP Initialize(PCIDLIST_ABSOLUTE, IDataObject* data, HKEY) override
        {
            paths = DropPaths(data);
            if (std::all_of(paths.begin(), paths.end(), [](const std::wstring& p) { return IsPack(p); })) return E_FAIL;
            return S_OK;
        }

        IFACEMETHODIMP QueryContextMenu(HMENU menu, UINT index, UINT first, UINT, UINT flags) override
        {
            if (flags & CMF_DEFAULTONLY) return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
            AddItem(menu, index, first, L"Ba\u00EDb\u00EDz");
            return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 1);
        }

        IFACEMETHODIMP InvokeCommand(CMINVOKECOMMANDINFO* info) override
        {
            if (IS_INTRESOURCE(info->lpVerb))
            {
                if (LOWORD(info->lpVerb) != 0) return E_INVALIDARG;
            }
            else
            {
                const CMINVOKECOMMANDINFOEX* ex = (const CMINVOKECOMMANDINFOEX*)info;
                const bool wide = info->cbSize >= sizeof(CMINVOKECOMMANDINFOEX) && (info->fMask & CMIC_MASK_UNICODE) && ex->lpVerbW;
                if (wide ? _wcsicmp(ex->lpVerbW, L"v31np.pack") != 0 : _stricmp(info->lpVerb, "v31np.pack") != 0) return E_INVALIDARG;
            }
            PackAsync(paths);
            return S_OK;
        }

        IFACEMETHODIMP GetCommandString(UINT_PTR cmd, UINT type, UINT*, LPSTR out, UINT cch) override
        {
            if (cmd != 0) return E_INVALIDARG;
            if (type == GCS_VERBW) return StringCchCopyW((LPWSTR)out, cch, L"v31np.pack");
            if (type == GCS_VERBA) return StringCchCopyA(out, cch, "v31np.pack");
            return E_NOTIMPL;
        }
    };

    class Callback final : public IContextMenuCB
    {
        REFS

    public:
        Place place;
        PIDLIST_ABSOLUTE root = nullptr;
        std::vector<std::wstring> names;

        ~Callback()
        {
            CoTaskMemFree(root);
        }

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(Callback, IContextMenuCB),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP CallBack(IShellFolder*, HWND hwnd, IDataObject*, UINT msg, WPARAM wParam, LPARAM) override
        {
            if (msg == DFM_MERGECONTEXTMENU) return S_OK;
            if (msg != DFM_INVOKECOMMAND) return E_NOTIMPL;
            if (wParam == DFM_CMD_DELETE && !names.empty())
            {
                HRESULT r = RemoveNodes(place, names);
                if (SUCCEEDED(r)) Changed(root);
                return r;
            }
            if (wParam == DFM_CMD_PASTE)
            {
                ComPtr<IDataObject> clip;
                if (FAILED(OleGetClipboard(&clip))) return E_FAIL;
                return PasteItems(clip.Get(), place, root, hwnd) ? S_OK : E_FAIL;
            }
            return S_FALSE;
        }
    };

    HRESULT MakeDefault(const Place& place, PCIDLIST_ABSOLUTE root, IShellFolder* folder, HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, ComPtr<IContextMenu>& out)
    {
        Callback* cb = new (std::nothrow) Callback();
        if (!cb) return E_OUTOFMEMORY;
        cb->place = place;
        cb->root = root ? ILCloneFull(root) : nullptr;
        for (UINT i = 0; i < cidl; i++)
            if (const Id* id = AsId(apidl[i])) cb->names.push_back(IdNatural(id));
        ComPtr<IQueryAssociations> assoc;
        if (cidl > 0) folder->GetUIObjectOf(hwnd, 1, apidl, IID_IQueryAssociations, nullptr, (void**)assoc.GetAddressOf());
        DEFCONTEXTMENU dcm = {};
        dcm.hwnd = hwnd;
        dcm.pcmcb = cb;
        dcm.pidlFolder = root;
        dcm.psf = folder;
        dcm.cidl = cidl;
        dcm.apidl = apidl;
        dcm.punkAssociationInfo = assoc.Get();
        HRESULT r = SHCreateDefaultContextMenu(&dcm, IID_PPV_ARGS(&out));
        cb->Release();
        return r;
    }

    class ItemMenu final : public IContextMenu3, public IObjectWithSite
    {
        REFS

    public:
        ComPtr<IContextMenu> inner;
        Place place;
        PIDLIST_ABSOLUTE root = nullptr;
        std::vector<std::wstring> names;
        bool plain = true;
        UINT used = 0;
        bool pack = false;

        ~ItemMenu()
        {
            CoTaskMemFree(root);
        }

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(ItemMenu, IContextMenu3),
                QITABENTMULTI(ItemMenu, IContextMenu2, IContextMenu3),
                QITABENTMULTI(ItemMenu, IContextMenu, IContextMenu3),
                QITABENT(ItemMenu, IObjectWithSite),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP QueryContextMenu(HMENU menu, UINT index, UINT first, UINT last, UINT flags) override
        {
            HRESULT r = inner->QueryContextMenu(menu, index, first, last, flags);
            pack = false;
            if (FAILED(r)) return r;
            used = HRESULT_CODE(r);
            if (flags & (CMF_DEFAULTONLY | CMF_VERBSONLY)) return r;
            for (int i = GetMenuItemCount(menu) - 1; i >= 0; i--)
            {
                const UINT id = GetMenuItemID(menu, i);
                if (id == (UINT)-1 || id < first || id >= first + used) continue;
                wchar_t verb[128] = {};
                if (FAILED(inner->GetCommandString(id - first, GCS_VERBW, nullptr, (LPSTR)verb, ARRAYSIZE(verb)))) continue;
                if (_wcsicmp(verb, L"Windows.ModernShare") == 0 || _wcsicmp(verb, L"Windows.Share") == 0) DeleteMenu(menu, i, MF_BYPOSITION);
            }
            if (first + used > last) return r;
            const int had = GetMenuItemCount(menu);
            InsertMenuW(menu, had > 0 ? (UINT)had - 1 : 0, MF_BYPOSITION | MF_STRING, first + used, L"Baíbíz");
            pack = true;
            return MAKE_HRESULT(SEVERITY_SUCCESS, 0, used + 1);
        }

        std::wstring VerbOf(CMINVOKECOMMANDINFO* info)
        {
            if (!IS_INTRESOURCE(info->lpVerb))
            {
                const CMINVOKECOMMANDINFOEX* ex = (const CMINVOKECOMMANDINFOEX*)info;
                if (info->cbSize >= sizeof(CMINVOKECOMMANDINFOEX) && (info->fMask & CMIC_MASK_UNICODE) && ex->lpVerbW) return ex->lpVerbW;
                return v31np::Wide(info->lpVerb);
            }
            wchar_t buf[128] = {};
            if (FAILED(inner->GetCommandString(LOWORD(info->lpVerb), GCS_VERBW, nullptr, (LPSTR)buf, ARRAYSIZE(buf)))) return {};
            return buf;
        }

        HRESULT OpenCopies(const std::wstring& verb, HWND hwnd)
        {
            auto a = Load(place.path, place.chain);
            if (!a) return E_FAIL;
            size_t here = a->Find(place.sub);
            if (here == v31np::None) return E_FAIL;
            wchar_t temp[MAX_PATH + 1];
            if (!GetTempPathW(ARRAYSIZE(temp), temp)) return E_FAIL;
            const std::wstring base = std::wstring(temp) + L"v31np";
            Sweep(base);
            GUID g;
            CoCreateGuid(&g);
            wchar_t tag[64];
            StringFromGUID2(g, tag, ARRAYSIZE(tag));
            const std::wstring dir = base + L"\\" + tag;
            if (SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr) != ERROR_SUCCESS) return E_FAIL;
            std::vector<Copy> copies;
            HRESULT r = S_OK;
            for (const auto& name : names)
            {
                const std::wstring f = v31np::Fold(name);
                size_t node = v31np::None;
                for (size_t k : a->index->tree.nodes[here].kids)
                    if (v31np::Fold(a->index->tree.nodes[k].natural) == f) node = k;
                if (node == v31np::None) continue;
                ComPtr<IStream> src;
                if (FAILED(CreateEntryStream(a, node, &src)))
                {
                    r = E_FAIL;
                    break;
                }
                const std::wstring path = dir + L"\\" + a->index->tree.nodes[node].natural;
                HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (h == INVALID_HANDLE_VALUE)
                {
                    r = HRESULT_FROM_WIN32(GetLastError());
                    break;
                }
                std::vector<BYTE> buf(1 << 20);
                bool ok = true;
                for (;;)
                {
                    ULONG got = 0;
                    if (FAILED(src->Read(buf.data(), (ULONG)buf.size(), &got)))
                    {
                        ok = false;
                        break;
                    }
                    if (got == 0) break;
                    DWORD put = 0;
                    if (!WriteFile(h, buf.data(), got, &put, nullptr) || put != got)
                    {
                        ok = false;
                        break;
                    }
                }
                CloseHandle(h);
                if (!ok)
                {
                    r = E_FAIL;
                    break;
                }
                Copy c;
                c.file = path;
                c.key = Join(StoredSegments(a->index->tree, node));
                if (Stat(path, c.size, c.time) && HashFile(path, c.basis)) copies.push_back(c);
                SHELLEXECUTEINFOW sei = { sizeof(sei) };
                sei.hwnd = hwnd;
                sei.lpVerb = verb.empty() ? nullptr : verb.c_str();
                sei.lpFile = path.c_str();
                sei.nShow = SW_SHOWNORMAL;
                ShellExecuteExW(&sei);
            }
            a.reset();
            if (!copies.empty()) Follow(place, dir, std::move(copies), root ? ILCloneFull(root) : nullptr);
            return r;
        }

        IFACEMETHODIMP InvokeCommand(CMINVOKECOMMANDINFO* info) override
        {
            // Delete not — Explorer may call our entry by its verb NAME instead of its number. Miss the name and it falls
            // through to the « open with » path: temp copies, and one « no app for this » error per file.
            if (pack && ((IS_INTRESOURCE(info->lpVerb) && LOWORD(info->lpVerb) == used) || (!IS_INTRESOURCE(info->lpVerb) && VerbOf(info) == L"v31np.pack")))
            {
                HRESULT r = PackNodes(place, names);
                if (SUCCEEDED(r)) Changed(root);
                return r;
            }
            const std::wstring verb = VerbOf(info);
            static const wchar_t* const Own[] = { L"copy", L"cut", L"paste", L"delete", L"rename", L"properties", L"link", L"pastelink", L"pastespecial" };
            bool own = false;
            for (auto v : Own)
                if (_wcsicmp(verb.c_str(), v) == 0) own = true;
            if (plain && !verb.empty() && !own) return OpenCopies(verb, info->hwnd);
            return inner->InvokeCommand(info);
        }

        IFACEMETHODIMP GetCommandString(UINT_PTR cmd, UINT type, UINT* reserved, LPSTR out, UINT cch) override
        {
            if (pack && cmd == used)
            {
                if (type == GCS_VERBW) return StringCchCopyW((LPWSTR)out, cch, L"v31np.pack");
                if (type == GCS_VERBA) return StringCchCopyA(out, cch, "v31np.pack");
                return E_NOTIMPL;
            }
            return inner->GetCommandString(cmd, type, reserved, out, cch);
        }

        IFACEMETHODIMP SetSite(IUnknown* site) override
        {
            return IUnknown_SetSite(inner.Get(), site);
        }

        IFACEMETHODIMP GetSite(REFIID riid, void** ppv) override
        {
            return IUnknown_GetSite(inner.Get(), riid, ppv);
        }

        IFACEMETHODIMP HandleMenuMsg(UINT msg, WPARAM w, LPARAM l) override
        {
            ComPtr<IContextMenu2> m;
            if (FAILED(inner.As(&m))) return E_NOTIMPL;
            return m->HandleMenuMsg(msg, w, l);
        }

        IFACEMETHODIMP HandleMenuMsg2(UINT msg, WPARAM w, LPARAM l, LRESULT* result) override
        {
            ComPtr<IContextMenu3> m;
            if (FAILED(inner.As(&m))) return E_NOTIMPL;
            return m->HandleMenuMsg2(msg, w, l, result);
        }
    };
}

HRESULT CreateItemMenu(const Place& place, PCIDLIST_ABSOLUTE root, IShellFolder* folder, HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, REFIID riid, void** ppv)
{
    *ppv = nullptr;
    ItemMenu* m = new (std::nothrow) ItemMenu();
    if (!m) return E_OUTOFMEMORY;
    m->place = place;
    m->root = root ? ILCloneFull(root) : nullptr;
    for (UINT i = 0; i < cidl; i++)
        if (const Id* id = AsId(apidl[i]))
        {
            m->names.push_back(IdNatural(id));
            if (id->dir || IsPack(IdNatural(id))) m->plain = false;
        }
    HRESULT r = MakeDefault(place, root, folder, hwnd, cidl, apidl, m->inner);
    if (SUCCEEDED(r)) r = m->QueryInterface(riid, ppv);
    m->Release();
    return r;
}

HRESULT CreateBackMenu(const Place& place, PCIDLIST_ABSOLUTE root, IShellFolder* folder, HWND hwnd, REFIID riid, void** ppv)
{
    *ppv = nullptr;
    ComPtr<IContextMenu> menu;
    HRESULT r = MakeDefault(place, root, folder, hwnd, 0, nullptr, menu);
    if (FAILED(r)) return r;
    return WrapBackMenu(place, root, menu.Get(), riid, ppv);
}

HRESULT CreatePackMenu(REFIID riid, void** ppv)
{
    *ppv = nullptr;
    PackMenu* m = new (std::nothrow) PackMenu();
    if (!m) return E_OUTOFMEMORY;
    HRESULT r = m->QueryInterface(riid, ppv);
    m->Release();
    return r;
}

HRESULT CreateFileMenu(REFIID riid, void** ppv)
{
    *ppv = nullptr;
    FileMenu* m = new (std::nothrow) FileMenu();
    if (!m) return E_OUTOFMEMORY;
    HRESULT r = m->QueryInterface(riid, ppv);
    m->Release();
    return r;
}
