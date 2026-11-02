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
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "../lib/officer.h"
#include "../lib/order.h"
#include "../lib/road.h"
#include "../lib/shadow.h"
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
extern const IID IID_Store;

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

struct Link
{
    char kind = 'A';
    uint64_t raw = 0;
    std::wstring file;
    bool pending = false;
};

struct Index
{
    std::vector<v31np::Entry> entries;
    std::vector<Link> links;
    std::vector<std::string> dirs;
    v31np::Tree tree;
    std::vector<std::unordered_map<std::wstring, size_t>> lookup;
    std::unique_ptr<std::atomic<bool>[]> blank;

    size_t Child(size_t dir, const std::wstring& natural) const;
};

struct Archive
{
    std::wstring path;
    HANDLE file = INVALID_HANDLE_VALUE;
    bool shared = false;
    std::shared_ptr<Archive> parent;
    size_t slot = 0;
    std::string key;
    uint64_t total = 0;
    uint64_t stamp = 0;
    uint64_t version = 0;
    std::shared_ptr<const v31np::Shadow> shadow;
    std::shared_ptr<const Index> index;
    std::mutex guard;
    std::map<std::string, std::weak_ptr<Archive>> nested;
    mutable std::mutex files;
    mutable std::map<std::wstring, HANDLE> handles;

    ~Archive();
    bool ReadAt(uint64_t pos, void* buf, uint32_t len) const;
    bool ReadEntry(size_t entry, uint64_t pos, void* buf, uint32_t len) const;
    size_t Find(const std::vector<std::wstring>& sub) const;
};

struct Place
{
    std::wstring path;
    std::vector<std::string> chain;
    std::vector<std::wstring> sub;
};

std::shared_ptr<Archive> Load(const std::wstring& path, const std::vector<std::string>& chain);
std::wstring ArchiveId(const std::wstring& path, const std::vector<std::string>& chain);

class Edit
{
public:
    std::wstring path;
    v31np::Shadow shadow;
    std::vector<std::wstring> made;
    std::vector<std::wstring> hide;

    std::vector<v31np::Row>* Part(const v31np::Chain& chain);
    void Put(const v31np::Chain& chain, v31np::Row row);
    void Drop(const v31np::Chain& chain, const std::string& prefix);
    void Move(const v31np::Chain& chain, const std::string& from, const std::string& to, bool keep, const std::string* base = nullptr, bool self = false);
    std::string Fresh(const v31np::Chain& chain, std::string key, const std::string& base, bool self, bool dir);
    std::wstring Temp();

private:
    struct Taken
    {
        std::set<std::string> files;
        std::set<std::string> dirs;
        std::map<std::string, std::string> moved;
    };
    std::map<v31np::Chain, Taken> taken;
};

HRESULT Mutate(const std::wstring& path, const std::function<HRESULT(Edit&)>& change, const char* hint = nullptr);
HRESULT Materialise(const std::shared_ptr<Archive>& a, size_t entry, std::wstring& file, uint64_t& size);
HRESULT CopyOut(const std::shared_ptr<Archive>& a, size_t entry, Edit& into, v31np::Row& row);
HRESULT Finalise(const std::wstring& path);
void Enter(const std::wstring& path);
void Leave(const std::wstring& path);
std::wstring TempFile();
v31np::Item Keep(const std::shared_ptr<Archive>& a, size_t entry, const std::string& key);
v31np::Item FromFile(const std::string& key, const std::wstring& file, uint64_t size);
std::vector<std::wstring> StoredSegments(const v31np::Tree& t, size_t node);
std::string Join(const std::vector<std::wstring>& segs);
uint64_t CacheId(const std::shared_ptr<Archive>& a, size_t entry);
HRESULT AddPaths(const Place& place, const std::vector<std::wstring>& paths, bool move);
HRESULT Transfer(const Place& from, const std::vector<std::wstring>& names, const Place& to, bool move);
HRESULT RemoveNodes(const Place& place, const std::vector<std::wstring>& names);
HRESULT RenameNode(const Place& place, const std::wstring& natural, const std::wstring& stored);
HRESULT PackNodes(const Place& place, const std::vector<std::wstring>& names);
HRESULT CreateFromPaths(const std::wstring& target, const std::vector<std::wstring>& paths);
std::vector<std::wstring> DropPaths(IDataObject* data);
bool PasteItems(IDataObject* clip, const Place& place, PCIDLIST_ABSOLUTE dest, HWND hwnd);
HRESULT Locate(PCIDLIST_ABSOLUTE full, Place& parent, std::wstring& natural);
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
HRESULT CreateData(const std::shared_ptr<Archive>& a, size_t base, const std::vector<size_t>& nodes, const Place& place, PCIDLIST_ABSOLUTE root, REFIID riid, void** ppv);
HRESULT CreateEntryStream(const std::shared_ptr<Archive>& a, size_t node, IStream** out);
HRESULT CreateFileStream(const std::wstring& file, bool temporary, const std::wstring& name, IStream** out);
HRESULT CreateStore(const Place& place, PCIDLIST_ABSOLUTE pidl, IStorage** out);
HRESULT CreateWriteStream(const Place& place, PCIDLIST_ABSOLUTE pidl, const std::wstring& natural, bool fresh, IStream** out);
HRESULT CreateWorker(REFIID riid, void** ppv);
HRESULT CreateSource(const Place& place, PCIDLIST_ABSOLUTE pidl, REFIID riid, void** ppv);
HRESULT CreateViewCallback(const std::wstring& path, PCIDLIST_ABSOLUTE root, IShellFolderViewCB** out);
HRESULT CreateDrop(const Place& place, PCIDLIST_ABSOLUTE pidl, REFIID riid, void** ppv);
HRESULT CreateItemMenu(const Place& place, PCIDLIST_ABSOLUTE root, IShellFolder* folder, HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, REFIID riid, void** ppv);
HRESULT WrapBackMenu(const Place& place, PCIDLIST_ABSOLUTE root, IContextMenu* inner, REFIID riid, void** ppv);
HRESULT CreateBackMenu(const Place& place, PCIDLIST_ABSOLUTE root, IShellFolder* folder, HWND hwnd, REFIID riid, void** ppv);
HRESULT CreateFileMenu(REFIID riid, void** ppv);
HRESULT CreateSend(REFIID riid, void** ppv);
HRESULT CreatePackMenu(REFIID riid, void** ppv);
void PackAsync(const std::vector<std::wstring>& paths);
std::wstring IconPath();
HRESULT ExtractTo(const std::wstring& file, const std::wstring& dest, HWND hwnd);
void Changed(PCIDLIST_ABSOLUTE pidl);
