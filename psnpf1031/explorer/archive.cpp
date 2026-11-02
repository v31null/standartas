#include "shell.h"

#include <algorithm>
#include <thread>
#include <unordered_set>

namespace
{
    std::mutex cacheGuard;
    std::map<std::wstring, std::weak_ptr<Archive>> cache;
    struct Known
    {
        uint64_t version = 0;
        std::shared_ptr<const Index> index;
    };
    std::map<std::wstring, Known> known;
    struct Journal
    {
        uint64_t size = 0;
        uint64_t time = 0;
        std::shared_ptr<const v31np::Shadow> shadow;
    };
    std::map<std::wstring, Journal> journals;
    std::map<std::wstring, int> views;
    constexpr DWORD Magic = 0x706E3133;

    uint64_t Join64(DWORD hi, DWORD lo)
    {
        return ((uint64_t)hi << 32) | lo;
    }

    uint64_t Mix(uint64_t h, const void* p, size_t n)
    {
        for (size_t i = 0; i < n; i++)
        {
            h ^= ((const BYTE*)p)[i];
            h *= 1099511628211ull;
        }
        return h;
    }

    uint64_t Mix(uint64_t h, uint64_t v)
    {
        return Mix(h, &v, sizeof(v));
    }

    std::wstring NameOf(const std::wstring& path)
    {
        std::wstring p = path;
        while (!p.empty() && (p.back() == L'\\' || p.back() == L'/')) p.pop_back();
        size_t cut = p.find_last_of(L"\\/");
        return cut == std::wstring::npos ? p : p.substr(cut + 1);
    }

    std::wstring JournalPath(const std::wstring& path)
    {
        const size_t cut = path.find_last_of(L"\\/");
        const std::wstring dir = cut == std::wstring::npos ? L"" : path.substr(0, cut + 1);
        return dir + v31np::JournalName(NameOf(path));
    }

    std::wstring PostPath(const std::wstring& path)
    {
        const size_t cut = path.find_last_of(L"\\/");
        const std::wstring dir = cut == std::wstring::npos ? L"" : path.substr(0, cut + 1);
        return dir + v31np::SignalName(NameOf(path));
    }

    bool Attributes(const std::wstring& file, uint64_t& size, uint64_t& time)
    {
        WIN32_FILE_ATTRIBUTE_DATA d;
        if (!GetFileAttributesExW(file.c_str(), GetFileExInfoStandard, &d) || (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) return false;
        size = Join64(d.nFileSizeHigh, d.nFileSizeLow);
        time = Join64(d.ftLastWriteTime.dwHighDateTime, d.ftLastWriteTime.dwLowDateTime);
        return true;
    }

    const std::string& Papers()
    {
        static const std::string papers = "plate=" + v31np::Plate() + " os=" + v31np::System();
        return papers;
    }

    class Checkpoint
    {
        HANDLE h = nullptr;
        std::unique_ptr<v31np::Signal> signal;

    public:
        bool green = true;
        bool shared = false;

        explicit Checkpoint(const std::wstring& path, bool signalled = true)
        {
            const std::wstring f = v31np::Fold(path);
            wchar_t name[64];
            swprintf_s(name, L"Local\\v31np-%016llx", (unsigned long long)Mix(14695981039346656037ull, f.data(), f.size() * sizeof(wchar_t)));
            h = CreateMutexW(nullptr, FALSE, name);
            if (h) WaitForSingleObject(h, INFINITE);
            shared = v31np::SharedRoad(path);
            if (!shared || !signalled) return;
            signal = std::make_unique<v31np::Signal>(PostPath(path));
            green = signal->Green(30000) && signal->ShowPapers(Papers());
        }

        ~Checkpoint()
        {
            signal.reset();
            if (!h) return;
            ReleaseMutex(h);
            CloseHandle(h);
        }
    };

    bool ReadText(const std::wstring& file, std::string& out, uint64_t& size, uint64_t& time)
    {
        HANDLE h = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) return false;
        LARGE_INTEGER li{};
        FILETIME ft{};
        bool ok = GetFileSizeEx(h, &li) && GetFileTime(h, nullptr, nullptr, &ft) && li.QuadPart < (1ll << 31);
        if (ok)
        {
            size = (uint64_t)li.QuadPart;
            time = Join64(ft.dwHighDateTime, ft.dwLowDateTime);
            out.resize((size_t)size);
            DWORD got = 0;
            ok = size == 0 || (ReadFile(h, out.data(), (DWORD)size, &got, nullptr) && got == size);
        }
        CloseHandle(h);
        return ok;
    }

    bool WriteText(const std::wstring& file, const std::string& text)
    {
        const std::wstring tmp = file + L".tmp";
        HANDLE h = CreateFileW(tmp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, nullptr);
        if (h == INVALID_HANDLE_VALUE) return false;
        DWORD put = 0;
        bool ok = WriteFile(h, text.data(), (DWORD)text.size(), &put, nullptr) && put == text.size();
        ok = FlushFileBuffers(h) && ok;
        CloseHandle(h);
        if (ok) ok = MoveFileExW(tmp.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;
        if (!ok)
        {
            DeleteFileW(tmp.c_str());
            return false;
        }
        SetFileAttributesW(file.c_str(), FILE_ATTRIBUTE_HIDDEN);
        return true;
    }

    void Forget(const std::wstring& path)
    {
        const std::wstring root = v31np::Fold(path);
        std::lock_guard<std::mutex> g(cacheGuard);
        for (auto it = known.begin(); it != known.end();)
            it = it->first.compare(0, root.size(), root) == 0 && (it->first.size() == root.size() || it->first[root.size()] == L'\0') ? known.erase(it) : std::next(it);
        cache.erase(root);
        journals.erase(root);
    }

    std::wstring DotPath(const std::wstring& path)
    {
        const size_t cut = path.find_last_of(L"\\/");
        const std::wstring dir = cut == std::wstring::npos ? L"" : path.substr(0, cut + 1);
        return dir + L"." + v31np::JournalName(NameOf(path));
    }

    bool Exists(const std::wstring& file)
    {
        return GetFileAttributesW(file.c_str()) != INVALID_FILE_ATTRIBUTES;
    }

    bool HasJournal(const std::wstring& path)
    {
        return Exists(JournalPath(path)) || Exists(DotPath(path));
    }

    std::wstring Chosen(const std::wstring& path)
    {
        const std::wstring plain = JournalPath(path);
        const std::wstring dot = DotPath(path);
        uint64_t ps = 0;
        uint64_t pt = 0;
        uint64_t ds = 0;
        uint64_t dt = 0;
        const bool hp = Attributes(plain, ps, pt);
        const bool hd = Attributes(dot, ds, dt);
        if (!hp || !hd) return hp ? plain : hd ? dot : std::wstring();
        std::string a;
        std::string b;
        bool ra = ReadText(plain, a, ps, pt);
        bool rb = ReadText(dot, b, ds, dt);
        if (ra && rb && a == b) return plain;
        v31np::Shadow s;
        ra = ra && v31np::ReadJournal(a, s);
        rb = rb && v31np::ReadJournal(b, s);
        if (ra != rb) return ra ? plain : dot;
        return dt > pt ? dot : plain;
    }

    bool Keyless(const v31np::Row& r)
    {
        return r.kind == 'R' || r.kind == 'H';
    }

    bool Pathed(const v31np::Shadow& s)
    {
        for (const auto& part : s.parts)
            for (const auto& r : part.second)
                if (r.kind != 'A' && r.kind != 'D') return true;
        return false;
    }

    bool Interpretable(const std::wstring& file)
    {
        if (file.size() >= 3 && file[0] == L'\\' && file[1] == L'\\') return true;
        const wchar_t d = file.empty() ? 0 : (wchar_t)towupper(file[0]);
        return file.size() >= 3 && d >= L'A' && d <= L'Z' && file[1] == L':' && (file[2] == L'\\' || file[2] == L'/');
    }

    const std::string& Mine()
    {
        static const std::string plate = v31np::Plate();
        return plate;
    }

    bool Alien(const v31np::Row& r)
    {
        return !v31np::Ours(r, Mine()) || (r.kind != 'A' && r.kind != 'D' && !Interpretable(r.file));
    }

    bool Foreign(const v31np::Shadow& s)
    {
        for (const auto& part : s.parts)
            for (const auto& r : part.second)
                if (Alien(r)) return true;
        return false;
    }

    void Sign(v31np::Shadow& s)
    {
        s.version = 2;
        for (auto& part : s.parts)
            for (auto& r : part.second)
                if (r.kind != 'A' && r.kind != 'D') r.plate = Mine();
    }

    bool OwnTemp(const std::wstring& file)
    {
        wchar_t temp[MAX_PATH + 1];
        if (!GetTempPathW(ARRAYSIZE(temp), temp)) return false;
        const std::wstring dir = std::wstring(temp) + L"v31np\\";
        if (file.size() != dir.size() + 42 || v31np::Fold(file.substr(0, dir.size())) != v31np::Fold(dir)) return false;
        const std::wstring name = file.substr(dir.size());
        if (name[0] != L'{' || name[37] != L'}' || name.substr(38) != L".tmp") return false;
        for (size_t i = 1; i < 37; i++)
        {
            const wchar_t c = name[i];
            const bool dash = i == 9 || i == 14 || i == 19 || i == 24;
            if (dash ? c != L'-' : !iswxdigit(c)) return false;
        }
        return true;
    }

    void Seen(const std::wstring& file)
    {
        SHChangeNotify(SHCNE_ATTRIBUTES, SHCNF_PATHW | SHCNF_FLUSHNOWAIT, file.c_str(), nullptr);
        const size_t cut = file.find_last_of(L"\\/");
        if (cut != std::wstring::npos) SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATHW | SHCNF_FLUSHNOWAIT, file.substr(0, cut).c_str(), nullptr);
    }

    constexpr DWORD Settable = FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;

    bool Visible(const std::wstring& file)
    {
        const DWORD a = GetFileAttributesW(file.c_str());
        return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_HIDDEN);
    }

    void Hide(const std::wstring& file)
    {
        const DWORD a = GetFileAttributesW(file.c_str());
        if (a == INVALID_FILE_ATTRIBUTES || (a & FILE_ATTRIBUTE_HIDDEN)) return;
        if (SetFileAttributesW(file.c_str(), (a & Settable) | FILE_ATTRIBUTE_HIDDEN)) Seen(file);
    }

    void Unhide(const std::wstring& file)
    {
        const DWORD a = GetFileAttributesW(file.c_str());
        if (a == INVALID_FILE_ATTRIBUTES || !(a & FILE_ATTRIBUTE_HIDDEN)) return;
        if (SetFileAttributesW(file.c_str(), (a & Settable) & ~FILE_ATTRIBUTE_HIDDEN)) Seen(file);
    }

    std::vector<std::wstring> Orphans(v31np::Shadow& s)
    {
        std::set<std::wstring> held;
        for (const auto& part : s.parts)
            for (const auto& r : part.second)
                if (r.kind == 'M' || r.kind == 'R') held.insert(v31np::Fold(r.file));
        std::vector<std::wstring> out;
        for (auto& part : s.parts)
        {
            auto& rows = part.second;
            for (const auto& r : rows)
                if (r.kind == 'H' && !held.count(v31np::Fold(r.file))) out.push_back(r.file);
            rows.erase(std::remove_if(rows.begin(), rows.end(), [&](const v31np::Row& r) { return r.kind == 'H' && !held.count(v31np::Fold(r.file)); }), rows.end());
        }
        return out;
    }

    void Discard(const std::wstring& path, const std::wstring& journal, const v31np::Shadow& s)
    {
        const bool ours = !Foreign(s) && !v31np::SharedRoad(path);
        for (const auto& part : s.parts)
            for (const auto& r : part.second)
                if (ours && r.kind == 'T' && OwnTemp(r.file)) DeleteFileW(r.file.c_str());
        DeleteFileW(journal.c_str());
        for (const auto& part : s.parts)
            for (const auto& r : part.second)
                if (ours && r.kind == 'H') Unhide(r.file);
        Forget(path);
    }

    enum class State
    {
        None,
        Fresh,
        Stale,
        Broken,
    };

    State Current(const std::wstring& path, uint64_t size, uint64_t stamp, v31np::Shadow& s, uint64_t& jsize, uint64_t& jtime, std::wstring& journal)
    {
        journal = Chosen(path);
        if (journal.empty()) return State::None;
        std::string text;
        if (!ReadText(journal, text, jsize, jtime)) return Exists(journal) ? State::Broken : State::None;
        if (!v31np::ReadJournal(text, s)) return State::Broken;
        if (Pathed(s) && v31np::SharedRoad(path)) return State::Broken;
        return s.size == size && s.stamp == stamp ? State::Fresh : State::Stale;
    }

    void Loosen(Archive& a)
    {
        a.shared = v31np::SharedRoad(a.path);
        if (!a.shared) return;
        CloseHandle(a.file);
        a.file = INVALID_HANDLE_VALUE;
    }

    std::shared_ptr<const v31np::Shadow> ShadowOf(const std::wstring& path, uint64_t size, uint64_t stamp, uint64_t& version)
    {
        version = 0;
        uint64_t jsize = 0;
        uint64_t jtime = 0;
        std::wstring journal = Chosen(path);
        if (journal.empty() || !Attributes(journal, jsize, jtime)) return nullptr;
        const std::wstring key = v31np::Fold(path);
        {
            std::lock_guard<std::mutex> g(cacheGuard);
            auto it = journals.find(key);
            if (it != journals.end() && it->second.size == jsize && it->second.time == jtime && it->second.shadow && it->second.shadow->size == size && it->second.shadow->stamp == stamp)
            {
                version = Mix(Mix(14695981039346656037ull, jsize), jtime);
                return it->second.shadow;
            }
        }
        auto s = std::make_shared<v31np::Shadow>();
        State st = Current(path, size, stamp, *s, jsize, jtime, journal);
        // Delete not — someone rewrote the archive behind our back, so the journal's offsets now point at nonsense.
        // The journal walks the plank, the archive on disk wins.
        if (st == State::Stale && v31np::SharedRoad(path)) return nullptr;
        if (st == State::Stale)
        {
            Checkpoint stop(path);
            if (!stop.green) return nullptr;
            st = Current(path, size, stamp, *s, jsize, jtime, journal);
            if (st == State::Stale) Discard(path, journal, *s);
        }
        if (st != State::Fresh) return nullptr;
        version = Mix(Mix(14695981039346656037ull, jsize), jtime);
        std::lock_guard<std::mutex> g(cacheGuard);
        journals[key] = { jsize, jtime, s };
        return s;
    }

    std::shared_ptr<const v31np::Shadow> JournalOf(const std::wstring& path)
    {
        uint64_t size = 0;
        uint64_t time = 0;
        if (!Attributes(path, size, time)) return nullptr;
        auto s = std::make_shared<v31np::Shadow>();
        uint64_t jsize = 0;
        uint64_t jtime = 0;
        std::wstring journal;
        if (Current(path, size, time, *s, jsize, jtime, journal) != State::Fresh) return nullptr;
        return s;
    }

    std::shared_ptr<const Index> Indexed(const std::wstring& id, uint64_t version, const Archive* raw, const v31np::Shadow* shadow, const v31np::Chain& chain)
    {
        {
            std::lock_guard<std::mutex> g(cacheGuard);
            auto it = known.find(id);
            if (it != known.end() && it->second.version == version) return it->second.index;
        }
        auto x = std::make_shared<Index>();
        auto part = shadow ? shadow->parts.find(chain) : decltype(shadow->parts.end()){};
        if (shadow && part != shadow->parts.end())
        {
            for (const auto& r : part->second)
            {
                if (r.kind == 'D')
                {
                    x->dirs.push_back(r.key);
                    continue;
                }
                if (!v31np::Stored(r)) continue;
                v31np::Chain deeper = chain;
                deeper.push_back(r.key);
                x->entries.push_back({ r.key, r.kind == 'A' ? r.at : 0, v31np::Logical(*shadow, chain, r) });
                x->links.push_back({ r.kind, r.size, Alien(r) ? std::wstring() : r.file, shadow->parts.count(deeper) > 0 });
            }
        }
        else
        {
            if (!v31np::Parse([raw](uint64_t pos, void* buf, uint32_t len) { return raw->ReadAt(pos, buf, len); }, raw->total, x->entries)) return nullptr;
            x->links.resize(x->entries.size());
            for (size_t i = 0; i < x->entries.size(); i++)
            {
                v31np::Entry& e = x->entries[i];
                x->links[i].raw = e.size;
                v31np::Chain deeper = chain;
                deeper.push_back(e.key);
                if (!shadow || !shadow->parts.count(deeper)) continue;
                x->links[i].pending = true;
                v31np::Row r;
                r.key = e.key;
                r.size = e.size;
                r.at = e.at;
                e.size = v31np::Logical(*shadow, chain, r);
            }
        }
        x->tree.Build(x->entries, x->dirs);
        x->lookup.resize(x->tree.nodes.size());
        for (size_t d = 0; d < x->tree.nodes.size(); d++)
            for (size_t k : x->tree.nodes[d].kids) x->lookup[d].emplace(v31np::Fold(x->tree.nodes[k].natural), k);
        x->blank = std::make_unique<std::atomic<bool>[]>(x->entries.size());
        std::lock_guard<std::mutex> g(cacheGuard);
        known[id] = { version, x };
        return x;
    }

    std::shared_ptr<Archive> OpenFile(const std::wstring& path)
    {
        uint64_t size = 0;
        uint64_t time = 0;
        if (!Attributes(path, size, time)) return nullptr;
        uint64_t jver = 0;
        auto shadow = ShadowOf(path, size, time, jver);
        const uint64_t version = Mix(Mix(Mix(14695981039346656037ull, size), time), jver);
        const std::wstring key = v31np::Fold(path);
        {
            std::lock_guard<std::mutex> g(cacheGuard);
            auto it = cache.find(key);
            if (it != cache.end())
                if (auto a = it->second.lock())
                    if (a->version == version) return a;
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
        if (a->total != size || a->stamp != time) return nullptr;
        Loosen(*a);
        a->version = version;
        a->shadow = shadow;
        a->index = Indexed(key, version, a.get(), shadow.get(), {});
        if (!a->index) return nullptr;
        std::lock_guard<std::mutex> g(cacheGuard);
        cache[key] = a;
        return a;
    }

    std::shared_ptr<Archive> OpenWith(const std::wstring& path, const std::shared_ptr<const v31np::Shadow>& shadow)
    {
        static std::atomic<uint64_t> round{ 0 };
        auto a = std::make_shared<Archive>();
        a->path = path;
        a->file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (a->file == INVALID_HANDLE_VALUE) return nullptr;
        LARGE_INTEGER li;
        FILETIME ft;
        if (!GetFileSizeEx(a->file, &li) || !GetFileTime(a->file, nullptr, nullptr, &ft)) return nullptr;
        a->total = (uint64_t)li.QuadPart;
        a->stamp = Join64(ft.dwHighDateTime, ft.dwLowDateTime);
        Loosen(*a);
        a->version = Mix(Mix(Mix(14695981039346656037ull, a->total), a->stamp), ++round | (1ull << 63));
        a->shadow = shadow;
        a->index = Indexed(v31np::Fold(path), a->version, a.get(), shadow.get(), {});
        return a->index ? a : nullptr;
    }

    std::shared_ptr<Archive> Descend(std::shared_ptr<Archive> a, const std::wstring& path, const v31np::Chain& chain)
    {
        const auto root = a;
        std::wstring id = v31np::Fold(path);
        v31np::Chain sofar;
        for (const auto& key : chain)
        {
            if (!a) return nullptr;
            id += L'\0';
            id += v31np::Wide(key);
            sofar.push_back(key);
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
            const size_t slot = (size_t)(e - a->index->entries.begin());
            auto n = std::make_shared<Archive>();
            n->path = path;
            n->parent = a;
            n->slot = slot;
            n->key = key;
            n->total = a->index->links[slot].raw;
            n->version = root->version;
            n->shadow = root->shadow;
            n->index = Indexed(id, root->version, n.get(), root->shadow.get(), sofar);
            if (!n->index) return nullptr;
            a->nested[key] = n;
            a = n;
        }
        return a;
    }

    std::shared_ptr<Archive> RootOf(std::shared_ptr<Archive> a)
    {
        while (a && a->parent) a = a->parent;
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

    v31np::Chain ChainOf(const Archive* a)
    {
        v31np::Chain c;
        for (; a && a->parent; a = a->parent.get()) c.push_back(a->key);
        std::reverse(c.begin(), c.end());
        return c;
    }

    bool CopyRange(const std::shared_ptr<Archive>& a, size_t entry, const std::wstring& to)
    {
        HANDLE h = CreateFileW(to.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
        if (h == INVALID_HANDLE_VALUE) return false;
        const uint64_t size = a->index->links[entry].raw;
        std::vector<BYTE> buf(1 << 20);
        bool ok = true;
        for (uint64_t pos = 0; ok && pos < size;)
        {
            DWORD len = (DWORD)std::min<uint64_t>(buf.size(), size - pos);
            DWORD put = 0;
            ok = a->ReadEntry(entry, pos, buf.data(), len) && WriteFile(h, buf.data(), len, &put, nullptr) && put == len;
            pos += len;
        }
        CloseHandle(h);
        return ok;
    }

    HRESULT WriteItems(const std::wstring& to, const std::vector<v31np::Item>& items, uint64_t& size)
    {
        HANDLE h = CreateFileW(to.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
        if (h == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());
        size = 0;
        const bool ok = v31np::Write(items, [h, &size](const void* b, uint32_t n)
        {
            DWORD put = 0;
            size += n;
            return WriteFile(h, b, n, &put, nullptr) && put == n;
        });
        CloseHandle(h);
        return ok ? S_OK : E_FAIL;
    }

    HRESULT Flatten(const std::shared_ptr<Archive>& n, std::wstring& file, uint64_t& size);

    HRESULT Items(const std::shared_ptr<Archive>& a, std::vector<v31np::Item>& items, std::vector<std::wstring>& temps)
    {
        for (size_t i = 0; i < a->index->entries.size(); i++)
        {
            const std::string& key = a->index->entries[i].key;
            const Link& l = a->index->links[i];
            if (l.pending)
            {
                v31np::Chain chain = ChainOf(a.get());
                chain.push_back(key);
                auto n = Descend(RootOf(a), a->path, chain);
                if (!n) return HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
                std::wstring f;
                uint64_t s = 0;
                HRESULT r = Flatten(n, f, s);
                if (FAILED(r)) return r;
                temps.push_back(f);
                items.push_back(FromFile(key, f, s));
            }
            else if (l.kind == 'A') items.push_back(Keep(a, i, key));
            else if (l.file.empty()) return E_ACCESSDENIED;
            else
            {
                uint64_t s = 0;
                uint64_t t = 0;
                if (!Attributes(l.file, s, t)) continue;
                items.push_back(FromFile(key, l.file, s));
            }
        }
        return S_OK;
    }

    HRESULT Flatten(const std::shared_ptr<Archive>& n, std::wstring& file, uint64_t& size)
    {
        std::vector<v31np::Item> items;
        std::vector<std::wstring> temps;
        HRESULT r = Items(n, items, temps);
        file = TempFile();
        if (SUCCEEDED(r)) r = file.empty() ? E_FAIL : WriteItems(file, items, size);
        items.clear();
        for (const auto& t : temps) DeleteFileW(t.c_str());
        if (FAILED(r) && !file.empty()) DeleteFileW(file.c_str());
        return r;
    }

    bool ReadHandle(HANDLE h, uint64_t pos, void* buf, uint32_t len)
    {
        BYTE* p = (BYTE*)buf;
        while (len > 0)
        {
            OVERLAPPED o{};
            o.Offset = (DWORD)pos;
            o.OffsetHigh = (DWORD)(pos >> 32);
            DWORD got = 0;
            if (!ReadFile(h, p, len, &got, &o) || got == 0) return false;
            p += got;
            pos += got;
            len -= got;
        }
        return true;
    }

    struct Beside
    {
        HANDLE h = INVALID_HANDLE_VALUE;
        std::wstring tmp;

        Beside() = default;
        Beside(const Beside&) = delete;
        Beside& operator=(const Beside&) = delete;

        ~Beside()
        {
            Drop();
        }

        void Drop()
        {
            if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
            h = INVALID_HANDLE_VALUE;
            if (!tmp.empty()) DeleteFileW(tmp.c_str());
            tmp.clear();
        }
    };

    HRESULT WriteBeside(const std::wstring& path, const std::vector<v31np::Item>& items, bool verify, Beside& out, const std::function<bool()>& renew = nullptr)
    {
        static std::atomic<uint32_t> count{ 0 };
        wchar_t tag[48];
        swprintf_s(tag, L".%08lx%016llx%08lx.tmp", GetCurrentProcessId(), (unsigned long long)GetTickCount64(), (unsigned long)++count);
        out.tmp = path + tag;
        out.h = CreateFileW(out.tmp.c_str(), GENERIC_READ | GENERIC_WRITE | DELETE, FILE_SHARE_DELETE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (out.h == INVALID_HANDLE_VALUE)
        {
            const HRESULT r = HRESULT_FROM_WIN32(GetLastError());
            out.tmp.clear();
            return r;
        }
        const HANDLE h = out.h;
        uint64_t total = 0;
        bool ok = v31np::Write(items, [h, &total, &renew](const void* b, uint32_t n)
        {
            if (renew && !renew()) return false;
            DWORD put = 0;
            total += n;
            return WriteFile(h, b, n, &put, nullptr) && put == n;
        });
        ok = FlushFileBuffers(h) && ok;
        if (ok && verify)
        {
            const v31np::ReadAt read = [h](uint64_t pos, void* buf, uint32_t len) { return ReadHandle(h, pos, buf, len); };
            std::vector<v31np::Entry> check;
            ok = v31np::Parse(read, total, check) && v31np::Verify(read, total);
        }
        if (ok) return S_OK;
        out.Drop();
        return E_FAIL;
    }

    HRESULT Replace(Beside& b, const std::wstring& path, bool shared, HANDLE held = INVALID_HANDLE_VALUE, HANDLE* close = nullptr)
    {
        const size_t bytes = sizeof(FILE_RENAME_INFO) + path.size() * sizeof(wchar_t);
        std::vector<BYTE> buf(bytes, 0);
        FILE_RENAME_INFO* info = (FILE_RENAME_INFO*)buf.data();
        info->Flags = FILE_RENAME_FLAG_REPLACE_IF_EXISTS | FILE_RENAME_FLAG_POSIX_SEMANTICS;
        info->FileNameLength = (DWORD)(path.size() * sizeof(wchar_t));
        memcpy(info->FileName, path.c_str(), info->FileNameLength);
        bool moved = false;
        FILE_INFO_BY_HANDLE_CLASS kind = FileRenameInfoEx;
        for (int tries = 0;; tries++)
        {
            moved = SetFileInformationByHandle(b.h, kind, info, (DWORD)bytes) != FALSE;
            if (!moved && kind == FileRenameInfoEx && GetLastError() == ERROR_INVALID_PARAMETER)
            {
                kind = FileRenameInfo;
                info->Flags = 0;
                info->ReplaceIfExists = TRUE;
                moved = SetFileInformationByHandle(b.h, kind, info, (DWORD)bytes) != FALSE;
            }
            if (moved || !shared || tries >= 40) break;
            Sleep(250);
        }
        CloseHandle(b.h);
        b.h = INVALID_HANDLE_VALUE;
        if (moved)
        {
            b.tmp.clear();
            return S_OK;
        }
        if (held != INVALID_HANDLE_VALUE && close)
        {
            CloseHandle(held);
            *close = INVALID_HANDLE_VALUE;
        }
        if (!MoveFileExW(b.tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        {
            const HRESULT r = HRESULT_FROM_WIN32(GetLastError());
            b.Drop();
            return r;
        }
        b.tmp.clear();
        return S_OK;
    }

    HRESULT Commit(const std::shared_ptr<Archive>& a, const std::vector<v31np::Item>& items, bool shared)
    {
        if (!a || a->parent) return E_ACCESSDENIED;
        Beside b;
        HRESULT r = WriteBeside(a->path, items, shared, b);
        if (FAILED(r)) return r;
        return Replace(b, a->path, shared, a->file, &a->file);
    }

    void RemoveEmpty(const std::wstring& dir)
    {
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileExW((dir + L"\\*").c_str(), FindExInfoBasic, &fd, FindExSearchLimitToDirectories, nullptr, 0);
        if (h != INVALID_HANDLE_VALUE)
        {
            do
            {
                const std::wstring name = fd.cFileName;
                if (name == L"." || name == L"..") continue;
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) RemoveEmpty(dir + L"\\" + name);
            } while (FindNextFileW(h, &fd));
            FindClose(h);
        }
        RemoveDirectoryW(dir.c_str());
    }

    void Walk(const std::wstring& dir, std::vector<std::wstring> segs, std::vector<v31np::Item>& out, std::vector<std::wstring>& files)
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
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) Walk(full, segs, out, files);
            }
            else
            {
                out.push_back(FromFile(Join(segs), full, Join64(fd.nFileSizeHigh, fd.nFileSizeLow)));
                files.push_back(full);
            }
            segs.pop_back();
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }

    void Gather(const std::vector<std::wstring>& base, const std::vector<std::wstring>& paths, std::vector<v31np::Item>& out, std::vector<std::wstring>& files)
    {
        for (const auto& p : paths)
        {
            WIN32_FILE_ATTRIBUTE_DATA d;
            if (!GetFileAttributesExW(p.c_str(), GetFileExInfoStandard, &d)) continue;
            std::vector<std::wstring> segs = base;
            segs.push_back(NameOf(p));
            if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!(d.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) Walk(p, segs, out, files);
            }
            else
            {
                out.push_back(FromFile(Join(segs), p, Join64(d.nFileSizeHigh, d.nFileSizeLow)));
                files.push_back(p);
            }
        }
    }

    std::vector<v31np::Row> RowsOf(const Archive& a)
    {
        std::vector<v31np::Row> rows;
        for (size_t i = 0; i < a.index->entries.size(); i++)
        {
            const Link& l = a.index->links[i];
            v31np::Row r;
            r.kind = l.kind;
            r.key = a.index->entries[i].key;
            r.size = l.raw;
            r.at = a.index->entries[i].at;
            r.file = l.file;
            rows.push_back(std::move(r));
        }
        for (const auto& d : a.index->dirs)
        {
            v31np::Row r;
            r.kind = 'D';
            r.key = d;
            rows.push_back(std::move(r));
        }
        return rows;
    }

    std::shared_ptr<Archive> Writable(const Place& place, size_t& dir)
    {
        auto a = Load(place.path, place.chain);
        if (!a) return nullptr;
        dir = a->Find(place.sub);
        if (dir == v31np::None || !a->index->tree.nodes[dir].dir) return nullptr;
        return a;
    }

    std::string Prefix(const std::shared_ptr<Archive>& a, size_t dir)
    {
        return Join(StoredSegments(a->index->tree, dir));
    }

    std::string Under(const std::string& prefix, const std::wstring& name)
    {
        const std::string n = v31np::Narrow(name);
        return prefix.empty() ? n : prefix + "/" + n;
    }

    std::string Cat(const std::string& a, const std::string& b)
    {
        return a.empty() ? b : a + "/" + b;
    }

    std::vector<std::pair<size_t, size_t>> Spans(const std::string& key)
    {
        std::vector<std::pair<size_t, size_t>> out;
        size_t i = 0;
        while (i < key.size() && key[i] == '/') i++;
        if (key.size() - i >= 2 && key[i + 1] == ':' && ((key[i] >= 'A' && key[i] <= 'Z') || (key[i] >= 'a' && key[i] <= 'z')))
        {
            out.emplace_back(i, 2);
            i += 2;
        }
        while (i < key.size())
        {
            size_t j = key.find('/', i);
            if (j == std::string::npos) j = key.size();
            if (j > i) out.emplace_back(i, j - i);
            i = j + 1;
        }
        return out;
    }

    void Finish(std::wstring path)
    {
        std::thread([path]()
        {
            Lock hold;
            Sleep(1500);
            {
                std::lock_guard<std::mutex> g(cacheGuard);
                auto it = views.find(v31np::Fold(path));
                if (it != views.end() && it->second > 0) return;
            }
            Finalise(path);
        }).detach();
    }
}

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

size_t Index::Child(size_t dir, const std::wstring& natural) const
{
    auto it = lookup[dir].find(v31np::Fold(natural));
    return it == lookup[dir].end() ? v31np::None : it->second;
}

Archive::~Archive()
{
    if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
    for (auto& h : handles) CloseHandle(h.second);
}

bool Archive::ReadAt(uint64_t pos, void* buf, uint32_t len) const
{
    if (parent) return pos <= total && len <= total - pos && parent->ReadEntry(slot, pos, buf, len);
    HANDLE h = file;
    if (shared)
    {
        h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) return false;
        LARGE_INTEGER li;
        FILETIME ft;
        if (!GetFileSizeEx(h, &li) || !GetFileTime(h, nullptr, nullptr, &ft) || (uint64_t)li.QuadPart != total || Join64(ft.dwHighDateTime, ft.dwLowDateTime) != stamp)
        {
            CloseHandle(h);
            return false;
        }
    }
    BYTE* p = (BYTE*)buf;
    bool ok = true;
    while (len > 0)
    {
        OVERLAPPED o{};
        o.Offset = (DWORD)pos;
        o.OffsetHigh = (DWORD)(pos >> 32);
        DWORD got = 0;
        if (!ReadFile(h, p, len, &got, &o) || got == 0)
        {
            ok = false;
            break;
        }
        p += got;
        pos += got;
        len -= got;
    }
    if (shared) CloseHandle(h);
    return ok;
}

bool Archive::ReadEntry(size_t entry, uint64_t pos, void* buf, uint32_t len) const
{
    if (entry >= index->links.size()) return false;
    const Link& l = index->links[entry];
    if (pos > l.raw || len > l.raw - pos) return false;
    if (l.kind == 'A') return ReadAt(index->entries[entry].at + pos, buf, len);
    HANDLE h;
    {
        std::lock_guard<std::mutex> g(files);
        auto it = handles.find(l.file);
        if (it == handles.end())
        {
            HANDLE n = CreateFileW(l.file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (n == INVALID_HANDLE_VALUE) return false;
            it = handles.emplace(l.file, n).first;
        }
        h = it->second;
    }
    BYTE* p = (BYTE*)buf;
    while (len > 0)
    {
        OVERLAPPED o{};
        o.Offset = (DWORD)pos;
        o.OffsetHigh = (DWORD)(pos >> 32);
        DWORD got = 0;
        if (!ReadFile(h, p, len, &got, &o) || got == 0) return false;
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

std::wstring ArchiveId(const std::wstring& path, const std::vector<std::string>& chain)
{
    std::wstring id = v31np::Fold(path);
    for (const auto& key : chain)
    {
        id += L'\0';
        id += v31np::Wide(key);
    }
    return id;
}

std::shared_ptr<Archive> Load(const std::wstring& path, const std::vector<std::string>& chain)
{
    return Descend(OpenFile(path), path, chain);
}

std::wstring TempFile()
{
    wchar_t temp[MAX_PATH + 1];
    if (!GetTempPathW(ARRAYSIZE(temp), temp)) return {};
    const std::wstring dir = std::wstring(temp) + L"v31np";
    CreateDirectoryW(dir.c_str(), nullptr);
    GUID g;
    CoCreateGuid(&g);
    wchar_t tag[64];
    StringFromGUID2(g, tag, ARRAYSIZE(tag));
    return dir + L"\\" + tag + L".tmp";
}

std::vector<v31np::Row>* Edit::Part(const v31np::Chain& chain)
{
    auto it = shadow.parts.find(chain);
    if (it != shadow.parts.end()) return &it->second;
    auto a = Load(path, chain);
    if (!a) return nullptr;
    return &(shadow.parts[chain] = RowsOf(*a));
}

void Edit::Drop(const v31np::Chain& chain, const std::string& prefix)
{
    auto it = shadow.parts.find(chain);
    if (it != shadow.parts.end())
    {
        auto& rows = it->second;
        rows.erase(std::remove_if(rows.begin(), rows.end(), [&](const v31np::Row& r) { return !Keyless(r) && v31np::Under(r.key, prefix); }), rows.end());
        // Delete not — an emptied folder haunts the archive (as a D row) until the repack exorcises it. The copy engine
        // deletes the kids first and the folder last; if the folder vanishes early, it panics with 0x80070002.
        const size_t cut = prefix.rfind('/');
        if (cut != std::string::npos)
        {
            const std::string up = prefix.substr(0, cut);
            bool used = false;
            for (const auto& r : rows)
                if (!Keyless(r) && v31np::Under(r.key, up)) used = true;
            if (!used)
            {
                v31np::Row d;
                d.kind = 'D';
                d.key = up;
                rows.push_back(std::move(d));
            }
        }
    }
    for (auto s = shadow.parts.begin(); s != shadow.parts.end();)
    {
        const auto& c = s->first;
        const bool below = c.size() > chain.size() && std::equal(chain.begin(), chain.end(), c.begin()) && v31np::Under(c[chain.size()], prefix);
        s = below ? shadow.parts.erase(s) : std::next(s);
    }
}

void Edit::Put(const v31np::Chain& chain, v31np::Row row)
{
    auto p = Part(chain);
    if (!p) return;
    auto& rows = *p;
    const std::string& k = row.key;
    if (row.kind == 'D')
    {
        for (const auto& r : rows)
            if (!Keyless(r) && (r.key == k || v31np::Under(r.key, k))) return;
        std::vector<std::string> gone;
        for (const auto& r : rows)
            if (v31np::Stored(r) && v31np::Under(k, r.key)) gone.push_back(r.key);
        for (const auto& g : gone) Drop(chain, g);
        rows.push_back(std::move(row));
        return;
    }
    std::vector<std::string> gone;
    for (const auto& r : rows)
        if (v31np::Stored(r) && (v31np::Under(r.key, k) || v31np::Under(k, r.key))) gone.push_back(r.key);
    for (const auto& g : gone) Drop(chain, g);
    auto& now = *Part(chain);
    now.erase(std::remove_if(now.begin(), now.end(), [&](const v31np::Row& r) { return r.kind == 'D' && (v31np::Under(r.key, k) || v31np::Under(k, r.key)); }), now.end());
    now.push_back(std::move(row));
}

void Edit::Move(const v31np::Chain& chain, const std::string& from, const std::string& to, bool keep, const std::string* base, bool self)
{
    if (!(self && keep && base) && (from == to || v31np::Under(to, from))) return;
    auto p = Part(chain);
    if (!p) return;
    std::vector<v31np::Row> moving;
    for (const auto& r : *p)
        if (!Keyless(r) && v31np::Under(r.key, from)) moving.push_back(r);
    std::vector<std::pair<v31np::Chain, std::vector<v31np::Row>>> parts;
    for (const auto& s : shadow.parts)
    {
        const auto& c = s.first;
        if (c.size() > chain.size() && std::equal(chain.begin(), chain.end(), c.begin()) && v31np::Under(c[chain.size()], from))
            parts.emplace_back(c, s.second);
    }
    if (!keep) Drop(chain, from);
    std::map<std::string, std::string> renamed;
    for (auto& r : moving)
    {
        const std::string old = r.key;
        r.key = to + r.key.substr(from.size());
        if (base) r.key = Fresh(chain, r.key, *base, self, r.kind == 'D');
        renamed[old] = r.key;
        if (keep && r.kind == 'M') r.kind = 'F';
        Put(chain, std::move(r));
    }
    for (auto& s : parts)
    {
        v31np::Chain n = s.first;
        auto it = renamed.find(n[chain.size()]);
        n[chain.size()] = it != renamed.end() ? it->second : to + n[chain.size()].substr(from.size());
        shadow.parts[n] = std::move(s.second);
    }
}

std::string Edit::Fresh(const v31np::Chain& chain, std::string key, const std::string& base, bool self, bool dir)
{
    auto found = taken.find(chain);
    if (found == taken.end())
    {
        Taken t;
        if (auto p = Part(chain))
            for (const auto& r : *p)
            {
                if (!v31np::Stored(r) && r.kind != 'D') continue;
                const auto sp = Spans(r.key);
                std::string c;
                for (size_t i = 0; i < sp.size(); i++)
                {
                    c = Cat(c, r.key.substr(sp[i].first, sp[i].second));
                    if (i + 1 < sp.size() || r.kind == 'D') t.dirs.insert(c);
                }
                if (r.kind != 'D' && !sp.empty()) t.files.insert(c);
            }
        found = taken.emplace(chain, std::move(t)).first;
    }
    Taken& t = found->second;
    const auto sp = Spans(key);
    if (sp.empty()) return key;
    const size_t depth = Spans(base).size();
    auto pick = [&t](const std::string& at, const std::string& seg)
    {
        const std::wstring w = v31np::Wide(seg);
        for (size_t k = 1;; k++)
        {
            const std::string n = v31np::Narrow(v31np::Suffix(w, k));
            const std::string c = Cat(at, n);
            if (!t.files.count(c) && !t.dirs.count(c)) return n;
        }
    };
    std::vector<std::string> segs(sp.size());
    for (size_t i = 0; i < sp.size(); i++) segs[i] = key.substr(sp[i].first, sp[i].second);
    std::vector<std::string> now = segs;
    std::string orig;
    std::string cur;
    for (size_t i = 0; i < sp.size(); i++)
    {
        orig = Cat(orig, segs[i]);
        const bool folder = i + 1 < sp.size() || dir;
        if (i >= depth)
        {
            const std::string c = Cat(cur, segs[i]);
            if (folder)
            {
                auto m = t.moved.find(orig);
                if (m != t.moved.end()) now[i] = m->second;
                else
                {
                    if ((i == depth && self) || t.files.count(c)) now[i] = pick(cur, segs[i]);
                    t.moved[orig] = now[i];
                }
            }
            else if (t.files.count(c) || t.dirs.count(c)) now[i] = pick(cur, segs[i]);
        }
        cur = Cat(cur, now[i]);
        if (folder) t.dirs.insert(cur);
    }
    if (!dir) t.files.insert(cur);
    for (size_t i = sp.size(); i-- > 0;)
        if (now[i] != segs[i]) key.replace(sp[i].first, sp[i].second, now[i]);
    return key;
}

std::wstring Edit::Temp()
{
    std::wstring t = TempFile();
    if (!t.empty()) made.push_back(t);
    return t;
}

namespace
{
    struct Level
    {
        std::vector<v31np::Row> before;
        std::vector<v31np::Row> after;
        std::set<std::string> removed;
        std::vector<v31np::Row> added;
        std::map<std::string, std::string> source;
        std::set<std::string> concerned;
    };

    using Plan = std::map<v31np::Chain, Level>;

    enum class Shape
    {
        Absent,
        File,
        Folder,
    };

    struct Found
    {
        Shape shape = Shape::Absent;
        uint64_t size = 0;
    };

    bool Same(const v31np::Row& a, const v31np::Row& b)
    {
        return a.kind == b.kind && a.key == b.key && a.size == b.size && a.at == b.at && a.file == b.file;
    }

    Found StateOf(const std::vector<v31np::Row>& rows, const std::string& key)
    {
        Found s;
        for (const auto& r : rows)
        {
            if (Keyless(r)) continue;
            if (r.key == key && v31np::Stored(r)) return { Shape::File, r.size };
            if ((r.key == key && r.kind == 'D') || (r.key.size() > key.size() && v31np::Under(r.key, key))) s.shape = Shape::Folder;
        }
        return s;
    }

    void Diff(Level& l)
    {
        std::map<std::string, const v31np::Row*> b;
        std::map<std::string, const v31np::Row*> a;
        for (const auto& r : l.before)
            if (!Keyless(r)) b[r.key] = &r;
        for (const auto& r : l.after)
            if (!Keyless(r)) a[r.key] = &r;
        for (const auto& [k, r] : b)
        {
            auto it = a.find(k);
            if (it == a.end() || !Same(*r, *it->second)) l.removed.insert(k);
        }
        for (const auto& [k, r] : a)
        {
            auto it = b.find(k);
            if (it != b.end() && Same(*it->second, *r)) continue;
            l.added.push_back(*r);
            if (r->kind != 'A') continue;
            std::string pick;
            for (const auto& [bk, br] : b)
                if (br->kind == 'A' && br->at == r->at && br->size == r->size && (pick.empty() || (l.removed.count(bk) && !l.removed.count(pick)))) pick = bk;
            if (!pick.empty()) l.source[k] = pick;
        }
        l.concerned = l.removed;
        for (const auto& r : l.added) l.concerned.insert(r.key);
        for (const auto& s : l.source) l.concerned.insert(s.second);
    }

    using Deed = std::pair<std::string, std::vector<std::string>>;

    std::vector<Deed> Deeds(const Plan& plan, const char* hint)
    {
        std::vector<std::string> replaced;
        std::vector<std::string> adds;
        std::vector<std::string> dels;
        std::vector<std::string> moved;
        std::vector<std::string> copied;
        for (const auto& [chain, l] : plan)
        {
            std::set<std::string> addedKeys;
            std::set<std::string> targets;
            std::set<std::string> paired;
            for (const auto& r : l.added) addedKeys.insert(r.key);
            for (const auto& [dst, src] : l.source)
            {
                auto& list = l.removed.count(src) && !addedKeys.count(src) ? moved : copied;
                list.push_back(src);
                list.push_back(dst);
                if (&list == &moved) paired.insert(src);
                targets.insert(dst);
            }
            for (const auto& r : l.added)
            {
                if (targets.count(r.key)) continue;
                if (l.removed.count(r.key))
                {
                    if (r.kind != 'D') replaced.push_back(r.key);
                    continue;
                }
                if (r.kind == 'D' && StateOf(l.before, r.key).shape == Shape::Folder) continue;
                adds.push_back(r.key);
            }
            for (const auto& k : l.removed)
                if (!paired.count(k) && !addedKeys.count(k)) dels.push_back(k);
        }
        std::vector<Deed> out;
        if (!replaced.empty())
        {
            out.push_back({ "delete", replaced });
            out.push_back({ "add", replaced });
        }
        if (!moved.empty()) out.push_back({ hint && strcmp(hint, "rename") == 0 ? "rename" : "move", moved });
        if (!copied.empty()) out.push_back({ "copy", copied });
        if (!adds.empty()) out.push_back({ "add", adds });
        if (!dels.empty()) out.push_back({ "delete", dels });
        return out;
    }

    bool NowRows(const std::wstring& path, const v31np::Chain& chain, std::vector<v31np::Row>& out)
    {
        auto a = Load(path, chain);
        if (!a) return false;
        out = RowsOf(*a);
        return true;
    }

    bool Rebase(const Level& l, const std::vector<v31np::Row>& now, std::vector<v31np::Row>& out)
    {
        std::map<std::string, const v31np::Row*> stored;
        for (const auto& r : now)
            if (v31np::Stored(r)) stored[r.key] = &r;
        std::vector<v31np::Row> added = l.added;
        for (auto& r : added)
        {
            if (r.kind != 'A') continue;
            auto s = l.source.find(r.key);
            if (s == l.source.end()) return false;
            auto it = stored.find(s->second);
            if (it == stored.end()) return false;
            const std::string key = r.key;
            r = *it->second;
            r.key = key;
        }
        std::set<std::string> put;
        for (const auto& r : added) put.insert(r.key);
        out.clear();
        for (const auto& r : now)
            if (Keyless(r) || (!l.removed.count(r.key) && !put.count(r.key))) out.push_back(r);
        for (auto& r : added) out.push_back(std::move(r));
        return true;
    }

    bool WritePlain(const std::wstring& file, const std::string& text)
    {
        HANDLE h = CreateFileW(file.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
        if (h == INVALID_HANDLE_VALUE) return false;
        DWORD put = 0;
        const bool ok = WriteFile(h, text.data(), (DWORD)text.size(), &put, nullptr) && put == text.size();
        CloseHandle(h);
        return ok;
    }

    class WinRoad : public v31np::Road
    {
    public:
        WinRoad(const std::wstring& path, const v31np::Papers& me) : path(path), name(NameOf(path)), me(me), signal(PostPath(path)) {}

        ~WinRoad() override
        {
            if (!dialog) return;
            dialog->StopProgressDialog();
            dialog->Release();
        }

        bool Green(uint32_t waitMs) override { return signal.Green(waitMs); }
        void Red() override { signal.Red(); }
        bool ShowPapers(const std::string& papers) override { return signal.ShowPapers(papers); }
        bool ServerNow(int64_t& now) override { return signal.ServerNow(now); }
        uint64_t Ticks() override { return GetTickCount64(); }

        int64_t Clock() override
        {
            FILETIME ft;
            GetSystemTimeAsFileTime(&ft);
            return ((int64_t)Join64(ft.dwHighDateTime, ft.dwLowDateTime) - 116444736000000000ll) / 10000000ll;
        }

        void Sleep(uint32_t ms) override
        {
            Show();
            ::Sleep(ms);
        }

        bool Archive(uint64_t& size, uint64_t& stamp) override { return Attributes(path, size, stamp); }

        bool ReadJournal(std::string& text, bool& exists) override
        {
            text.clear();
            exists = false;
            const std::wstring journal = Chosen(path);
            if (journal.empty()) return true;
            uint64_t size = 0;
            uint64_t time = 0;
            if (!ReadText(journal, text, size, time)) return !Exists(journal);
            exists = true;
            v31np::Shadow s;
            holder.clear();
            if (!driving && v31np::ReadJournal(text, s) && s.officer.held && s.officer.lane.papers.plate != me.plate) holder = v31np::Wide(v31np::PapersLine(s.officer.lane.papers));
            if (dialog) dialog->SetLine(1, holder.c_str(), FALSE, nullptr);
            return true;
        }

        bool WriteJournal(const std::string& text) override
        {
            for (int tries = 0; tries < 40; tries++)
            {
                if (WriteText(JournalPath(path), text)) return true;
                ::Sleep(250);
            }
            return false;
        }

        bool Ask(const v31np::Papers& h) override
        {
            const std::wstring text = L"Still waiting for " + v31np::Wide(h.plate) + L" (" + v31np::Wide(h.os) + L"), which holds this archive. Go ahead anyway?";
            return MessageBoxW(nullptr, text.c_str(), name.c_str(), MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND) == IDYES;
        }

        bool Cancelled() override { return dialog && dialog->HasUserCancelled(); }

        void Driving()
        {
            driving = true;
            holder.clear();
            if (dialog) dialog->SetLine(1, L"", FALSE, nullptr);
        }

    private:
        void Show()
        {
            if (started) return;
            started = true;
            if (FAILED(CoCreateInstance(CLSID_ProgressDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))))
            {
                dialog = nullptr;
                return;
            }
            dialog->SetTitle(name.c_str());
            dialog->SetLine(1, holder.c_str(), FALSE, nullptr);
            if (SUCCEEDED(dialog->StartProgressDialog(nullptr, nullptr, PROGDLG_NORMAL | PROGDLG_NOTIME | PROGDLG_NOMINIMIZE | PROGDLG_MARQUEEPROGRESS, nullptr))) return;
            dialog->Release();
            dialog = nullptr;
        }

        std::wstring path;
        std::wstring name;
        v31np::Papers me;
        v31np::Signal signal;
        IProgressDialog* dialog = nullptr;
        bool started = false;
        bool driving = false;
        std::wstring holder;
    };

    class WinTrip : public v31np::Trip
    {
    public:
        WinTrip(const std::wstring& path, const Plan& plan, WinRoad& road) : path(path), plan(plan), road(road) {}

        ~WinTrip() override
        {
            for (const auto& t : temps) DeleteFileW(t.c_str());
        }

        HRESULT error = S_OK;

        bool Look(const v31np::Shadow&) override
        {
            road.Driving();
            Forget(path);
            for (const auto& [chain, l] : plan)
            {
                std::vector<v31np::Row> now;
                if (!NowRows(path, chain, now)) return false;
                for (const auto& k : l.concerned)
                {
                    const Found a = StateOf(l.before, k);
                    const Found b = StateOf(now, k);
                    if (a.shape != b.shape || (a.shape == Shape::File && a.size != b.size)) return false;
                }
            }
            return true;
        }

        bool Build(const std::vector<v31np::Record>& records, const std::function<bool()>& renew) override
        {
            Forget(path);
            if (!Attributes(path, size, stamp))
            {
                error = STG_E_FILENOTFOUND;
                return false;
            }
            auto shadow = std::make_shared<v31np::Shadow>();
            shadow->size = size;
            shadow->stamp = stamp;
            for (const auto& [chain, l] : plan)
            {
                std::vector<v31np::Row> now;
                std::vector<v31np::Row> rows;
                if (!NowRows(path, chain, now) || !Rebase(l, now, rows))
                {
                    error = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                    return false;
                }
                std::vector<std::string> keys;
                for (const auto& r : rows)
                    if (v31np::Stored(r)) keys.push_back(r.key);
                for (const auto& [key, text] : v31np::CustodyFiles(keys, records))
                {
                    const std::wstring f = TempFile();
                    if (f.empty() || !WritePlain(f, text))
                    {
                        error = E_FAIL;
                        return false;
                    }
                    temps.push_back(f);
                    v31np::Row r;
                    r.kind = 'F';
                    r.key = key;
                    r.size = text.size();
                    r.file = f;
                    rows.push_back(std::move(r));
                }
                shadow->parts[chain] = std::move(rows);
            }
            std::vector<std::wstring> flat;
            HRESULT r = S_OK;
            {
                auto root = OpenWith(path, shadow);
                std::vector<v31np::Item> items;
                r = root ? Items(root, items, flat) : HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
                if (SUCCEEDED(r) && (root->total != size || root->stamp != stamp)) r = E_FAIL;
                if (SUCCEEDED(r)) r = WriteBeside(path, items, true, beside, [&]() { return !road.Cancelled() && renew(); });
            }
            for (const auto& t : flat) DeleteFileW(t.c_str());
            for (const auto& t : temps) DeleteFileW(t.c_str());
            temps.clear();
            Forget(path);
            if (SUCCEEDED(r)) return true;
            error = r;
            return false;
        }

        bool Land(v31np::Shadow& next, int64_t) override
        {
            uint64_t s = 0;
            uint64_t t = 0;
            if (!Attributes(path, s, t) || s != size || t != stamp)
            {
                error = E_FAIL;
                return false;
            }
            const HRESULT r = Replace(beside, path, true);
            Forget(path);
            if (FAILED(r))
            {
                error = r;
                return false;
            }
            if (!Attributes(path, next.size, next.stamp)) return true;
            std::set<v31np::Chain> chains;
            for (const auto& p : next.parts) chains.insert(p.first);
            for (const auto& [chain, l] : plan)
                for (const auto& row : l.added)
                    if (row.kind == 'D') chains.insert(chain);
            auto fresh = OpenWith(path, nullptr);
            std::map<v31np::Chain, std::vector<v31np::Row>> parts;
            for (const auto& chain : chains)
            {
                std::vector<v31np::Row> leases;
                auto held = next.parts.find(chain);
                if (held != next.parts.end())
                    for (const auto& row : held->second)
                        if (row.kind == 'D') leases.push_back(row);
                auto mine = plan.find(chain);
                if (mine != plan.end())
                {
                    const Level& l = mine->second;
                    leases.erase(std::remove_if(leases.begin(), leases.end(), [&](const v31np::Row& d) { return l.removed.count(d.key) > 0; }), leases.end());
                    for (const auto& row : l.added)
                    {
                        if (row.kind != 'D') continue;
                        leases.erase(std::remove_if(leases.begin(), leases.end(), [&](const v31np::Row& d) { return d.key == row.key; }), leases.end());
                        v31np::Row d;
                        d.kind = 'D';
                        d.key = row.key;
                        leases.push_back(std::move(d));
                    }
                }
                if (leases.empty() || !fresh) continue;
                auto level = Descend(fresh, path, chain);
                if (!level) continue;
                std::vector<v31np::Row> rows = v31np::Listing(level->index->entries, {});
                bool any = false;
                for (const auto& d : leases)
                {
                    const bool landed = std::any_of(level->index->entries.begin(), level->index->entries.end(), [&](const v31np::Entry& e) { return v31np::Under(e.key, d.key); });
                    if (landed) continue;
                    rows.push_back(d);
                    any = true;
                }
                if (any) parts[chain] = std::move(rows);
            }
            next.parts = std::move(parts);
            Forget(path);
            return true;
        }

        void Abandon() override
        {
            beside.Drop();
            for (const auto& t : temps) DeleteFileW(t.c_str());
            temps.clear();
        }

    private:
        std::wstring path;
        const Plan& plan;
        WinRoad& road;
        Beside beside;
        std::vector<std::wstring> temps;
        uint64_t size = 0;
        uint64_t stamp = 0;
    };

    HRESULT Car(const std::wstring& path, const std::function<HRESULT(Edit&)>& change, const char* hint)
    {
        Checkpoint stop(path, false);
        uint64_t size = 0;
        uint64_t time = 0;
        if (!Attributes(path, size, time)) return STG_E_FILENOTFOUND;
        Edit view;
        view.path = path;
        {
            uint64_t jsize = 0;
            uint64_t jtime = 0;
            std::wstring journal;
            if (Current(path, size, time, view.shadow, jsize, jtime, journal) != State::Fresh) view.shadow = v31np::Shadow{};
            view.shadow.officer = v31np::Officer{};
            view.shadow.size = size;
            view.shadow.stamp = time;
        }
        Edit base = view;
        HRESULT r = change(view);
        auto clean = [&]()
        {
            for (const auto& t : view.made) DeleteFileW(t.c_str());
        };
        if (FAILED(r))
        {
            clean();
            return r;
        }
        Plan plan;
        for (const auto& [chain, rows] : view.shadow.parts)
        {
            auto b = base.Part(chain);
            if (!b) continue;
            Level l;
            l.before = *b;
            l.after = rows;
            Diff(l);
            if (!l.removed.empty() || !l.added.empty()) plan[chain] = std::move(l);
        }
        if (plan.empty())
        {
            clean();
            return S_OK;
        }
        v31np::Request q;
        q.me = { Mine(), v31np::System() };
        for (const auto& [chain, l] : plan)
            for (const auto& k : l.concerned) q.keys.push_back(k);
        for (auto& [action, lines] : Deeds(plan, hint)) q.steps.push_back({ std::move(action), std::move(lines) });
        v31np::Drove d;
        HRESULT error = S_OK;
        bool cancelled = false;
        {
            WinRoad road(path, q.me);
            WinTrip trip(path, plan, road);
            d = v31np::Drive(road, trip, q);
            error = trip.error;
            cancelled = road.Cancelled();
        }
        Forget(path);
        if (d == v31np::Drove::Done)
        {
            for (const auto& part : view.shadow.parts)
                for (const auto& row : part.second)
                    if (row.kind == 'M' || (row.kind == 'T' && OwnTemp(row.file))) DeleteFileW(row.file.c_str());
            for (const auto& part : view.shadow.parts)
                for (const auto& row : part.second)
                    if (row.kind == 'R') RemoveEmpty(row.file);
        }
        clean();
        if (d == v31np::Drove::Done || d == v31np::Drove::Refused) SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW | SHCNF_FLUSHNOWAIT, path.c_str(), nullptr);
        switch (d)
        {
        case v31np::Drove::Done:
            return S_OK;
        case v31np::Drove::Refused:
            return HRESULT_FROM_WIN32(ERROR_FILE_INVALID);
        case v31np::Drove::Lost:
            return HRESULT_FROM_WIN32(ERROR_LOCK_VIOLATION);
        case v31np::Drove::Skewed:
            return HRESULT_FROM_WIN32(ERROR_TIME_SKEW);
        case v31np::Drove::Cancelled:
            return HRESULT_FROM_WIN32(ERROR_CANCELLED);
        default:
            if (cancelled) return HRESULT_FROM_WIN32(ERROR_CANCELLED);
            return FAILED(error) ? error : E_FAIL;
        }
    }
}

HRESULT Mutate(const std::wstring& path, const std::function<HRESULT(Edit&)>& change, const char* hint)
{
    if (v31np::SharedRoad(path)) return Car(path, change, hint);
    Checkpoint stop(path, false);
    uint64_t size = 0;
    uint64_t time = 0;
    if (!Attributes(path, size, time)) return STG_E_FILENOTFOUND;
    Edit e;
    e.path = path;
    uint64_t jsize = 0;
    uint64_t jtime = 0;
    std::wstring journal;
    State st = Current(path, size, time, e.shadow, jsize, jtime, journal);
    if (st == State::Fresh && Foreign(e.shadow)) return HRESULT_FROM_WIN32(ERROR_WRITE_PROTECT);
    if (st == State::Stale) Discard(path, journal, e.shadow);
    if (st != State::Fresh)
    {
        e.shadow = v31np::Shadow{};
        e.shadow.size = size;
        e.shadow.stamp = time;
    }
    Forget(path);
    HRESULT r = change(e);
    std::vector<std::wstring> shown;
    if (SUCCEEDED(r)) shown = Orphans(e.shadow);
    Sign(e.shadow);
    if (SUCCEEDED(r) && !e.shadow.parts.empty() && !WriteText(JournalPath(path), v31np::WriteJournal(e.shadow))) r = E_FAIL;
    if (FAILED(r))
    {
        for (const auto& t : e.made) DeleteFileW(t.c_str());
        Forget(path);
        return r;
    }
    for (const auto& f : shown) Unhide(f);
    for (const auto& f : e.hide) Hide(f);
    Forget(path);
    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW | SHCNF_FLUSHNOWAIT, path.c_str(), nullptr);
    return S_OK;
}

HRESULT Materialise(const std::shared_ptr<Archive>& a, size_t entry, std::wstring& file, uint64_t& size)
{
    v31np::Chain chain = ChainOf(a.get());
    chain.push_back(a->index->entries[entry].key);
    auto n = Load(a->path, chain);
    if (!n) return HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
    return Flatten(n, file, size);
}

HRESULT CopyOut(const std::shared_ptr<Archive>& a, size_t entry, Edit& into, v31np::Row& row)
{
    const Link& l = a->index->links[entry];
    if (l.kind != 'A' && !l.pending && l.file.empty()) return E_ACCESSDENIED;
    if (l.pending)
    {
        std::wstring f;
        uint64_t s = 0;
        HRESULT r = Materialise(a, entry, f, s);
        if (FAILED(r)) return r;
        into.made.push_back(f);
        row.kind = 'T';
        row.file = f;
        row.size = s;
        return S_OK;
    }
    if (l.kind == 'F' || l.kind == 'M')
    {
        row.kind = 'F';
        row.file = l.file;
        row.size = l.raw;
        return S_OK;
    }
    const std::wstring t = into.Temp();
    if (t.empty()) return E_FAIL;
    const bool ok = l.kind == 'T' ? CopyFileW(l.file.c_str(), t.c_str(), FALSE) != FALSE : CopyRange(a, entry, t);
    if (!ok) return E_FAIL;
    row.kind = 'T';
    row.file = t;
    row.size = l.raw;
    return S_OK;
}

HRESULT Finalise(const std::wstring& path)
{
    Checkpoint stop(path, false);
    uint64_t size = 0;
    uint64_t time = 0;
    if (!Attributes(path, size, time)) return S_OK;
    v31np::Shadow s;
    uint64_t jsize = 0;
    uint64_t jtime = 0;
    std::wstring journal;
    if (stop.shared) return S_OK;
    State st = Current(path, size, time, s, jsize, jtime, journal);
    if (st == State::Stale) Discard(path, journal, s);
    if (st != State::Fresh || Foreign(s)) return S_OK;
    Forget(path);
    auto a = Load(path, {});
    if (!a) return E_FAIL;
    std::vector<v31np::Item> items;
    std::vector<std::wstring> temps;
    HRESULT r = Items(a, items, temps);
    if (SUCCEEDED(r)) r = Commit(a, items, false);
    items.clear();
    a.reset();
    for (const auto& t : temps) DeleteFileW(t.c_str());
    if (FAILED(r)) return r;
    for (const auto& part : s.parts)
        for (const auto& row : part.second)
            if (row.kind == 'M' || (row.kind == 'T' && OwnTemp(row.file))) DeleteFileW(row.file.c_str());
    for (const auto& part : s.parts)
        for (const auto& row : part.second)
            if (row.kind == 'R') RemoveEmpty(row.file);
    DeleteFileW(journal.c_str());
    for (const auto& part : s.parts)
        for (const auto& row : part.second)
            if (row.kind == 'H') Unhide(row.file);
    Forget(path);
    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW | SHCNF_FLUSHNOWAIT, path.c_str(), nullptr);
    return S_OK;
}

namespace
{
    std::map<std::wstring, std::wstring> roads;
    bool renewing = false;

    bool Holds(const std::wstring& path, const std::string& plate)
    {
        const std::wstring journal = Chosen(path);
        if (journal.empty()) return false;
        std::string text;
        uint64_t size = 0;
        uint64_t time = 0;
        v31np::Shadow s;
        if (!ReadText(journal, text, size, time) || !v31np::ReadJournal(text, s)) return false;
        for (const auto& part : s.parts)
            for (const auto& r : part.second)
                if (r.kind == 'D' && r.plate == plate) return true;
        return false;
    }

    void Renewals()
    {
        Lock hold;
        const v31np::Papers me{ Mine(), v31np::System() };
        for (;;)
        {
            for (int64_t i = 0; i < v31np::Renewal; i++)
            {
                Sleep(1000);
                std::lock_guard<std::mutex> g(cacheGuard);
                if (!roads.empty()) continue;
                renewing = false;
                return;
            }
            std::vector<std::wstring> paths;
            {
                std::lock_guard<std::mutex> g(cacheGuard);
                for (const auto& r : roads) paths.push_back(r.second);
            }
            for (const auto& path : paths)
            {
                if (!Holds(path, me.plate)) continue;
                WinRoad road(path, me);
                v31np::Visit(road, me, [&](v31np::Shadow& s, int64_t now) { v31np::RenewFolders(s, me.plate, now, v31np::Lease); });
                Forget(path);
            }
        }
    }
}

void Enter(const std::wstring& path)
{
    const bool shared = v31np::SharedRoad(path);
    std::lock_guard<std::mutex> g(cacheGuard);
    const std::wstring f = v31np::Fold(path);
    views[f]++;
    if (!shared) return;
    roads[f] = path;
    if (renewing) return;
    renewing = true;
    std::thread(Renewals).detach();
}

void Leave(const std::wstring& path)
{
    {
        std::lock_guard<std::mutex> g(cacheGuard);
        const std::wstring f = v31np::Fold(path);
        auto it = views.find(f);
        if (it == views.end()) return;
        if (--it->second > 0) return;
        views.erase(it);
        roads.erase(f);
    }
    if (HasJournal(path)) Finish(path);
}

v31np::Item Keep(const std::shared_ptr<Archive>& a, size_t entry, const std::string& key)
{
    auto pos = std::make_shared<uint64_t>(0);
    return { key, a->index->links[entry].raw, [a, entry, pos](void* buf, uint32_t len)
    {
        if (!a->ReadEntry(entry, *pos, buf, len)) return false;
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
    const std::string& key = a->index->entries[entry].key;
    h = Mix(h, key.data(), key.size() + 1);
    const Link& l = a->index->links[entry];
    h = Mix(h, &l.kind, 1);
    h = Mix(h, l.raw);
    h = Mix(h, a->index->entries[entry].at);
    h = Mix(h, l.file.data(), l.file.size() * sizeof(wchar_t));
    const Archive* r = a.get();
    for (; r->parent; r = r->parent.get()) h = Mix(h, r->key.data(), r->key.size() + 1);
    const std::wstring path = v31np::Fold(r->path);
    h = Mix(h, path.data(), path.size() * sizeof(wchar_t));
    h = Mix(h, r->total);
    h = Mix(h, r->stamp);
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

HRESULT AddPaths(const Place& place, const std::vector<std::wstring>& paths, bool move)
{
    if (paths.empty()) return E_INVALIDARG;
    return Mutate(place.path, [&](Edit& e)
    {
        size_t dir;
        auto a = Writable(place, dir);
        if (!a) return E_ACCESSDENIED;
        if (!e.Part(place.chain)) return E_FAIL;
        std::vector<v31np::Item> found;
        std::vector<std::wstring> files;
        const std::vector<std::wstring> segs = StoredSegments(a->index->tree, dir);
        const std::string base = Join(segs);
        Gather(segs, paths, found, files);
        a.reset();
        for (size_t i = 0; i < found.size(); i++)
        {
            v31np::Row r;
            r.kind = move ? 'M' : 'F';
            r.key = e.Fresh(place.chain, found[i].key, base, false, false);
            r.size = found[i].size;
            r.file = files[i];
            e.Put(place.chain, std::move(r));
        }
        for (const auto& p : paths)
        {
            WIN32_FILE_ATTRIBUTE_DATA d;
            if (!GetFileAttributesExW(p.c_str(), GetFileExInfoStandard, &d)) continue;
            const bool folder = (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(d.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT);
            if (move && folder)
            {
                v31np::Row r;
                r.kind = 'R';
                r.file = p;
                e.Part(place.chain)->push_back(std::move(r));
            }
            if (move && (folder || !(d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) && Visible(p))
            {
                v31np::Row h;
                h.kind = 'H';
                h.file = p;
                e.Part(place.chain)->push_back(std::move(h));
                e.hide.push_back(p);
            }
        }
        return found.empty() ? E_INVALIDARG : S_OK;
    });
}

HRESULT RemoveNodes(const Place& place, const std::vector<std::wstring>& names)
{
    return Mutate(place.path, [&](Edit& e)
    {
        size_t dir;
        auto a = Writable(place, dir);
        if (!a) return E_ACCESSDENIED;
        if (!e.Part(place.chain)) return E_FAIL;
        std::vector<std::string> keys;
        for (const auto& n : names)
        {
            size_t node = a->index->Child(dir, n);
            if (node != v31np::None) keys.push_back(Join(StoredSegments(a->index->tree, node)));
        }
        a.reset();
        for (const auto& k : keys) e.Drop(place.chain, k);
        return S_OK;
    });
}

HRESULT RenameNode(const Place& place, const std::wstring& natural, const std::wstring& stored)
{
    if (stored.empty() || stored.find(L'/') != std::wstring::npos) return E_INVALIDARG;
    return Mutate(place.path, [&](Edit& e)
    {
        size_t dir;
        auto a = Writable(place, dir);
        if (!a) return E_ACCESSDENIED;
        size_t node = a->index->Child(dir, natural);
        if (node == v31np::None) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        for (size_t k : a->index->tree.nodes[dir].kids)
            if (k != node && a->index->tree.nodes[k].stored == stored) return HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        const std::string from = Join(StoredSegments(a->index->tree, node));
        const std::string to = Under(Prefix(a, dir), stored);
        a.reset();
        if (from == to) return S_OK;
        if (!e.Part(place.chain)) return E_FAIL;
        e.Move(place.chain, from, to, false);
        return S_OK;
    }, "rename");
}

HRESULT Transfer(const Place& from, const std::vector<std::wstring>& names, const Place& to, bool move)
{
    const bool sameFile = v31np::Fold(from.path) == v31np::Fold(to.path);
    if (sameFile && from.chain == to.chain)
    {
        return Mutate(to.path, [&](Edit& e)
        {
            size_t src;
            auto a = Writable(from, src);
            if (!a) return E_ACCESSDENIED;
            size_t dst = a->Find(to.sub);
            if (dst == v31np::None || !a->index->tree.nodes[dst].dir) return E_ACCESSDENIED;
            const std::string base = Prefix(a, dst);
            struct Step
            {
                std::string from;
                std::string to;
                bool self = false;
            };
            std::vector<Step> plan;
            for (const auto& n : names)
            {
                size_t node = a->index->Child(src, n);
                if (node == v31np::None) continue;
                const std::string k = Join(StoredSegments(a->index->tree, node));
                const std::string t = Under(base, a->index->tree.nodes[node].stored);
                const bool self = k == t;
                if (self && move) continue;
                if (!self && v31np::Under(t, k)) return E_INVALIDARG;
                plan.push_back({ k, t, self });
            }
            a.reset();
            if (!e.Part(from.chain)) return E_FAIL;
            for (const auto& p : plan) e.Move(from.chain, p.from, p.to, !move, &base, p.self);
            return S_OK;
        });
    }
    std::vector<std::string> taken;
    std::set<std::wstring> carried;
    std::set<std::wstring> hidden;
    HRESULT r = Mutate(to.path, [&](Edit& e)
    {
        size_t src;
        auto a = Writable(from, src);
        if (!a) return E_ACCESSDENIED;
        size_t dst;
        auto b = Writable(to, dst);
        if (!b) return E_ACCESSDENIED;
        const std::string base = Prefix(b, dst);
        b.reset();
        if (!e.Part(to.chain)) return E_FAIL;
        const std::string custody = "ALICEV31NP";
        const bool road = v31np::SharedRoad(to.path);
        std::string records;
        auto brought = [&](const std::string& key) -> std::string
        {
            if (!road || !v31np::Under(key, custody)) return key;
            if (records.empty())
            {
                std::wstring n = from.chain.empty() ? NameOf(from.path) : v31np::Wide(from.chain.back());
                const size_t slash = n.find_last_of(L'/');
                if (slash != std::wstring::npos) n = n.substr(slash + 1);
                if (IsPack(n)) n.resize(n.size() - 6);
                auto taken = [&](const std::string& c)
                {
                    for (const auto& r : *e.Part(to.chain))
                        if (!Keyless(r) && v31np::Under(r.key, c)) return true;
                    return false;
                };
                std::string pick = v31np::Narrow(n);
                for (size_t k = 1; taken(custody + "/" + pick); k++) pick = v31np::Narrow(v31np::Suffix(n, k));
                records = custody + "/" + pick;
            }
            return records + key.substr(custody.size());
        };
        for (const auto& n : names)
        {
            size_t node = a->index->Child(src, n);
            if (node == v31np::None) continue;
            const std::string k = Join(StoredSegments(a->index->tree, node));
            const std::string t = Under(base, a->index->tree.nodes[node].stored);
            taken.push_back(k);
            std::vector<size_t> files;
            a->index->tree.Files(node, files);
            for (size_t f : files)
            {
                const size_t entry = a->index->tree.nodes[f].entry;
                const std::string& key = a->index->entries[entry].key;
                v31np::Row row;
                row.key = e.Fresh(to.chain, brought(t + key.substr(k.size())), base, false, false);
                HRESULT h = CopyOut(a, entry, e, row);
                if (FAILED(h)) return h;
                if (move && a->index->links[entry].kind == 'M')
                {
                    row.kind = 'M';
                    if (!sameFile) carried.insert(v31np::Fold(row.file));
                }
                e.Put(to.chain, std::move(row));
            }
            for (const auto& d : a->index->dirs)
                if (v31np::Under(d, k))
                {
                    v31np::Row row;
                    row.kind = 'D';
                    row.key = e.Fresh(to.chain, brought(t + d.substr(k.size())), base, false, true);
                    e.Put(to.chain, std::move(row));
                }
        }
        if (!carried.empty())
            if (auto s = JournalOf(from.path))
                for (const auto& part : s->parts)
                    for (const auto& h : part.second)
                        if (h.kind == 'H' && carried.count(v31np::Fold(h.file)))
                        {
                            e.Part(to.chain)->push_back(h);
                            hidden.insert(v31np::Fold(h.file));
                        }
        return S_OK;
    });
    if (FAILED(r) || !move || taken.empty()) return r;
    return Mutate(from.path, [&](Edit& e)
    {
        if (!e.Part(from.chain)) return E_FAIL;
        for (const auto& k : taken) e.Drop(from.chain, k);
        for (auto& part : e.shadow.parts)
        {
            auto& rows = part.second;
            rows.erase(std::remove_if(rows.begin(), rows.end(), [&](const v31np::Row& x) { return x.kind == 'H' && hidden.count(v31np::Fold(x.file)); }), rows.end());
        }
        return S_OK;
    });
}

HRESULT PackNodes(const Place& place, const std::vector<std::wstring>& names)
{
    size_t dir;
    auto a = Writable(place, dir);
    if (!a || names.empty()) return E_ACCESSDENIED;
    const auto& t = a->index->tree;
    const std::string prefix = Prefix(a, dir);
    const size_t cut = prefix.empty() ? 0 : prefix.size() + 1;
    std::vector<v31np::Item> items;
    std::vector<std::wstring> temps;
    std::vector<std::string> moved;
    std::wstring first;
    bool firstFile = false;
    HRESULT r = S_OK;
    for (const auto& n : names)
    {
        const size_t node = a->index->Child(dir, n);
        if (node == v31np::None) continue;
        moved.push_back(Join(StoredSegments(t, node)));
        if (first.empty())
        {
            first = t.nodes[node].stored;
            firstFile = !t.nodes[node].dir;
        }
        std::vector<size_t> files;
        t.Files(node, files);
        for (size_t f : files)
        {
            const size_t entry = t.nodes[f].entry;
            const std::string key = a->index->entries[entry].key.substr(cut);
            if (a->index->links[entry].pending)
            {
                std::wstring file;
                uint64_t size = 0;
                r = Materialise(a, entry, file, size);
                if (FAILED(r)) break;
                temps.push_back(file);
                items.push_back(FromFile(key, file, size));
            }
            else items.push_back(Keep(a, entry, key));
        }
        if (FAILED(r)) break;
    }
    if (SUCCEEDED(r) && items.empty()) r = E_INVALIDARG;
    std::wstring packed;
    uint64_t size = 0;
    if (SUCCEEDED(r))
    {
        packed = TempFile();
        r = packed.empty() ? E_FAIL : WriteItems(packed, items, size);
    }
    items.clear();
    for (const auto& f : temps) DeleteFileW(f.c_str());
    if (FAILED(r))
    {
        if (!packed.empty()) DeleteFileW(packed.c_str());
        return r;
    }
    std::wstring base = first;
    if (firstFile)
    {
        const size_t dot = base.rfind(L'.');
        if (dot != std::wstring::npos && dot > 0) base.resize(dot);
    }
    base += L".v31np";
    std::wstring name = base;
    auto taken = [&](const std::wstring& x)
    {
        for (size_t k : t.nodes[dir].kids)
            if (v31np::Fold(t.nodes[k].stored) == v31np::Fold(x) || v31np::Fold(t.nodes[k].natural) == v31np::Fold(x)) return true;
        return false;
    };
    for (size_t k = 1; taken(name); k++) name = v31np::Suffix(base, k);
    const std::string key = Under(prefix, name);
    a.reset();
    r = Mutate(place.path, [&](Edit& e)
    {
        if (!e.Part(place.chain)) return E_FAIL;
        v31np::Row row;
        row.kind = 'T';
        row.key = key;
        row.size = size;
        row.file = packed;
        e.made.push_back(packed);
        for (const auto& k : moved) e.Drop(place.chain, k);
        e.Put(place.chain, std::move(row));
        return S_OK;
    });
    return r;
}

HRESULT CreateFromPaths(const std::wstring& target, const std::vector<std::wstring>& paths)
{
    std::vector<v31np::Item> items;
    std::vector<std::wstring> files;
    Gather({}, paths, items, files);
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
