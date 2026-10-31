#pragma once

#define NOMINMAX
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propsys.h>
#include <propvarutil.h>
#include <wrl/client.h>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../lib/order.h"
#include "../lib/tree.h"
#include "../lib/v31np.h"

using Microsoft::WRL::ComPtr;

extern HINSTANCE Module;
extern std::atomic<long> Locks;
extern const CLSID ClsidFolder;
extern const CLSID ClsidMenu;
extern const CLSID ClsidSend;
extern const CLSID ClsidPack;
extern const CLSID ClsidThumb;
extern const wchar_t TypeName[];

struct Lock
{
    Lock() { ++Locks; }
    ~Lock() { --Locks; }
};

#define REFS \
private: \
    std::atomic<ULONG> refs{ 1 }; \
    Lock lock; \
public: \
    IFACEMETHODIMP_(ULONG) AddRef() override { return ++refs; } \
    IFACEMETHODIMP_(ULONG) Release() override { ULONG r = --refs; if (!r) delete this; return r; }

struct Index
{
    std::vector<v31np::Entry> entries;
    v31np::Tree tree;
    std::vector<std::unordered_map<std::wstring, size_t>> lookup;
    std::unique_ptr<std::atomic<bool>[]> blank;

    size_t Child(size_t dir, const std::wstring& natural) const;
};

struct Archive
{
    std::wstring path;
    HANDLE file = INVALID_HANDLE_VALUE;
    std::shared_ptr<Archive> parent;
    std::string key;
    uint64_t base = 0;
    uint64_t total = 0;
    uint64_t stamp = 0;
    std::shared_ptr<const Index> index;
    std::mutex guard;
    std::map<std::string, std::weak_ptr<Archive>> nested;

    ~Archive();
    bool ReadAt(uint64_t pos, void* buf, uint32_t len) const;
    size_t Find(const std::vector<std::wstring>& sub) const;
};

struct Place
{
    std::wstring path;
    std::vector<std::string> chain;
    std::vector<std::wstring> sub;
};

std::shared_ptr<Archive> Load(const std::wstring& path, const std::vector<std::string>& chain);
HRESULT Commit(const std::shared_ptr<Archive>& a, const std::vector<v31np::Item>& items);
v31np::Item Keep(const std::shared_ptr<Archive>& a, size_t entry, const std::string& key);
std::vector<std::wstring> StoredSegments(const v31np::Tree& t, size_t node);
std::string Join(const std::vector<std::wstring>& segs);
uint64_t CacheId(const std::shared_ptr<Archive>& a, size_t entry);
HRESULT AddPaths(const Place& place, const std::vector<std::wstring>& paths);
HRESULT RemoveNodes(const Place& place, const std::vector<std::wstring>& names);
HRESULT RenameNode(const Place& place, const std::wstring& natural, const std::wstring& stored);
HRESULT CreateFromPaths(const std::wstring& target, const std::vector<std::wstring>& paths);
std::vector<std::wstring> DropPaths(IDataObject* data);
bool IsPack(const std::wstring& name);

#pragma pack(push, 1)
struct Id
{
    USHORT cb;
    DWORD magic;
    BYTE dir;
    ULONGLONG size;
    USHORT stored;
    USHORT natural;
};
#pragma pack(pop)

PITEMID_CHILD MakeId(const v31np::Node& n, uint64_t size);
const Id* AsId(PCUIDLIST_RELATIVE pidl);
std::wstring IdStored(const Id* id);
std::wstring IdNatural(const Id* id);

HRESULT CreateFolder(const Place& place, PCIDLIST_ABSOLUTE root, REFIID riid, void** ppv);
HRESULT CreateData(const std::shared_ptr<Archive>& a, size_t base, const std::vector<size_t>& nodes, size_t prefix, REFIID riid, void** ppv);
HRESULT CreateEntryStream(const std::shared_ptr<Archive>& a, size_t node, IStream** out);
HRESULT CreateWorker(REFIID riid, void** ppv);
HRESULT CreateDrop(const Place& place, PCIDLIST_ABSOLUTE pidl, REFIID riid, void** ppv);
HRESULT CreateItemMenu(const Place& place, PCIDLIST_ABSOLUTE root, IShellFolder* folder, HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, REFIID riid, void** ppv);
HRESULT CreateBackMenu(const Place& place, PCIDLIST_ABSOLUTE root, IShellFolder* folder, HWND hwnd, REFIID riid, void** ppv);
HRESULT CreateFileMenu(REFIID riid, void** ppv);
HRESULT CreateSend(REFIID riid, void** ppv);
HRESULT CreatePackMenu(REFIID riid, void** ppv);
void PackAsync(const std::vector<std::wstring>& paths);
std::wstring IconPath();
HRESULT ExtractTo(const std::wstring& file, const std::wstring& dest, HWND hwnd);
void Changed(PCIDLIST_ABSOLUTE pidl);
