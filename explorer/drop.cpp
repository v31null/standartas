#include "shell.h"

#include <thread>

namespace
{
    bool HasFiles(IDataObject* data)
    {
        FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        return data && data->QueryGetData(&fmt) == S_OK;
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

    class DropTarget final : public IDropTarget
    {
        REFS

    public:
        Place place;
        PIDLIST_ABSOLUTE pidl = nullptr;
        DWORD allowed = DROPEFFECT_NONE;

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

        IFACEMETHODIMP DragEnter(IDataObject* data, DWORD, POINTL, DWORD* effect) override
        {
            allowed = HasFiles(data) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
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
            if (paths.empty())
            {
                *effect = DROPEFFECT_NONE;
                return S_OK;
            }
            *effect = DROPEFFECT_COPY;
            Place p = place;
            PIDLIST_ABSOLUTE at = pidl ? ILCloneFull(pidl) : nullptr;
            std::thread([p, paths, at]()
            {
                Lock hold;
                if (SUCCEEDED(AddPaths(p, paths))) Changed(at);
                CoTaskMemFree(at);
            }).detach();
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
            allowed = HasFiles(data) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
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
            if (paths.empty())
            {
                *effect = DROPEFFECT_NONE;
                return S_OK;
            }
            *effect = DROPEFFECT_COPY;
            PackAsync(paths);
            return S_OK;
        }
    };
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
