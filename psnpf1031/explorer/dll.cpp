#include "shell.h"

HINSTANCE Module = nullptr;
std::atomic<long> Locks{ 0 };
const CLSID ClsidFolder = { 0xd4b5bf5a, 0x7301, 0x4c74, { 0x94, 0x2f, 0x66, 0xd1, 0xd2, 0xc6, 0x8e, 0xc1 } };
const CLSID ClsidMenu = { 0xe8ec8617, 0x4d37, 0x43a1, { 0xa8, 0xbc, 0x7a, 0xc3, 0x78, 0x9c, 0x26, 0x88 } };
const CLSID ClsidSend = { 0xb92ec11e, 0xb08e, 0x4c3a, { 0xb3, 0x7d, 0x97, 0xef, 0xdb, 0x9a, 0x07, 0x91 } };
const CLSID ClsidPack = { 0x315e5677, 0x6dde, 0x474f, { 0xa4, 0xae, 0x21, 0x2e, 0x1c, 0x80, 0x09, 0xfe } };
const CLSID ClsidThumb = { 0x9c37da14, 0xb53a, 0x4fb7, { 0x81, 0x17, 0x85, 0xb5, 0x2c, 0x95, 0x00, 0x8d } };
const wchar_t TypeName[] = L"Baíbízù format ekta kokain";

namespace
{
    const wchar_t ProgId[] = L"v31np.file";
    const wchar_t SendProgId[] = L"v31np.sendto";
    const wchar_t SendExt[] = L".v31npsendto";
    const wchar_t Approved[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved";

    class Factory final : public IClassFactory
    {
        REFS

    public:
        CLSID clsid;

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(Factory, IClassFactory),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP CreateInstance(IUnknown* outer, REFIID riid, void** ppv) override
        {
            *ppv = nullptr;
            if (outer) return CLASS_E_NOAGGREGATION;
            if (clsid == ClsidFolder) return CreateFolder(Place{}, nullptr, riid, ppv);
            if (clsid == ClsidMenu) return CreateFileMenu(riid, ppv);
            if (clsid == ClsidSend) return CreateSend(riid, ppv);
            if (clsid == ClsidPack) return CreatePackMenu(riid, ppv);
            if (clsid == ClsidThumb) return CreateWorker(riid, ppv);
            return CLASS_E_CLASSNOTAVAILABLE;
        }

        IFACEMETHODIMP LockServer(BOOL on) override
        {
            if (on) ++Locks;
            else --Locks;
            return S_OK;
        }
    };

    std::wstring Guid(const CLSID& id)
    {
        wchar_t s[64];
        StringFromGUID2(id, s, ARRAYSIZE(s));
        return s;
    }

    HRESULT Put(const std::wstring& key, const wchar_t* name, const std::wstring& value)
    {
        LSTATUS s = RegSetKeyValueW(HKEY_LOCAL_MACHINE, (L"Software\\Classes\\" + key).c_str(), name, REG_SZ, value.c_str(), (DWORD)((value.size() + 1) * sizeof(wchar_t)));
        return HRESULT_FROM_WIN32(s);
    }

    HRESULT PutExpand(const std::wstring& key, const wchar_t* name, const std::wstring& value)
    {
        LSTATUS s = RegSetKeyValueW(HKEY_LOCAL_MACHINE, (L"Software\\Classes\\" + key).c_str(), name, REG_EXPAND_SZ, value.c_str(), (DWORD)((value.size() + 1) * sizeof(wchar_t)));
        return HRESULT_FROM_WIN32(s);
    }

    HRESULT PutDword(const std::wstring& key, const wchar_t* name, DWORD value)
    {
        LSTATUS s = RegSetKeyValueW(HKEY_LOCAL_MACHINE, (L"Software\\Classes\\" + key).c_str(), name, REG_DWORD, &value, sizeof(value));
        return HRESULT_FROM_WIN32(s);
    }

    void Drop(const std::wstring& key)
    {
        RegDeleteTreeW(HKEY_LOCAL_MACHINE, (L"Software\\Classes\\" + key).c_str());
    }

    std::wstring Self()
    {
        std::wstring path(MAX_PATH, L'\0');
        for (;;)
        {
            DWORD n = GetModuleFileNameW(Module, path.data(), (DWORD)path.size());
            if (n < path.size())
            {
                path.resize(n);
                return path;
            }
            path.resize(path.size() * 2);
        }
    }

    std::wstring Home()
    {
        std::wstring dll = Self();
        dll.resize(dll.find_last_of(L'\\'));
        size_t up = dll.find_last_of(L'\\');
        return up == std::wstring::npos ? dll : dll.substr(0, up);
    }

    HRESULT Server(const CLSID& id, const std::wstring& dll)
    {
        const std::wstring key = L"CLSID\\" + Guid(id);
        HRESULT r = Put(key, nullptr, TypeName);
        if (SUCCEEDED(r)) r = Put(key + L"\\InprocServer32", nullptr, dll);
        if (SUCCEEDED(r)) r = Put(key + L"\\InprocServer32", L"ThreadingModel", L"Apartment");
        if (SUCCEEDED(r))
        {
            const std::wstring g = Guid(id);
            r = HRESULT_FROM_WIN32(RegSetKeyValueW(HKEY_LOCAL_MACHINE, Approved, g.c_str(), REG_SZ, TypeName, (DWORD)sizeof(TypeName)));
        }
        return r;
    }

    HRESULT Isolated(const CLSID& id, const std::wstring& dll)
    {
        const std::wstring g = Guid(id);
        HRESULT r = Put(L"CLSID\\" + g, nullptr, TypeName);
        if (SUCCEEDED(r)) r = Put(L"CLSID\\" + g, L"AppID", g);
        if (SUCCEEDED(r)) r = Put(L"CLSID\\" + g + L"\\InprocServer32", nullptr, dll);
        if (SUCCEEDED(r)) r = Put(L"CLSID\\" + g + L"\\InprocServer32", L"ThreadingModel", L"Both");
        if (SUCCEEDED(r)) r = Put(L"AppID\\" + g, L"DllSurrogate", L"");
        return r;
    }

    HRESULT Template(const std::wstring& path)
    {
        if (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES) return S_OK;
        HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());
        bool ok = v31np::Write({}, [h](const void* b, uint32_t n)
        {
            DWORD put = 0;
            return WriteFile(h, b, n, &put, nullptr) && put == n;
        });
        CloseHandle(h);
        return ok ? S_OK : E_FAIL;
    }
}

std::wstring IconPath()
{
    return Home() + L"\\nzuu.ico";
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        Module = instance;
        DisableThreadLibraryCalls(instance);
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv)
{
    *ppv = nullptr;
    if (clsid != ClsidFolder && clsid != ClsidMenu && clsid != ClsidSend && clsid != ClsidPack && clsid != ClsidThumb) return CLASS_E_CLASSNOTAVAILABLE;
    Factory* f = new (std::nothrow) Factory();
    if (!f) return E_OUTOFMEMORY;
    f->clsid = clsid;
    HRESULT r = f->QueryInterface(riid, ppv);
    f->Release();
    return r;
}

STDAPI DllCanUnloadNow()
{
    return Locks == 0 ? S_OK : S_FALSE;
}

STDAPI DllRegisterServer()
{
    const std::wstring dll = Self();
    const std::wstring home = Home();
    const std::wstring icon = IconPath();
    const std::wstring empty = home + L"\\empty.v31np";
    HRESULT r = Template(empty);
    if (SUCCEEDED(r)) r = Server(ClsidFolder, dll);
    if (SUCCEEDED(r)) r = Server(ClsidMenu, dll);
    if (SUCCEEDED(r)) r = Server(ClsidSend, dll);
    if (SUCCEEDED(r)) r = Server(ClsidPack, dll);
    if (SUCCEEDED(r)) r = Isolated(ClsidThumb, dll);
    if (SUCCEEDED(r)) r = PutDword(L"CLSID\\" + Guid(ClsidFolder) + L"\\ShellFolder", L"Attributes", 0x200001a0);
    if (SUCCEEDED(r)) r = Put(L"CLSID\\" + Guid(ClsidFolder) + L"\\Implemented Categories\\{00021490-0000-0000-C000-000000000046}", nullptr, L"");
    if (SUCCEEDED(r)) r = Put(L"CLSID\\" + Guid(ClsidFolder) + L"\\ProgID", nullptr, ProgId);
    if (SUCCEEDED(r)) r = Put(L".v31np", nullptr, ProgId);
    if (SUCCEEDED(r)) r = Put(L".v31np\\ShellNew", L"FileName", empty);
    if (SUCCEEDED(r)) r = Put(ProgId, nullptr, TypeName);
    if (SUCCEEDED(r)) r = Put(std::wstring(ProgId) + L"\\DefaultIcon", nullptr, icon);
    if (SUCCEEDED(r)) r = Put(std::wstring(ProgId) + L"\\CLSID", nullptr, Guid(ClsidFolder));
    if (SUCCEEDED(r)) r = Put(std::wstring(ProgId) + L"\\shellex\\ContextMenuHandlers\\v31np", nullptr, Guid(ClsidMenu));
    if (SUCCEEDED(r)) r = Put(std::wstring(ProgId) + L"\\Shell\\Open", L"MultiSelectModel", L"Document");
    if (SUCCEEDED(r)) r = PutExpand(std::wstring(ProgId) + L"\\Shell\\Open\\Command", nullptr, L"%SystemRoot%\\Explorer.exe /idlist,%I,%L");
    if (SUCCEEDED(r)) r = Put(std::wstring(ProgId) + L"\\Shell\\Open\\Command", L"DelegateExecute", L"{11dbb47c-a525-400b-9e80-a54615a090c0}");
    if (SUCCEEDED(r)) r = Put(L"AllFilesystemObjects\\shellex\\ContextMenuHandlers\\v31np", nullptr, Guid(ClsidPack));
    if (SUCCEEDED(r)) r = Put(SendExt, nullptr, SendProgId);
    if (SUCCEEDED(r)) r = Put(SendProgId, nullptr, TypeName);
    if (SUCCEEDED(r)) r = Put(SendProgId, L"NeverShowExt", L"");
    if (SUCCEEDED(r)) r = Put(std::wstring(SendProgId) + L"\\DefaultIcon", nullptr, icon);
    if (SUCCEEDED(r)) r = Put(std::wstring(SendProgId) + L"\\shellex\\DropHandler", nullptr, Guid(ClsidSend));
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    return r;
}

STDAPI DllUnregisterServer()
{
    Drop(L"AllFilesystemObjects\\shellex\\ContextMenuHandlers\\v31np");
    for (const CLSID* id : { &ClsidFolder, &ClsidMenu, &ClsidSend, &ClsidPack })
    {
        Drop(L"CLSID\\" + Guid(*id));
        RegDeleteKeyValueW(HKEY_LOCAL_MACHINE, Approved, Guid(*id).c_str());
    }
    Drop(L"CLSID\\" + Guid(ClsidThumb));
    Drop(L"AppID\\" + Guid(ClsidThumb));
    Drop(L".v31np");
    Drop(ProgId);
    Drop(SendExt);
    Drop(SendProgId);
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    return S_OK;
}
