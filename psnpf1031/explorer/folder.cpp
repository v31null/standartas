#include "shell.h"

#include <thumbcache.h>
#include <wincodec.h>

#include <algorithm>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "gdi32.lib")

namespace
{
    const PROPERTYKEY Columns[] = { PKEY_ItemNameDisplay, PKEY_ItemTypeText, PKEY_Size };

    class Enum final : public IEnumIDList
    {
        REFS

    public:
        std::vector<PITEMID_CHILD> ids;
        size_t at = 0;

        ~Enum()
        {
            for (auto id : ids) CoTaskMemFree(id);
        }

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(Enum, IEnumIDList),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP Next(ULONG celt, PITEMID_CHILD* rgelt, ULONG* fetched) override
        {
            ULONG n = 0;
            while (n < celt && at < ids.size())
            {
                rgelt[n] = (PITEMID_CHILD)ILClone(ids[at]);
                if (!rgelt[n]) break;
                n++;
                at++;
            }
            if (fetched) *fetched = n;
            return n == celt ? S_OK : S_FALSE;
        }

        IFACEMETHODIMP Skip(ULONG celt) override
        {
            at = std::min<size_t>(ids.size(), at + celt);
            return at < ids.size() ? S_OK : S_FALSE;
        }

        IFACEMETHODIMP Reset() override
        {
            at = 0;
            return S_OK;
        }

        IFACEMETHODIMP Clone(IEnumIDList**) override
        {
            return E_NOTIMPL;
        }
    };

    std::mutex typeGuard;
    std::map<std::wstring, std::wstring> types;

    std::wstring TypeOf(const std::wstring& natural, bool dir)
    {
        const size_t dot = natural.rfind(L'.');
        const std::wstring key = dir ? L"\\" : dot == std::wstring::npos ? L"" : v31np::Fold(natural.substr(dot));
        {
            std::lock_guard<std::mutex> g(typeGuard);
            auto it = types.find(key);
            if (it != types.end()) return it->second;
        }
        SHFILEINFOW sfi{};
        SHGetFileInfoW(natural.c_str(), dir ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);
        std::lock_guard<std::mutex> g(typeGuard);
        return types.emplace(key, sfi.szTypeName).first->second;
    }

    HRESULT Decode(IStream* s, UINT cx, HBITMAP* out)
    {
        ComPtr<IWICImagingFactory> f;
        ComPtr<IWICBitmapDecoder> dec;
        ComPtr<IWICBitmapFrameDecode> frame;
        ComPtr<IWICBitmapScaler> scale;
        ComPtr<IWICFormatConverter> conv;
        UINT w = 0;
        UINT h = 0;
        HRESULT r = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f));
        if (SUCCEEDED(r)) r = f->CreateDecoderFromStream(s, nullptr, WICDecodeMetadataCacheOnDemand, &dec);
        if (SUCCEEDED(r)) r = dec->GetFrame(0, &frame);
        if (SUCCEEDED(r)) r = frame->GetSize(&w, &h);
        if (SUCCEEDED(r) && (!w || !h)) r = E_FAIL;
        if (FAILED(r)) return r;
        const double k = std::min(1.0, (double)cx / std::max(w, h));
        const UINT tw = std::max(1u, (UINT)(w * k));
        const UINT th = std::max(1u, (UINT)(h * k));
        r = f->CreateBitmapScaler(&scale);
        if (SUCCEEDED(r)) r = scale->Initialize(frame.Get(), tw, th, WICBitmapInterpolationModeFant);
        if (SUCCEEDED(r)) r = f->CreateFormatConverter(&conv);
        if (SUCCEEDED(r)) r = conv->Initialize(scale.Get(), GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom);
        if (FAILED(r)) return r;
        BITMAPINFO bi{};
        bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
        bi.bmiHeader.biWidth = (LONG)tw;
        bi.bmiHeader.biHeight = -(LONG)th;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;
        void* bits = nullptr;
        HBITMAP b = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!b) return E_OUTOFMEMORY;
        r = conv->CopyPixels(nullptr, tw * 4, tw * 4 * th, (BYTE*)bits);
        if (FAILED(r))
        {
            DeleteObject(b);
            return r;
        }
        *out = b;
        return S_OK;
    }

    class Thumbnail final : public IThumbnailProvider
    {
        REFS

    public:
        ComPtr<IThumbnailProvider> inner;
        ComPtr<IStream> stream;
        CLSID props{};
        bool hasProps = false;
        std::shared_ptr<const Index> index;
        size_t entry = 0;

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(Thumbnail, IThumbnailProvider),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP* bmp, WTS_ALPHATYPE* alpha) override
        {
            const HRESULT r = Extract(cx, bmp, alpha);
            if (FAILED(r)) index->blank[entry] = true;
            return r;
        }

        HRESULT Extract(UINT cx, HBITMAP* bmp, WTS_ALPHATYPE* alpha)
        {
            *bmp = nullptr;
            if (inner && SUCCEEDED(inner->GetThumbnail(cx, bmp, alpha)) && *bmp) return S_OK;
            LARGE_INTEGER zero{};
            if (SUCCEEDED(stream->Seek(zero, STREAM_SEEK_SET, nullptr)) && SUCCEEDED(Decode(stream.Get(), cx, bmp)))
            {
                *alpha = WTSAT_ARGB;
                return S_OK;
            }
            if (!hasProps) return E_FAIL;
            stream->Seek(zero, STREAM_SEEK_SET, nullptr);
            ComPtr<IInitializeWithStream> init;
            ComPtr<IPropertyStore> store;
            HRESULT r = CoCreateInstance(props, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&init));
            if (SUCCEEDED(r)) r = init->Initialize(stream.Get(), STGM_READ);
            if (SUCCEEDED(r)) r = init.As(&store);
            PROPVARIANT v;
            PropVariantInit(&v);
            if (SUCCEEDED(r)) r = store->GetValue(PKEY_ThumbnailStream, &v);
            if (SUCCEEDED(r) && (v.vt != VT_STREAM || !v.pStream)) r = E_FAIL;
            if (SUCCEEDED(r)) r = Decode(v.pStream, cx, bmp);
            PropVariantClear(&v);
            if (SUCCEEDED(r)) *alpha = WTSAT_ARGB;
            return r;
        }
    };

    class Remote final : public IThumbnailProvider
    {
        REFS

    public:
        std::wstring ids;
        std::shared_ptr<const Index> index;
        size_t entry = 0;

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(Remote, IThumbnailProvider),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP* bmp, WTS_ALPHATYPE* alpha) override
        {
            *bmp = nullptr;
            ComPtr<IInitializeWithFile> init;
            ComPtr<IThumbnailProvider> worker;
            HRESULT r = CoCreateInstance(ClsidThumb, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&init));
            if (SUCCEEDED(r)) r = init->Initialize(ids.c_str(), STGM_READ);
            if (SUCCEEDED(r)) r = init.As(&worker);
            if (FAILED(r)) return r;
            r = worker->GetThumbnail(cx, bmp, alpha);
            if (FAILED(r)) index->blank[entry] = true;
            return r;
        }
    };

    std::wstring Hex(PCIDLIST_ABSOLUTE p)
    {
        static const wchar_t digits[] = L"0123456789abcdef";
        const BYTE* b = (const BYTE*)p;
        const UINT n = ILGetSize(p);
        std::wstring s((size_t)n * 2, L'0');
        for (UINT i = 0; i < n; i++)
        {
            s[2 * (size_t)i] = digits[b[i] >> 4];
            s[2 * (size_t)i + 1] = digits[b[i] & 15];
        }
        return s;
    }

    bool Unhex(const wchar_t* s, std::vector<BYTE>& out)
    {
        auto digit = [](wchar_t c) { return c >= L'0' && c <= L'9' ? c - L'0' : c >= L'a' && c <= L'f' ? c - L'a' + 10 : -1; };
        const size_t len = wcslen(s);
        if (len < 2 * sizeof(USHORT) || len % 2) return false;
        out.resize(len / 2);
        for (size_t i = 0; i < out.size(); i++)
        {
            const int hi = digit(s[2 * i]);
            const int lo = digit(s[2 * i + 1]);
            if (hi < 0 || lo < 0) return false;
            out[i] = (BYTE)(hi << 4 | lo);
        }
        size_t at = 0;
        for (;;)
        {
            if (at + sizeof(USHORT) > out.size()) return false;
            USHORT cb;
            memcpy(&cb, out.data() + at, sizeof(cb));
            if (cb == 0) return at + sizeof(USHORT) == out.size();
            if (cb < sizeof(USHORT)) return false;
            at += cb;
        }
    }

    HRESULT HandlerOf(const std::wstring& natural, CLSID* clsid)
    {
        const size_t dot = natural.rfind(L'.');
        if (dot == std::wstring::npos) return E_FAIL;
        wchar_t text[64];
        DWORD n = ARRAYSIZE(text);
        HRESULT r = AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_SHELLEXTENSION, natural.substr(dot).c_str(), L"{e357fccd-a995-4576-b01f-234630154e96}", text, &n);
        return SUCCEEDED(r) ? CLSIDFromString(text, clsid) : r;
    }

    HRESULT Ret(const std::wstring& s, STRRET* out)
    {
        out->uType = STRRET_WSTR;
        return SHStrDupW(s.c_str(), &out->pOleStr);
    }

    class PackFolder final : public IShellFolder2, public IPersistFolder3, public IThumbnailHandlerFactory, public IStorage
    {
        REFS

    public:
        Place place;
        PIDLIST_ABSOLUTE root = nullptr;

        ~PackFolder()
        {
            CoTaskMemFree(root);
        }

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            if (riid == IID_Store)
            {
                *ppv = nullptr;
                return CreateStore(place, root, (IStorage**)ppv);
            }
            static const QITAB qit[] = {
                QITABENT(PackFolder, IShellFolder2),
                QITABENTMULTI(PackFolder, IShellFolder, IShellFolder2),
                QITABENT(PackFolder, IPersistFolder3),
                QITABENTMULTI(PackFolder, IPersistFolder2, IPersistFolder3),
                QITABENTMULTI(PackFolder, IPersistFolder, IPersistFolder3),
                QITABENTMULTI(PackFolder, IPersist, IPersistFolder3),
                QITABENT(PackFolder, IThumbnailHandlerFactory),
                QITABENT(PackFolder, IStorage),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        HRESULT Stg(ComPtr<IStorage>& s)
        {
            return CreateStore(place, root, s.GetAddressOf());
        }

        IFACEMETHODIMP CreateStream(LPCOLESTR name, DWORD mode, DWORD r1, DWORD r2, IStream** out) override
        {
            *out = nullptr;
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->CreateStream(name, mode, r1, r2, out);
        }

        IFACEMETHODIMP OpenStream(LPCOLESTR name, void* r1, DWORD mode, DWORD r2, IStream** out) override
        {
            *out = nullptr;
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->OpenStream(name, r1, mode, r2, out);
        }

        IFACEMETHODIMP CreateStorage(LPCOLESTR name, DWORD mode, DWORD r1, DWORD r2, IStorage** out) override
        {
            *out = nullptr;
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->CreateStorage(name, mode, r1, r2, out);
        }

        IFACEMETHODIMP OpenStorage(LPCOLESTR name, IStorage* prio, DWORD mode, SNB snb, DWORD r1, IStorage** out) override
        {
            *out = nullptr;
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->OpenStorage(name, prio, mode, snb, r1, out);
        }

        IFACEMETHODIMP CopyTo(DWORD n, const IID* ex, SNB snb, IStorage* dest) override
        {
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->CopyTo(n, ex, snb, dest);
        }

        IFACEMETHODIMP MoveElementTo(LPCOLESTR name, IStorage* dest, LPCOLESTR newName, DWORD flags) override
        {
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->MoveElementTo(name, dest, newName, flags);
        }

        IFACEMETHODIMP Commit(DWORD flags) override
        {
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->Commit(flags);
        }

        IFACEMETHODIMP Revert() override
        {
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->Revert();
        }

        IFACEMETHODIMP EnumElements(DWORD r1, void* r2, DWORD r3, IEnumSTATSTG** out) override
        {
            *out = nullptr;
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->EnumElements(r1, r2, r3, out);
        }

        IFACEMETHODIMP DestroyElement(LPCOLESTR name) override
        {
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->DestroyElement(name);
        }

        IFACEMETHODIMP RenameElement(LPCOLESTR from, LPCOLESTR to) override
        {
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->RenameElement(from, to);
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
            ComPtr<IStorage> s;
            HRESULT r = Stg(s);
            return FAILED(r) ? r : s->Stat(st, flag);
        }

        IFACEMETHODIMP GetThumbnailHandler(PCUITEMID_CHILD pidl, IBindCtx*, REFIID riid, void** ppv) override
        {
            *ppv = nullptr;
            const Id* id = AsId(pidl);
            if (!id || id->dir || IsPack(IdNatural(id))) return E_NOINTERFACE;
            return Thumb(pidl, riid, ppv);
        }

        std::shared_ptr<Archive> Arc(size_t& here)
        {
            auto a = Load(place.path, place.chain);
            here = a ? a->Find(place.sub) : v31np::None;
            return here == v31np::None ? nullptr : a;
        }

        size_t Child(const std::shared_ptr<Archive>& a, size_t here, const Id* id)
        {
            return a->index->Child(here, IdNatural(id));
        }

        uint64_t SizeOf(const std::shared_ptr<Archive>& a, size_t node)
        {
            const v31np::Node& n = a->index->tree.nodes[node];
            return n.dir ? 0 : a->index->entries[n.entry].size;
        }

        bool Writable()
        {
            return true;
        }

        HRESULT Thumb(PCUITEMID_CHILD pidl, REFIID riid, void** ppv)
        {
            const std::wstring natural = IdNatural(AsId(pidl));
            Place next;
            std::shared_ptr<Archive> a;
            size_t node;
            HRESULT r = Descend(pidl, next, a, node);
            if (FAILED(r)) return r;
            const size_t entry = a->index->tree.nodes[node].entry;
            if (a->index->blank[entry]) return E_NOINTERFACE;
            CLSID clsid;
            if (FAILED(HandlerOf(natural, &clsid)) && FAILED(PSLookupPropertyHandlerCLSID(natural.c_str(), &clsid)))
            {
                a->index->blank[entry] = true;
                return E_NOINTERFACE;
            }
            PIDLIST_ABSOLUTE full = ILCombine(root, pidl);
            if (!full) return E_OUTOFMEMORY;
            Remote* t = new (std::nothrow) Remote();
            if (!t)
            {
                CoTaskMemFree(full);
                return E_OUTOFMEMORY;
            }
            t->ids = Hex(full);
            CoTaskMemFree(full);
            t->index = a->index;
            t->entry = entry;
            r = t->QueryInterface(riid, ppv);
            t->Release();
            return r;
        }

        HRESULT Local(PCUITEMID_CHILD pidl, REFIID riid, void** ppv)
        {
            const std::wstring natural = IdNatural(AsId(pidl));
            Place next;
            std::shared_ptr<Archive> a;
            size_t node;
            HRESULT r = Descend(pidl, next, a, node);
            if (FAILED(r)) return r;
            const size_t entry = a->index->tree.nodes[node].entry;
            if (a->index->blank[entry]) return E_NOINTERFACE;
            ComPtr<IStream> s;
            r = CreateEntryStream(a, node, &s);
            if (FAILED(r)) return r;
            ComPtr<IThumbnailProvider> inner;
            CLSID clsid;
            ComPtr<IUnknown> handler;
            if (SUCCEEDED(HandlerOf(natural, &clsid))
                && SUCCEEDED(CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&handler))))
            {
                ComPtr<IInitializeWithStream> byStream;
                ComPtr<IInitializeWithItem> byItem;
                if (SUCCEEDED(handler.As(&byStream)))
                {
                    ComPtr<IStream> own;
                    if (SUCCEEDED(CreateEntryStream(a, node, &own)) && SUCCEEDED(byStream->Initialize(own.Get(), STGM_READ))) handler.As(&inner);
                }
                else if (SUCCEEDED(handler.As(&byItem)))
                {
                    ComPtr<IShellItem> item;
                    if (SUCCEEDED(SHCreateItemWithParent(root, this, pidl, IID_PPV_ARGS(&item))) && SUCCEEDED(byItem->Initialize(item.Get(), STGM_READ))) handler.As(&inner);
                }
            }
            CLSID props;
            const bool hasProps = SUCCEEDED(PSLookupPropertyHandlerCLSID(natural.c_str(), &props));
            if (!inner && !hasProps)
            {
                a->index->blank[entry] = true;
                return E_NOINTERFACE;
            }
            Thumbnail* t = new (std::nothrow) Thumbnail();
            if (!t) return E_OUTOFMEMORY;
            t->inner = inner;
            t->stream = s;
            t->props = props;
            t->hasProps = hasProps;
            t->index = a->index;
            t->entry = entry;
            r = t->QueryInterface(riid, ppv);
            t->Release();
            return r;
        }

        IFACEMETHODIMP GetClassID(CLSID* clsid) override
        {
            *clsid = ClsidFolder;
            return S_OK;
        }

        IFACEMETHODIMP Initialize(PCIDLIST_ABSOLUTE pidl) override
        {
            return InitializeEx(nullptr, pidl, nullptr);
        }

        IFACEMETHODIMP GetCurFolder(PIDLIST_ABSOLUTE* ppidl) override
        {
            *ppidl = root ? ILCloneFull(root) : nullptr;
            return *ppidl ? S_OK : E_FAIL;
        }

        IFACEMETHODIMP InitializeEx(IBindCtx*, PCIDLIST_ABSOLUTE pidlRoot, const PERSIST_FOLDER_TARGET_INFO* target) override
        {
            CoTaskMemFree(root);
            root = ILCloneFull(pidlRoot);
            if (!root) return E_OUTOFMEMORY;
            if (!place.path.empty()) return S_OK;
            PWSTR path = nullptr;
            HRESULT r;
            if (target && target->pidlTargetFolder) r = SHGetNameFromIDList(target->pidlTargetFolder, SIGDN_FILESYSPATH, &path);
            else if (target && target->szTargetParsingName[0]) r = SHStrDupW(target->szTargetParsingName, &path);
            else r = SHGetNameFromIDList(pidlRoot, SIGDN_FILESYSPATH, &path);
            if (FAILED(r)) return r;
            place.path = path;
            place.chain.clear();
            place.sub.clear();
            CoTaskMemFree(path);
            return S_OK;
        }

        IFACEMETHODIMP GetFolderTargetInfo(PERSIST_FOLDER_TARGET_INFO*) override
        {
            return E_NOTIMPL;
        }

        IFACEMETHODIMP ParseDisplayName(HWND, IBindCtx*, LPWSTR name, ULONG* eaten, PIDLIST_RELATIVE* ppidl, ULONG* attrs) override
        {
            *ppidl = nullptr;
            size_t here;
            auto a = Arc(here);
            if (!a) return E_FAIL;
            PIDLIST_RELATIVE result = nullptr;
            std::wstring s = name;
            size_t i = 0;
            size_t cur = here;
            std::vector<std::string> chain = place.chain;
            while (i < s.size())
            {
                size_t j = s.find_first_of(L"\\/", i);
                if (j == std::wstring::npos) j = s.size();
                std::wstring part = s.substr(i, j - i);
                i = j + 1;
                if (part.empty()) continue;
                if (!a->index->tree.nodes[cur].dir)
                {
                    if (!IsPack(a->index->tree.nodes[cur].natural))
                    {
                        CoTaskMemFree(result);
                        return E_INVALIDARG;
                    }
                    chain.push_back(a->index->entries[a->index->tree.nodes[cur].entry].key);
                    a = Load(place.path, chain);
                    if (!a)
                    {
                        CoTaskMemFree(result);
                        return HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
                    }
                    cur = 0;
                }
                const std::wstring f = v31np::Fold(part);
                size_t next = v31np::None;
                for (size_t k : a->index->tree.nodes[cur].kids)
                    if (v31np::Fold(a->index->tree.nodes[k].natural) == f || a->index->tree.nodes[k].stored == part)
                    {
                        next = k;
                        break;
                    }
                if (next == v31np::None)
                {
                    CoTaskMemFree(result);
                    return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                }
                PITEMID_CHILD id = MakeId(a->index->tree.nodes[next], SizeOf(a, next));
                if (!id)
                {
                    CoTaskMemFree(result);
                    return E_OUTOFMEMORY;
                }
                PIDLIST_RELATIVE joined = result ? ILCombine(result, id) : (PIDLIST_RELATIVE)ILClone(id);
                CoTaskMemFree(id);
                CoTaskMemFree(result);
                result = joined;
                if (!result) return E_OUTOFMEMORY;
                cur = next;
            }
            if (!result) return E_INVALIDARG;
            if (eaten) *eaten = (ULONG)s.size();
            *ppidl = result;
            if (attrs && *attrs)
            {
                PCUITEMID_CHILD last = ILFindLastID(result);
                GetAttributesOf(1, &last, attrs);
            }
            return S_OK;
        }

        IFACEMETHODIMP EnumObjects(HWND, SHCONTF flags, IEnumIDList** out) override
        {
            *out = nullptr;
            size_t here;
            auto a = Arc(here);
            if (!a) return E_FAIL;
            Enum* e = new (std::nothrow) Enum();
            if (!e) return E_OUTOFMEMORY;
            for (size_t k : a->index->tree.nodes[here].kids)
            {
                const bool dir = a->index->tree.nodes[k].dir;
                if (!(flags & SHCONTF_STORAGE) && (dir ? !(flags & SHCONTF_FOLDERS) : !(flags & SHCONTF_NONFOLDERS))) continue;
                PITEMID_CHILD id = MakeId(a->index->tree.nodes[k], SizeOf(a, k));
                if (id) e->ids.push_back(id);
            }
            *out = e;
            return S_OK;
        }

        HRESULT Descend(PCUIDLIST_RELATIVE pidl, Place& next, std::shared_ptr<Archive>& a, size_t& node)
        {
            next = place;
            size_t here;
            a = Arc(here);
            if (!a) return E_FAIL;
            node = here;
            for (PCUIDLIST_RELATIVE p = pidl; p && p->mkid.cb; p = ILNext(p))
            {
                const Id* id = AsId(p);
                if (!id) return E_INVALIDARG;
                if (!a->index->tree.nodes[node].dir)
                {
                    next.chain.push_back(a->index->entries[a->index->tree.nodes[node].entry].key);
                    next.sub.clear();
                    a = Load(next.path, next.chain);
                    if (!a) return HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
                    node = 0;
                }
                const size_t found = a->index->Child(node, IdNatural(id));
                if (found == v31np::None) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                node = found;
                next.sub.push_back(a->index->tree.nodes[node].natural);
            }
            return S_OK;
        }

        IFACEMETHODIMP BindToObject(PCUIDLIST_RELATIVE pidl, IBindCtx* pbc, REFIID riid, void** ppv) override
        {
            *ppv = nullptr;
            Place next;
            std::shared_ptr<Archive> a;
            size_t node;
            HRESULT r = Descend(pidl, next, a, node);
            if (FAILED(r)) return r;
            const v31np::Node& n = a->index->tree.nodes[node];
            if (!n.dir && (riid == IID_IStream || riid == IID_ISequentialStream))
            {
                IStream* s = nullptr;
                BIND_OPTS bo = { sizeof(bo) };
                if (pbc && SUCCEEDED(pbc->GetBindOptions(&bo)) && (bo.grfMode & (STGM_WRITE | STGM_READWRITE)))
                {
                    const std::wstring natural = n.natural;
                    a.reset();
                    Place up = next;
                    up.sub.pop_back();
                    PIDLIST_ABSOLUTE full = ILCombine(root, pidl);
                    if (!full) return E_OUTOFMEMORY;
                    ILRemoveLastID(full);
                    r = CreateWriteStream(up, full, natural, (bo.grfMode & STGM_CREATE) != 0, &s);
                    CoTaskMemFree(full);
                    if (FAILED(r)) return r;
                    r = s->QueryInterface(riid, ppv);
                    s->Release();
                    return r;
                }
                r = CreateEntryStream(a, node, &s);
                if (FAILED(r)) return r;
                r = s->QueryInterface(riid, ppv);
                s->Release();
                return r;
            }
            if (!n.dir)
            {
                if (!IsPack(n.natural)) return E_NOINTERFACE;
                next.chain.push_back(a->index->entries[n.entry].key);
                next.sub.clear();
                if (!Load(next.path, next.chain)) return HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
            }
            PIDLIST_ABSOLUTE full = ILCombine(root, pidl);
            if (!full) return E_OUTOFMEMORY;
            r = CreateFolder(next, full, riid, ppv);
            CoTaskMemFree(full);
            return r;
        }

        IFACEMETHODIMP BindToStorage(PCUIDLIST_RELATIVE pidl, IBindCtx* pbc, REFIID riid, void** ppv) override
        {
            return BindToObject(pidl, pbc, riid, ppv);
        }

        int Compare(int column, const Id* x, const Id* y)
        {
            if (x->dir != y->dir) return x->dir ? -1 : 1;
            int c = 0;
            if (column == 1) c = StrCmpLogicalW(TypeOf(IdNatural(x), x->dir != 0).c_str(), TypeOf(IdNatural(y), y->dir != 0).c_str());
            else if (column == 2) c = x->size < y->size ? -1 : x->size > y->size ? 1 : 0;
            if (c == 0) c = v31np::Order(IdStored(x), IdStored(y));
            if (c == 0) c = v31np::Order(IdNatural(x), IdNatural(y));
            return c < 0 ? -1 : c > 0 ? 1 : 0;
        }

        IFACEMETHODIMP CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE p1, PCUIDLIST_RELATIVE p2) override
        {
            const int column = (int)(lParam & SHCIDS_COLUMNMASK);
            while (true)
            {
                const Id* x = AsId(p1);
                const Id* y = AsId(p2);
                if (!x || !y) return E_INVALIDARG;
                int c = Compare(column, x, y);
                if (c != 0) return MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(short)c);
                p1 = ILNext(p1);
                p2 = ILNext(p2);
                const bool e1 = !p1 || !p1->mkid.cb;
                const bool e2 = !p2 || !p2->mkid.cb;
                if (e1 || e2) return MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(short)(e1 == e2 ? 0 : e1 ? -1 : 1));
            }
        }

        IFACEMETHODIMP CreateViewObject(HWND hwnd, REFIID riid, void** ppv) override
        {
            *ppv = nullptr;
            if (riid == IID_IShellView)
            {
                SFV_CREATE c = { sizeof(c) };
                c.pshf = this;
                ComPtr<IShellFolderViewCB> cb;
                if (!place.path.empty()) CreateViewCallback(place.path, root, &cb);
                c.psfvcb = cb.Get();
                return SHCreateShellFolderView(&c, (IShellView**)ppv);
            }
            // Delete not — no ITransferDestination on purpose: it only takes content through resource IDs Microsoft keeps
            // in a drawer nobody has found. The copy engine writes through IStorage instead. Windows' zip folder skips it too.
            if (riid == __uuidof(ITransferSource))
                return CreateSource(place, root, riid, ppv);
            if (riid == IID_IDropTarget)
            {
                if (!Writable()) return E_NOINTERFACE;
                return CreateDrop(place, root, riid, ppv);
            }
            if (riid == IID_IContextMenu)
                return CreateBackMenu(place, root, this, hwnd, riid, ppv);
            return E_NOINTERFACE;
        }

        IFACEMETHODIMP GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF* inout) override
        {
            SFGAOF all = ~(SFGAOF)0;
            const SFGAOF edit = Writable() ? (SFGAO_CANDELETE | SFGAO_CANRENAME | SFGAO_CANMOVE) : 0;
            if (cidl == 0) all = SFGAO_FOLDER | SFGAO_HASSUBFOLDER | (Writable() ? SFGAO_DROPTARGET : 0);
            for (UINT i = 0; i < cidl; i++)
            {
                const Id* id = AsId(apidl[i]);
                if (!id) return E_INVALIDARG;
                SFGAOF f = SFGAO_CANCOPY | edit;
                if (id->dir) f |= SFGAO_FOLDER | SFGAO_HASSUBFOLDER | (Writable() ? SFGAO_DROPTARGET : 0);
                else
                {
                    f |= SFGAO_STREAM;
                    if (IsPack(IdNatural(id))) f |= SFGAO_FOLDER | (Writable() ? SFGAO_DROPTARGET : 0);
                }
                all &= f;
            }
            *inout &= all;
            return S_OK;
        }

        IFACEMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, REFIID riid, UINT*, void** ppv) override
        {
            *ppv = nullptr;
            if (cidl == 0) return E_INVALIDARG;
            if (riid == IID_IContextMenu || riid == IID_IContextMenu2 || riid == IID_IContextMenu3)
                return CreateItemMenu(place, root, this, hwnd, cidl, apidl, riid, ppv);
            if (riid == IID_IDataObject)
            {
                size_t here;
                auto a = Arc(here);
                if (!a) return E_FAIL;
                std::vector<size_t> nodes;
                for (UINT i = 0; i < cidl; i++)
                {
                    const Id* id = AsId(apidl[i]);
                    if (!id) return E_INVALIDARG;
                    size_t k = Child(a, here, id);
                    if (k != v31np::None) nodes.push_back(k);
                }
                ComPtr<IDataObject> inner;
                HRESULT r = CreateData(a, here, nodes, place, root, IID_PPV_ARGS(&inner));
                if (FAILED(r)) return r;
                return SHCreateDataObject(root, cidl, apidl, inner.Get(), riid, ppv);
            }
            const Id* id = AsId(apidl[0]);
            if (!id) return E_INVALIDARG;
            if (riid == IID_IExtractIconW || riid == IID_IExtractIconA)
                return SHCreateFileExtractIconW(IdNatural(id).c_str(), id->dir ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL, riid, ppv);
            if (riid == IID_IThumbnailProvider && !id->dir && !IsPack(IdNatural(id)))
                return Thumb(apidl[0], riid, ppv);
            if (riid == IID_IDropTarget && (id->dir || IsPack(IdNatural(id))) && Writable())
            {
                Place next = place;
                if (id->dir) next.sub.push_back(IdNatural(id));
                else
                {
                    size_t here;
                    auto a = Arc(here);
                    const size_t k = a ? Child(a, here, id) : v31np::None;
                    if (k == v31np::None || a->index->tree.nodes[k].dir) return E_NOINTERFACE;
                    next.chain.push_back(a->index->entries[a->index->tree.nodes[k].entry].key);
                    next.sub.clear();
                    if (!Load(next.path, next.chain)) return HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
                }
                PIDLIST_ABSOLUTE full = ILCombine(root, apidl[0]);
                HRESULT r = CreateDrop(next, full, riid, ppv);
                CoTaskMemFree(full);
                return r;
            }
            if (riid == IID_IQueryAssociations)
            {
                const std::wstring natural = IdNatural(id);
                const size_t dot = natural.rfind(L'.');
                const std::wstring ext = dot == std::wstring::npos ? L"" : natural.substr(dot);
                ASSOCIATIONELEMENT file[] = {
                    { ASSOCCLASS_PROGID_STR, nullptr, ext.c_str() },
                    { ASSOCCLASS_SYSTEM_STR, nullptr, ext.c_str() },
                    { ASSOCCLASS_STAR, nullptr, nullptr },
                };
                ASSOCIATIONELEMENT folder[] = {
                    { ASSOCCLASS_FOLDER, nullptr, nullptr },
                };
                if (id->dir || IsPack(natural)) return AssocCreateForClasses(folder, ARRAYSIZE(folder), riid, ppv);
                if (ext.empty()) return AssocCreateForClasses(file + 2, 1, riid, ppv);
                return AssocCreateForClasses(file, ARRAYSIZE(file), riid, ppv);
            }
            return E_NOINTERFACE;
        }

        IFACEMETHODIMP GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF flags, STRRET* name) override
        {
            const Id* id = AsId(pidl);
            if (!id) return E_INVALIDARG;
            if (flags & SHGDN_FORPARSING)
            {
                if (flags & SHGDN_INFOLDER) return Ret(IdNatural(id), name);
                PWSTR base = nullptr;
                HRESULT r = SHGetNameFromIDList(root, SIGDN_DESKTOPABSOLUTEPARSING, &base);
                if (FAILED(r)) return r;
                std::wstring full = base;
                CoTaskMemFree(base);
                return Ret(full + L"\\" + IdNatural(id), name);
            }
            return Ret(IdStored(id), name);
        }

        IFACEMETHODIMP SetNameOf(HWND, PCUITEMID_CHILD pidl, LPCWSTR newName, SHGDNF, PITEMID_CHILD* out) override
        {
            if (out) *out = nullptr;
            const Id* id = AsId(pidl);
            if (!id || !Writable()) return E_INVALIDARG;
            HRESULT r = RenameNode(place, IdNatural(id), newName);
            if (FAILED(r)) return r;
            Changed(root);
            if (out)
            {
                size_t here;
                auto a = Arc(here);
                if (a)
                    for (size_t k : a->index->tree.nodes[here].kids)
                        if (a->index->tree.nodes[k].stored == newName)
                        {
                            *out = MakeId(a->index->tree.nodes[k], SizeOf(a, k));
                            break;
                        }
            }
            return S_OK;
        }

        IFACEMETHODIMP GetDefaultSearchGUID(GUID*) override
        {
            return E_NOTIMPL;
        }

        IFACEMETHODIMP EnumSearches(IEnumExtraSearch**) override
        {
            return E_NOTIMPL;
        }

        IFACEMETHODIMP GetDefaultColumn(DWORD, ULONG* sort, ULONG* display) override
        {
            *sort = 0;
            *display = 0;
            return S_OK;
        }

        IFACEMETHODIMP GetDefaultColumnState(UINT column, SHCOLSTATEF* flags) override
        {
            if (column >= ARRAYSIZE(Columns)) return E_INVALIDARG;
            *flags = SHCOLSTATE_ONBYDEFAULT | (column == 2 ? SHCOLSTATE_TYPE_INT : SHCOLSTATE_TYPE_STR);
            return S_OK;
        }

        IFACEMETHODIMP GetDetailsEx(PCUITEMID_CHILD pidl, const SHCOLUMNID* key, VARIANT* v) override
        {
            const Id* id = AsId(pidl);
            if (!id) return E_INVALIDARG;
            if (IsEqualPropertyKey(*key, PKEY_ItemNameDisplay)) return InitVariantFromString(IdStored(id).c_str(), v);
            if (IsEqualPropertyKey(*key, PKEY_ItemTypeText)) return InitVariantFromString(TypeOf(IdNatural(id), id->dir != 0).c_str(), v);
            if (IsEqualPropertyKey(*key, PKEY_FileName) || IsEqualPropertyKey(*key, PKEY_ParsingName)) return InitVariantFromString(IdNatural(id).c_str(), v);
            if (IsEqualPropertyKey(*key, PKEY_Size) && !id->dir) return InitVariantFromUInt64(id->size, v);
            if (IsEqualPropertyKey(*key, PKEY_FileAttributes)) return InitVariantFromUInt32(id->dir ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL, v);
            if (IsEqualPropertyKey(*key, PKEY_ThumbnailCacheId) && !id->dir)
            {
                size_t here;
                auto a = Arc(here);
                if (!a) return E_FAIL;
                const size_t k = Child(a, here, id);
                if (k == v31np::None) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
                return InitVariantFromUInt64(CacheId(a, a->index->tree.nodes[k].entry), v);
            }
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        }

        IFACEMETHODIMP GetDetailsOf(PCUITEMID_CHILD pidl, UINT column, SHELLDETAILS* sd) override
        {
            if (column >= ARRAYSIZE(Columns)) return E_FAIL;
            sd->fmt = column == 2 ? LVCFMT_RIGHT : LVCFMT_LEFT;
            sd->cxChar = column == 0 ? 32 : 16;
            if (!pidl)
            {
                ComPtr<IPropertyDescription> desc;
                HRESULT r = PSGetPropertyDescription(Columns[column], IID_PPV_ARGS(&desc));
                if (FAILED(r)) return r;
                PWSTR label = nullptr;
                r = desc->GetDisplayName(&label);
                if (FAILED(r)) return r;
                sd->str.uType = STRRET_WSTR;
                sd->str.pOleStr = label;
                return S_OK;
            }
            VARIANT v;
            VariantInit(&v);
            HRESULT r = GetDetailsEx(pidl, &Columns[column], &v);
            if (FAILED(r)) return Ret(L"", &sd->str);
            PROPVARIANT pv;
            r = VariantToPropVariant(&v, &pv);
            VariantClear(&v);
            if (FAILED(r)) return r;
            PWSTR text = nullptr;
            r = PSFormatForDisplayAlloc(Columns[column], pv, PDFF_DEFAULT, &text);
            PropVariantClear(&pv);
            if (FAILED(r)) return r;
            sd->str.uType = STRRET_WSTR;
            sd->str.pOleStr = text;
            return S_OK;
        }

        IFACEMETHODIMP MapColumnToSCID(UINT column, SHCOLUMNID* key) override
        {
            if (column >= ARRAYSIZE(Columns)) return E_FAIL;
            *key = Columns[column];
            return S_OK;
        }
    };

    HRESULT FromIds(PCIDLIST_ABSOLUTE full, REFIID riid, void** ppv)
    {
        *ppv = nullptr;
        PCUIDLIST_RELATIVE rel = full;
        while (rel->mkid.cb && !AsId(rel)) rel = ILNext(rel);
        if (!rel->mkid.cb) return E_INVALIDARG;
        PIDLIST_ABSOLUTE top = ILCloneFull(full);
        if (!top) return E_OUTOFMEMORY;
        ((PUIDLIST_RELATIVE)((BYTE*)top + ((const BYTE*)rel - (const BYTE*)full)))->mkid.cb = 0;
        ComPtr<IShellFolder2> folder;
        ComPtr<IPersistFolder> persist;
        HRESULT r = CreateFolder(Place{}, nullptr, IID_PPV_ARGS(&folder));
        if (SUCCEEDED(r)) r = folder.As(&persist);
        if (SUCCEEDED(r)) r = persist->Initialize(top);
        CoTaskMemFree(top);
        if (FAILED(r)) return r;
        PCUITEMID_CHILD last = ILFindLastID(rel);
        if (last != rel)
        {
            PIDLIST_RELATIVE up = ILClone(rel);
            if (!up) return E_OUTOFMEMORY;
            ILRemoveLastID(up);
            ComPtr<IShellFolder2> sub;
            r = folder->BindToObject(up, nullptr, IID_PPV_ARGS(&sub));
            CoTaskMemFree(up);
            if (FAILED(r)) return r;
            folder = sub;
        }
        return static_cast<PackFolder*>(folder.Get())->Local(last, riid, ppv);
    }

    HRESULT Top(PCIDLIST_ABSOLUTE full, ComPtr<IShellFolder2>& folder, PCUIDLIST_RELATIVE& rel)
    {
        rel = full;
        while (rel->mkid.cb && !AsId(rel)) rel = ILNext(rel);
        if (!rel->mkid.cb) return E_INVALIDARG;
        PIDLIST_ABSOLUTE top = ILCloneFull(full);
        if (!top) return E_OUTOFMEMORY;
        ((PUIDLIST_RELATIVE)((BYTE*)top + ((const BYTE*)rel - (const BYTE*)full)))->mkid.cb = 0;
        ComPtr<IPersistFolder> persist;
        HRESULT r = CreateFolder(Place{}, nullptr, IID_PPV_ARGS(&folder));
        if (SUCCEEDED(r)) r = folder.As(&persist);
        if (SUCCEEDED(r)) r = persist->Initialize(top);
        CoTaskMemFree(top);
        return r;
    }

    class Worker final : public IInitializeWithFile, public IThumbnailProvider
    {
        REFS

    public:
        std::vector<BYTE> ids;

        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
        {
            static const QITAB qit[] = {
                QITABENT(Worker, IInitializeWithFile),
                QITABENT(Worker, IThumbnailProvider),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP Initialize(LPCWSTR path, DWORD) override
        {
            if (!ids.empty()) return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
            std::vector<BYTE> b;
            if (!path || !Unhex(path, b)) return E_INVALIDARG;
            ids = std::move(b);
            return S_OK;
        }

        IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP* bmp, WTS_ALPHATYPE* alpha) override
        {
            *bmp = nullptr;
            if (ids.empty()) return E_UNEXPECTED;
            ComPtr<IThumbnailProvider> t;
            HRESULT r = FromIds((PCIDLIST_ABSOLUTE)ids.data(), IID_PPV_ARGS(&t));
            return SUCCEEDED(r) ? t->GetThumbnail(cx, bmp, alpha) : r;
        }
    };
}

HRESULT Locate(PCIDLIST_ABSOLUTE full, Place& parent, std::wstring& natural)
{
    if (!full) return E_INVALIDARG;
    ComPtr<IShellFolder2> folder;
    PCUIDLIST_RELATIVE rel = nullptr;
    HRESULT r = Top(full, folder, rel);
    if (FAILED(r)) return r;
    PackFolder* f = static_cast<PackFolder*>(folder.Get());
    PCUITEMID_CHILD last = ILFindLastID(rel);
    const Id* id = AsId(last);
    if (!id) return E_INVALIDARG;
    natural = IdNatural(id);
    if (last == rel)
    {
        parent = f->place;
        return S_OK;
    }
    PIDLIST_RELATIVE up = ILClone(rel);
    if (!up) return E_OUTOFMEMORY;
    ILRemoveLastID(up);
    std::shared_ptr<Archive> a;
    size_t node;
    r = f->Descend(up, parent, a, node);
    CoTaskMemFree(up);
    if (FAILED(r)) return r;
    const v31np::Node& n = a->index->tree.nodes[node];
    if (!n.dir)
    {
        if (!IsPack(n.natural)) return E_INVALIDARG;
        parent.chain.push_back(a->index->entries[n.entry].key);
        parent.sub.clear();
    }
    return S_OK;
}

HRESULT CreateWorker(REFIID riid, void** ppv)
{
    *ppv = nullptr;
    Worker* w = new (std::nothrow) Worker();
    if (!w) return E_OUTOFMEMORY;
    HRESULT r = w->QueryInterface(riid, ppv);
    w->Release();
    return r;
}

HRESULT CreateFolder(const Place& place, PCIDLIST_ABSOLUTE root, REFIID riid, void** ppv)
{
    *ppv = nullptr;
    PackFolder* f = new (std::nothrow) PackFolder();
    if (!f) return E_OUTOFMEMORY;
    f->place = place;
    if (root)
    {
        f->root = ILCloneFull(root);
        if (!f->root)
        {
            f->Release();
            return E_OUTOFMEMORY;
        }
    }
    HRESULT r = f->QueryInterface(riid, ppv);
    f->Release();
    return r;
}
