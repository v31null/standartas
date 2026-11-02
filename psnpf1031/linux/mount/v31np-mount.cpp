#define FUSE_USE_VERSION 31

#include "change.h"
#include "officer.h"
#include "road.h"
#include "order.h"
#include "shadow.h"
#include "tree.h"
#include "v31np.h"

#include <fuse.h>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <spawn.h>
#include <sys/file.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

namespace
{
    struct Identity
    {
        dev_t dev = 0;
        ino_t ino = 0;
        off_t size = -1;
        struct timespec mtime = {};

        bool operator==(const Identity& o) const
        {
            return dev == o.dev && ino == o.ino && size == o.size && mtime.tv_sec == o.mtime.tv_sec && mtime.tv_nsec == o.mtime.tv_nsec;
        }
    };

    Identity Of(const struct stat& st)
    {
        return { st.st_dev, st.st_ino, st.st_size, st.st_mtim };
    }

    bool Probe(const std::string& path, Identity& id)
    {
        struct stat st;
        if (stat(path.c_str(), &st) != 0) return false;
        id = Of(st);
        return true;
    }

    uint64_t FileTime(const struct timespec& t)
    {
        return (uint64_t)t.tv_sec * 10000000ull + (uint64_t)t.tv_nsec / 100ull + 116444736000000000ull;
    }

    struct Source
    {
        int fd = -1;
        ~Source()
        {
            if (fd >= 0) close(fd);
        }
    };

    struct Node
    {
        std::string name;
        v31np::Chain chain;
        change::Path stored;
        bool dir = false;
        bool nested = false;
        bool inner = false;
        uint64_t size = 0;
        char kind = 0;
        uint64_t at = 0;
        std::string file;
        std::vector<size_t> kids;
    };

    struct Snapshot
    {
        std::shared_ptr<Source> archive;
        Identity id;
        std::map<std::string, Identity> journals;
        std::vector<Node> nodes;
        std::unordered_map<std::string, size_t> paths;
        std::map<v31np::Chain, size_t> levels;
        v31np::Shadow shadow;
        bool editable = false;
    };

    struct Pending
    {
        v31np::Chain chain;
        change::Path key;
        bool dirty = false;
        bool archived = false;
        bool written = false;
    };

    struct Handle
    {
        std::shared_ptr<Snapshot> snap;
        uint64_t at = 0;
        uint64_t size = 0;
        bool archive = false;
        int fd = -1;
        bool write = false;
        std::string temp;
    };

    struct Mount
    {
        std::string archive;
        std::string folder;
        std::string stem;
        std::string cache;
        std::string plate;
        std::string system;
        bool local = false;
        bool road = false;
        std::map<std::string, Pending> held;
        std::shared_ptr<Snapshot> now;
        std::recursive_mutex guard;
        std::set<Handle*> open;
        std::chrono::steady_clock::time_point active = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point seen = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point renewed = std::chrono::steady_clock::now();
        std::chrono::seconds leave{ 0 };
        std::string point;
        bool pending = false;
        std::atomic<bool> stop{ false };
        std::thread idle;
    } mount;

    constexpr auto Wait = std::chrono::milliseconds(1500);

    bool ReadAll(int fd, uint64_t pos, void* buf, uint32_t len)
    {
        uint8_t* p = (uint8_t*)buf;
        while (len > 0)
        {
            ssize_t n = pread(fd, p, len, (off_t)pos);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) return false;
            p += n;
            pos += (uint64_t)n;
            len -= (uint32_t)n;
        }
        return true;
    }

    bool WriteAll(int fd, const void* buf, size_t len)
    {
        const uint8_t* p = (const uint8_t*)buf;
        while (len > 0)
        {
            ssize_t n = write(fd, p, len);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) return false;
            p += n;
            len -= (size_t)n;
        }
        return true;
    }

    std::string Slurp(const std::string& path, Identity& id)
    {
        int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
        if (fd < 0) return {};
        struct stat st;
        std::string text;
        if (fstat(fd, &st) == 0 && S_ISREG(st.st_mode) && st.st_size < (64 << 20))
        {
            id = Of(st);
            text.resize((size_t)st.st_size);
            if (!ReadAll(fd, 0, text.data(), (uint32_t)text.size())) text.clear();
        }
        close(fd);
        return text;
    }

    std::string Hex32()
    {
        uint8_t raw[16];
        size_t got = 0;
        while (got < sizeof raw)
        {
            ssize_t n = getrandom(raw + got, sizeof raw - got, 0);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) break;
            got += (size_t)n;
        }
        if (got < sizeof raw)
        {
            const uint64_t t = (uint64_t)std::chrono::steady_clock::now().time_since_epoch().count();
            for (size_t i = 0; i < sizeof raw; i++) raw[i] = (uint8_t)((t >> ((i % 8) * 8)) ^ (uint64_t)getpid() ^ i);
        }
        static const char digits[] = "0123456789abcdef";
        std::string out;
        for (uint8_t b : raw)
        {
            out += digits[b >> 4];
            out += digits[b & 15];
        }
        return out;
    }

    bool SyncFolder(const std::string& folder)
    {
        int fd = open(folder.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        if (fd < 0) return false;
        const bool ok = fsync(fd) == 0;
        close(fd);
        return ok;
    }

    bool Own(const std::wstring& wide)
    {
        const std::string file = v31np::Narrow(wide);
        const size_t slash = file.rfind('/');
        if (slash == std::string::npos) return false;
        const std::string name = file.substr(slash + 1);
        if (name.size() != 36 || name.compare(32, 4, ".tmp") != 0) return false;
        for (size_t i = 0; i < 32; i++)
            if (!((name[i] >= '0' && name[i] <= '9') || (name[i] >= 'a' && name[i] <= 'f'))) return false;
        char* real = realpath(file.substr(0, slash).c_str(), nullptr);
        if (!real) return false;
        const bool ours = mount.cache == real;
        std::free(real);
        return ours;
    }

    bool Busy(const std::string& temp)
    {
        for (const Handle* h : mount.open)
            if (h->write && h->temp == temp) return true;
        return false;
    }

    void Unref(const std::wstring& file, const change::Parts& parts)
    {
        if (!Own(file) || change::Refers(parts, file) || Busy(v31np::Narrow(file))) return;
        unlink(v31np::Narrow(file).c_str());
    }

    std::vector<std::string> JournalPaths()
    {
        const std::string name = v31np::Narrow(v31np::JournalName(v31np::Wide(mount.stem + ".v31np")));
        return { mount.folder + "/" + name, mount.folder + "/." + name };
    }

    enum class Kind
    {
        Plain,
        Rich,
        Foreign
    };

    Kind Classify(const v31np::Shadow& s)
    {
        bool rich = false;
        for (const auto& part : s.parts)
        {
            for (const auto& r : part.second)
            {
                if (r.kind == 'A' || r.kind == 'D') continue;
                if (!v31np::Ours(r, mount.plate) || r.file.empty() || r.file[0] != L'/') return Kind::Foreign;
                if (r.kind != 'T') rich = true;
            }
        }
        return rich ? Kind::Rich : Kind::Plain;
    }

    struct Found
    {
        std::string path;
        Identity id;
        bool readable = false;
        bool fresh = false;
        Kind kind = Kind::Plain;
        v31np::Shadow shadow;
    };

    std::vector<Found> Scan(const Identity& archive)
    {
        std::vector<Found> out;
        for (const auto& p : JournalPaths())
        {
            Found f;
            f.path = p;
            if (!Probe(p, f.id)) continue;
            const std::string text = Slurp(p, f.id);
            f.readable = v31np::ReadJournal(text, f.shadow);
            if (f.readable)
            {
                f.fresh = f.shadow.size == (uint64_t)archive.size && f.shadow.stamp == FileTime(archive.mtime);
                f.kind = Classify(f.shadow);
            }
            out.push_back(std::move(f));
        }
        return out;
    }

    const Found* Newest(const std::vector<Found>& found)
    {
        const Found* best = nullptr;
        int64_t newest = 0;
        for (const auto& f : found)
        {
            if (!f.readable || !f.fresh) continue;
            const int64_t t = (int64_t)f.id.mtime.tv_sec * 1000000000 + f.id.mtime.tv_nsec;
            if (best && t <= newest) continue;
            best = &f;
            newest = t;
        }
        return best;
    }

    bool Blocked(const std::vector<Found>& found)
    {
        for (const auto& f : found)
        {
            if (!f.readable) continue;
            if (f.fresh && f.kind != Kind::Plain) return true;
            if (!f.fresh && f.kind == Kind::Rich) return true;
        }
        return false;
    }

    std::string Shown(const std::string& stored)
    {
        if (stored.empty() || stored == "." || stored == ".." || stored.size() > 255)
            return v31np::Narrow(v31np::Natural(v31np::Wide(stored)));
        return stored;
    }

    void Name(Snapshot& s, size_t d)
    {
        auto& kids = s.nodes[d].kids;
        std::vector<std::string> wanted(kids.size());
        std::map<std::string, int> count;
        for (size_t i = 0; i < kids.size(); i++)
        {
            wanted[i] = Shown(s.nodes[kids[i]].name);
            count[wanted[i]]++;
        }
        std::vector<size_t> pass;
        for (size_t i = 0; i < kids.size(); i++)
            if (wanted[i] == s.nodes[kids[i]].name) pass.push_back(i);
        for (size_t i = 0; i < kids.size(); i++)
            if (wanted[i] != s.nodes[kids[i]].name) pass.push_back(i);
        std::map<std::string, bool> used;
        for (size_t i : pass)
        {
            std::string name = wanted[i];
            if (used.count(name))
            {
                for (size_t k = 1;; k++)
                {
                    std::string cand = v31np::Narrow(v31np::Suffix(v31np::Wide(wanted[i]), k));
                    if (!used.count(cand) && !count.count(cand))
                    {
                        name = cand;
                        break;
                    }
                }
            }
            used[name] = true;
            s.nodes[kids[i]].name = name;
        }
        std::sort(kids.begin(), kids.end(), [&](size_t a, size_t b)
            {
                if (s.nodes[a].dir != s.nodes[b].dir) return s.nodes[a].dir;
                return v31np::Order(v31np::Wide(s.nodes[a].name), v31np::Wide(s.nodes[b].name)) < 0;
            });
        for (size_t k : kids)
            if (s.nodes[k].dir) Name(s, k);
    }

    size_t Place(Snapshot& s, std::map<std::pair<size_t, std::string>, size_t>& dirs, size_t root, const change::Path& segs, size_t upto, bool inner, const v31np::Chain& chain)
    {
        size_t cur = root;
        for (size_t k = 0; k < upto; k++)
        {
            auto it = dirs.find({ cur, segs[k] });
            if (it == dirs.end() || !s.nodes[it->second].dir)
            {
                Node n;
                n.name = segs[k];
                n.chain = chain;
                n.stored.assign(segs.begin(), segs.begin() + (std::ptrdiff_t)k + 1);
                n.dir = true;
                n.inner = inner;
                s.nodes.push_back(n);
                const size_t id = s.nodes.size() - 1;
                s.nodes[cur].kids.push_back(id);
                it = dirs.insert_or_assign({ cur, segs[k] }, id).first;
            }
            cur = it->second;
        }
        return cur;
    }

    constexpr int Depth = 16;

    bool Nested(const std::string& name)
    {
        return name.size() > 6 && strcasecmp(name.c_str() + name.size() - 6, ".v31np") == 0;
    }

    bool Inner(int fd, uint64_t base, uint64_t size, std::vector<v31np::Row>& rows)
    {
        std::vector<v31np::Entry> entries;
        auto read = [fd, base, size](uint64_t pos, void* buf, uint32_t len) { return pos + len <= size && ReadAll(fd, base + pos, buf, len); };
        if (!v31np::Parse(read, size, entries)) return false;
        rows.clear();
        for (const auto& e : entries) rows.push_back({ 'A', e.key, e.size, e.at, {}, {}, 0 });
        return true;
    }

    void Populate(Snapshot& s, size_t root, const std::vector<v31np::Row>& rows, const std::string& file, uint64_t base, const v31np::Chain& chain, const v31np::Shadow& shadow, int depth)
    {
        std::map<std::pair<size_t, std::string>, size_t> dirs;
        const bool inner = depth > 0;
        for (const auto& r : rows)
        {
            if (r.key.empty()) continue;
            const change::Path segs = change::Split(r.key);
            if (segs.empty()) continue;
            if (r.kind == 'D')
            {
                Place(s, dirs, root, segs, segs.size(), inner, chain);
                continue;
            }
            if (!v31np::Stored(r) || !v31np::Ours(r, mount.plate)) continue;
            size_t parent = Place(s, dirs, root, segs, segs.size() - 1, inner, chain);
            Node f;
            f.name = segs.back();
            f.chain = chain;
            f.stored = segs;
            f.inner = inner;
            f.size = r.size;
            f.kind = r.kind;
            f.file = r.kind == 'A' ? file : v31np::Narrow(r.file);
            f.at = r.kind == 'A' ? base + r.at : 0;
            if (r.kind != 'A')
            {
                struct stat fs;
                if (f.file.empty() || f.file[0] != '/' || stat(f.file.c_str(), &fs) != 0 || !S_ISREG(fs.st_mode)) continue;
                f.size = (uint64_t)fs.st_size;
            }
            std::vector<v31np::Row> deeper_rows;
            bool open = false;
            if (depth < Depth && Nested(f.name))
            {
                v31np::Chain deeper = chain;
                deeper.push_back(r.key);
                auto section = shadow.parts.find(deeper);
                if (section != shadow.parts.end())
                {
                    deeper_rows = section->second;
                    open = true;
                }
                else
                {
                    const int fd = f.file.empty() ? s.archive->fd : ::open(f.file.c_str(), O_RDONLY | O_CLOEXEC);
                    if (fd >= 0)
                    {
                        open = Inner(fd, f.at, f.size, deeper_rows);
                        if (!f.file.empty()) close(fd);
                    }
                }
                if (open)
                {
                    f.dir = true;
                    f.nested = true;
                    v31np::Chain next = chain;
                    next.push_back(r.key);
                    s.nodes.push_back(f);
                    const size_t id = s.nodes.size() - 1;
                    s.nodes[parent].kids.push_back(id);
                    s.levels[next] = id;
                    Populate(s, id, deeper_rows, f.file, f.at, next, shadow, depth + 1);
                    continue;
                }
            }
            s.nodes.push_back(f);
            s.nodes[parent].kids.push_back(s.nodes.size() - 1);
        }
    }

    void Index(Snapshot& s, size_t node, const std::string& path)
    {
        s.paths[path.empty() ? "/" : path] = node;
        for (size_t k : s.nodes[node].kids) Index(s, k, path + "/" + s.nodes[k].name);
    }

    bool Pathed(const v31np::Shadow& s)
    {
        for (const auto& part : s.parts)
            for (const auto& r : part.second)
                if (r.kind != 'A' && r.kind != 'D') return true;
        return false;
    }

    std::vector<v31np::Row>* Section(const Snapshot& snap, v31np::Shadow& shadow, const v31np::Chain& chain)
    {
        for (size_t n = 1; n <= chain.size(); n++)
        {
            const v31np::Chain head(chain.begin(), chain.begin() + (std::ptrdiff_t)n);
            if (shadow.parts.count(head)) continue;
            auto it = snap.levels.find(head);
            if (it == snap.levels.end()) return nullptr;
            const Node& box = snap.nodes[it->second];
            const int fd = box.file.empty() ? snap.archive->fd : open(box.file.c_str(), O_RDONLY | O_CLOEXEC);
            if (fd < 0) return nullptr;
            std::vector<v31np::Row> rows;
            const bool ok = Inner(fd, box.at, box.size, rows);
            if (!box.file.empty()) close(fd);
            if (!ok) return nullptr;
            shadow.parts[head] = std::move(rows);
        }
        return &shadow.parts[chain];
    }

    void Assemble(Snapshot& snap)
    {
        snap.nodes.clear();
        snap.paths.clear();
        snap.levels.clear();
        snap.nodes.emplace_back();
        snap.nodes[0].dir = true;
        Populate(snap, 0, snap.shadow.parts[{}], {}, 0, {}, snap.shadow, 0);
        Name(snap, 0);
        Index(snap, 0, "");
    }

    std::shared_ptr<Snapshot> Load(bool keep = false, bool overlay = true)
    {
        auto snap = std::make_shared<Snapshot>();
        snap->archive = std::make_shared<Source>();
        snap->archive->fd = open(mount.archive.c_str(), O_RDONLY | O_CLOEXEC);
        if (snap->archive->fd < 0) return nullptr;
        struct stat st;
        if (fstat(snap->archive->fd, &st) != 0) return nullptr;
        snap->id = Of(st);
        const int fd = snap->archive->fd;
        std::vector<v31np::Entry> entries;
        if (!v31np::Parse([fd](uint64_t pos, void* buf, uint32_t len) { return ReadAll(fd, pos, buf, len); }, (uint64_t)st.st_size, entries)) return nullptr;

        auto found = Scan(snap->id);
        for (const auto& f : found) snap->journals[f.path] = f.id;
        if (!mount.local)
            for (auto& f : found)
                if (f.readable && Pathed(f.shadow)) f.fresh = false;
        snap->editable = mount.road || (mount.local && !Blocked(found));
        const Found* use = Newest(found);

        if (use) snap->shadow = use->shadow;
        if (!use || !snap->shadow.parts.count({}))
        {
            auto& top = snap->shadow.parts[{}];
            top.clear();
            for (const auto& e : entries) top.push_back({ 'A', e.key, e.size, e.at, {}, {}, 0 });
        }
        snap->shadow.version = 2;
        Assemble(*snap);

        if (overlay && mount.road)
        {
            bool any = false;
            for (const auto& [temp, p] : mount.held)
            {
                if (!p.dirty && p.archived) continue;
                std::vector<v31np::Row>* rows = Section(*snap, snap->shadow, p.chain);
                if (!rows) continue;
                struct stat ts;
                const uint64_t size = stat(temp.c_str(), &ts) == 0 ? (uint64_t)ts.st_size : 0;
                change::Put(*rows, { 'T', change::Join(p.key), size, 0, v31np::Wide(temp), mount.plate, 0 });
                any = true;
            }
            if (any) Assemble(*snap);
        }
        if (!mount.local && !keep)
        {
            close(snap->archive->fd);
            snap->archive->fd = -1;
        }
        return snap;
    }

    int Borrow(const Snapshot& s)
    {
        const int fd = open(mount.archive.c_str(), O_RDONLY | O_CLOEXEC);
        if (fd < 0) return -1;
        struct stat st;
        if (fstat(fd, &st) == 0 && Of(st) == s.id) return fd;
        close(fd);
        return -1;
    }

    bool Fresh()
    {
        Identity id;
        if (!mount.now || !Probe(mount.archive, id) || !(id == mount.now->id)) return false;
        for (const auto& p : JournalPaths())
        {
            Identity jid;
            const bool exists = Probe(p, jid);
            auto it = mount.now->journals.find(p);
            if (exists != (it != mount.now->journals.end())) return false;
            if (exists && !(jid == it->second)) return false;
        }
        return true;
    }

    std::shared_ptr<Snapshot> Current()
    {
        if (!Fresh())
        {
            auto next = Load();
            if (next) mount.now = next;
        }
        return mount.now;
    }

    const Node* Find(const Snapshot& s, const char* path)
    {
        auto it = s.paths.find(path);
        return it == s.paths.end() ? nullptr : &s.nodes[it->second];
    }

    class Lock
    {
    public:
        int fd = -1;
        Identity id;

        bool Take()
        {
            for (int tries = 0; tries < 100; tries++)
            {
                fd = open(mount.archive.c_str(), O_RDONLY | O_CLOEXEC);
                if (fd < 0) return false;
                int r;
                do r = flock(fd, LOCK_EX);
                while (r != 0 && errno == EINTR);
                struct stat held, named;
                if (r == 0 && fstat(fd, &held) == 0 && stat(mount.archive.c_str(), &named) == 0 && held.st_dev == named.st_dev && held.st_ino == named.st_ino)
                {
                    id = Of(held);
                    return true;
                }
                close(fd);
                fd = -1;
            }
            return false;
        }

        ~Lock()
        {
            if (fd >= 0) close(fd);
        }
    };

    void Discard(const Found& f, const change::Parts& keep)
    {
        if (f.kind == Kind::Plain)
            for (const auto& part : f.shadow.parts)
                for (const auto& r : part.second)
                    if (r.kind == 'T') Unref(r.file, keep);
        unlink(f.path.c_str());
    }

    bool WriteJournalFile(const std::string& target, const std::string& text)
    {
        const std::string tmp = target + "." + Hex32() + ".tmp";
        int fd = open(tmp.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
        if (fd < 0) return false;
        bool ok = WriteAll(fd, text.data(), text.size());
        ok = fsync(fd) == 0 && ok;
        close(fd);
        if (ok) ok = rename(tmp.c_str(), target.c_str()) == 0;
        if (!ok)
        {
            unlink(tmp.c_str());
            return false;
        }
        v31np::Hide(target);
        SyncFolder(mount.folder);
        return true;
    }

    struct Work
    {
        std::shared_ptr<Snapshot> snap;
        v31np::Shadow* shadow = nullptr;
        std::vector<std::string> made;
    };

    std::vector<v31np::Row>* Level(Work& w, const v31np::Chain& chain)
    {
        return Section(*w.snap, *w.shadow, chain);
    }

    struct Base
    {
        v31np::Shadow shadow;
        std::string target;
        std::string old;
        bool any = false;
    };

    bool Prepare(Lock& lock, Base& base)
    {
        const auto found = Scan(lock.id);
        if (Blocked(found)) return false;
        const Found* use = Newest(found);
        if (use)
        {
            base.shadow = use->shadow;
            if (use->path != JournalPaths()[0]) base.old = use->path;
            base.any = true;
        }
        const change::Parts keep = use ? base.shadow.parts : change::Parts{};
        for (const auto& f : found)
            if (f.readable && !f.fresh) Discard(f, keep);
        base.target = JournalPaths()[0];
        return true;
    }

    struct Plan
    {
        std::string action;
        v31np::Chain chain;
        std::vector<change::Path> keys;
        std::vector<std::string> lines;
    };

    int Car(const Plan& plan, const std::function<int(Work&)>& apply);

    int Change(const Plan& plan, const std::function<int(Work&)>& apply)
    {
        if (mount.road) return Car(plan, apply);
        if (!mount.local) return -EROFS;
        Lock lock;
        if (!lock.Take()) return -EIO;
        Base base;
        if (!Prepare(lock, base)) return -EROFS;
        auto snap = Load();
        if (!snap) return -EIO;
        mount.now = snap;
        if (!snap->editable) return -EROFS;

        v31np::Shadow s = base.shadow;
        if (!base.any || !s.parts.count({}))
        {
            std::vector<v31np::Entry> entries;
            const int fd = lock.fd;
            if (!v31np::Parse([fd](uint64_t pos, void* buf, uint32_t len) { return ReadAll(fd, pos, buf, len); }, (uint64_t)lock.id.size, entries)) return -EIO;
            auto& top = s.parts[{}];
            top.clear();
            for (const auto& e : entries) top.push_back({ 'A', e.key, e.size, e.at, {}, {}, 0 });
        }
        s.version = 2;
        s.size = (uint64_t)lock.id.size;
        s.stamp = FileTime(lock.id.mtime);

        std::vector<std::wstring> before;
        for (const auto& part : s.parts)
            for (const auto& row : part.second)
                if (row.kind == 'T') before.push_back(row.file);
        Work w;
        w.snap = snap;
        w.shadow = &s;
        int r = apply(w);
        if (r == 0) change::Prune(s.parts);
        if (r == 0 && !WriteJournalFile(base.target, v31np::WriteJournal(s))) r = -EIO;
        if (r == 0 && !base.old.empty()) unlink(base.old.c_str());
        if (r != 0)
        {
            for (const auto& t : w.made)
                if (!Busy(t)) unlink(t.c_str());
            return r;
        }
        for (const auto& old : before) Unref(old, s.parts);
        mount.now = Load();
        mount.pending = true;
        return 0;
    }

    void Touch()
    {
        mount.active = std::chrono::steady_clock::now();
        mount.seen = mount.active;
    }

    void Seen()
    {
        mount.seen = std::chrono::steady_clock::now();
    }

    bool Unmount()
    {
        const char* args[] = { "fusermount3", "-u", mount.point.c_str(), nullptr };
        pid_t pid = 0;
        if (posix_spawnp(&pid, "fusermount3", nullptr, nullptr, (char* const*)args, environ) != 0) return false;
        int status = 0;
        while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {}
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }

    int NewTemp(std::string& path, int& fd)
    {
        path = mount.cache + "/" + Hex32() + ".tmp";
        fd = open(path.c_str(), O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
        return fd < 0 ? -errno : 0;
    }

    struct Puller
    {
        int fd = -1;
        uint64_t pos = 0;
        uint64_t end = 0;
        bool own = false;
    };

    enum class Outcome
    {
        Nothing,
        Done,
        Busy,
        Failed
    };

    struct Origin
    {
        int fd = -1;
        uint64_t base = 0;
        uint64_t size = 0;
    };

    Outcome Linked(const v31np::Row& r, int& fd, uint64_t& size)
    {
        fd = open(v31np::Narrow(r.file).c_str(), O_RDONLY | O_CLOEXEC);
        struct stat st;
        if (fd < 0 || fstat(fd, &st) != 0 || !S_ISREG(st.st_mode))
        {
            if (fd >= 0) close(fd);
            fd = -1;
            return Outcome::Failed;
        }
        if (!mount.road)
        {
            if (flock(fd, LOCK_EX | LOCK_NB) != 0 || Busy(v31np::Narrow(r.file)))
            {
                close(fd);
                fd = -1;
                return Outcome::Busy;
            }
            flock(fd, LOCK_UN);
        }
        size = (uint64_t)st.st_size;
        return Outcome::Nothing;
    }

    Outcome Pack(const change::Parts& parts, const v31np::Chain& chain, const Origin& from, int out, uint64_t& total, std::vector<std::string>& temps, const std::function<bool()>* tick = nullptr)
    {
        static const std::vector<v31np::Row> none;
        auto found = parts.find(chain);
        const auto& rows = found == parts.end() ? none : found->second;
        std::vector<std::unique_ptr<Puller>> pullers;
        std::vector<v31np::Item> items;
        auto closeAll = [&]()
        {
            for (auto& p : pullers)
                if (p->own && p->fd >= 0) close(p->fd);
            pullers.clear();
        };
        for (const auto& r : rows)
        {
            if (!v31np::Stored(r)) continue;
            if (r.kind != 'A' && r.kind != 'T')
            {
                closeAll();
                return Outcome::Failed;
            }
            Origin own;
            int opened = -1;
            if (r.kind == 'A')
            {
                if (r.at > from.size || r.size > from.size - r.at)
                {
                    closeAll();
                    return Outcome::Failed;
                }
                own = { from.fd, from.base + r.at, r.size };
            }
            else
            {
                const Outcome o = Linked(r, opened, own.size);
                if (o != Outcome::Nothing)
                {
                    closeAll();
                    return o;
                }
                own.fd = opened;
            }
            auto p = std::make_unique<Puller>();
            v31np::Chain deeper = chain;
            deeper.push_back(r.key);
            if (parts.count(deeper))
            {
                std::string temp;
                int built = -1;
                if (NewTemp(temp, built) != 0)
                {
                    if (opened >= 0) close(opened);
                    closeAll();
                    return Outcome::Failed;
                }
                temps.push_back(temp);
                uint64_t length = 0;
                const Outcome o = Pack(parts, deeper, own, built, length, temps, tick);
                if (opened >= 0) close(opened);
                if (o != Outcome::Done)
                {
                    close(built);
                    closeAll();
                    return o;
                }
                p->fd = built;
                p->own = true;
                p->pos = 0;
                p->end = length;
            }
            else
            {
                p->fd = own.fd;
                p->own = opened >= 0;
                p->pos = own.base;
                p->end = own.base + own.size;
            }
            const uint64_t size = p->end - p->pos;
            Puller* raw = p.get();
            items.push_back({ r.key, size, [raw](void* buf, uint32_t len)
                {
                    if (raw->pos + len > raw->end || !ReadAll(raw->fd, raw->pos, buf, len)) return false;
                    raw->pos += len;
                    return true;
                } });
            pullers.push_back(std::move(p));
        }
        total = 0;
        uint64_t mark = 0;
        const bool ok = v31np::Write(items, [out, &total, &mark, tick](const void* buf, uint32_t len)
            {
                if (!WriteAll(out, buf, len)) return false;
                total += len;
                if (tick && total - mark >= (4u << 20))
                {
                    mark = total;
                    if (!(*tick)()) return false;
                }
                return true;
            });
        closeAll();
        return ok ? Outcome::Done : Outcome::Failed;
    }

    Outcome Repack()
    {
        if (!mount.local) return Outcome::Nothing;
        Lock lock;
        if (!lock.Take()) return Outcome::Failed;
        Base base;
        if (!Prepare(lock, base)) return Outcome::Nothing;
        if (!base.any) return Outcome::Nothing;
        const v31np::Shadow& s = base.shadow;

        struct stat old;
        if (fstat(lock.fd, &old) != 0) return Outcome::Failed;
        const std::string tmp = mount.archive + "." + Hex32() + ".tmp";
        int out = open(tmp.c_str(), O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, old.st_mode & 07777);
        if (out < 0) return Outcome::Failed;
        fchmod(out, old.st_mode & 07777);
        std::vector<std::string> temps;
        uint64_t total = 0;
        const Outcome packed = Pack(s.parts, {}, { lock.fd, 0, (uint64_t)lock.id.size }, out, total, temps);
        for (const auto& t : temps) unlink(t.c_str());
        bool ok = packed == Outcome::Done && fsync(out) == 0;
        if (ok)
        {
            const v31np::ReadAt read = [out](uint64_t pos, void* buf, uint32_t len) { return ReadAll(out, pos, buf, len); };
            std::vector<v31np::Entry> check;
            ok = v31np::Parse(read, total, check) && v31np::Verify(read, total);
        }
        close(out);
        Identity named;
        ok = ok && Probe(mount.archive, named) && named == lock.id;
        ok = ok && rename(tmp.c_str(), mount.archive.c_str()) == 0;
        if (!ok)
        {
            unlink(tmp.c_str());
            return packed == Outcome::Busy ? Outcome::Busy : Outcome::Failed;
        }
        SyncFolder(mount.folder);
        unlink(base.target.c_str());
        if (!base.old.empty()) unlink(base.old.c_str());
        static const change::Parts none;
        for (const auto& part : s.parts)
            for (const auto& r : part.second)
                if (r.kind == 'T') Unref(r.file, none);
        mount.now = Load();
        return Outcome::Done;
    }

    std::string PostPath()
    {
        return mount.folder + "/" + v31np::Narrow(v31np::SignalName(v31np::Wide(mount.stem + ".v31np")));
    }

    bool Entries(int fd, uint64_t base, uint64_t size, const v31np::Chain& chain, size_t depth, std::vector<v31np::Entry>& out)
    {
        std::vector<v31np::Entry> entries;
        auto read = [fd, base, size](uint64_t pos, void* buf, uint32_t len) { return pos + len <= size && ReadAll(fd, base + pos, buf, len); };
        if (!v31np::Parse(read, size, entries)) return false;
        if (depth == chain.size())
        {
            out = std::move(entries);
            return true;
        }
        for (const auto& e : entries)
            if (e.key == chain[depth]) return Entries(fd, base + e.at, e.size, chain, depth + 1, out);
        return false;
    }

    bool Replace(const std::string& tmp)
    {
        for (int tries = 0; tries <= 40; tries++)
        {
            if (rename(tmp.c_str(), mount.archive.c_str()) == 0) return true;
            if (errno != EACCES && errno != EBUSY && errno != ETXTBSY && errno != EPERM) return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        return false;
    }

    std::vector<std::string> Digest(const Snapshot& snap, const v31np::Chain& chain, const change::Path& key)
    {
        std::vector<std::string> out;
        for (const auto& n : snap.nodes)
        {
            if (n.chain != chain || n.stored.size() < key.size() || n.stored.empty()) continue;
            if (!std::equal(key.begin(), key.end(), n.stored.begin())) continue;
            out.push_back(change::Join(n.stored) + (n.dir ? "\t/" : "\t" + std::to_string(n.size)));
        }
        std::sort(out.begin(), out.end());
        return out;
    }

    bool Ask(const v31np::Papers& holder)
    {
        const std::string title = mount.stem + ".v31np";
        const std::string text = "Still waiting for " + holder.plate + " (" + holder.os + "), which holds this archive. Go ahead anyway?";
        const std::string t1 = "--title=" + title;
        const std::string t2 = "--text=" + text;
        const char* args[] = { "zenity", "--question", "--no-markup", "--timeout=60", t1.c_str(), t2.c_str(), nullptr };
        pid_t pid;
        if (posix_spawnp(&pid, "zenity", nullptr, nullptr, (char* const*)args, environ) != 0) return false;
        int status = 0;
        while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {}
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }

    class MountRoad : public v31np::Road
    {
    public:
        MountRoad() : signal(v31np::Wide(PostPath())) {}
        bool Green(uint32_t waitMs) override { return signal.Green(waitMs); }
        void Red() override { signal.Red(); }
        bool ShowPapers(const std::string& papers) override { return signal.ShowPapers(papers); }
        bool ServerNow(int64_t& now) override { return signal.ServerNow(now); }
        int64_t Clock() override { return (int64_t)time(nullptr); }
        uint64_t Ticks() override
        {
            return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        }
        void Sleep(uint32_t ms) override { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
        bool Archive(uint64_t& size, uint64_t& stamp) override
        {
            Identity id;
            if (!Probe(mount.archive, id)) return false;
            size = (uint64_t)id.size;
            stamp = FileTime(id.mtime);
            return true;
        }
        bool ReadJournal(std::string& text, bool& exists) override
        {
            const std::string path = JournalPaths()[0];
            Identity id;
            exists = Probe(path, id);
            if (!exists) return errno == ENOENT;
            text = Slurp(path, id);
            return true;
        }
        bool WriteJournal(const std::string& text) override
        {
            const std::string path = JournalPaths()[0];
            for (int tries = 0; tries < 40; tries++)
            {
                if (WriteJournalFile(path, text)) return true;
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
            return false;
        }
        bool Ask(const v31np::Papers& holder) override { return ::Ask(holder); }
        bool Cancelled() override { return mount.stop; }

    private:
        v31np::Signal signal;
    };

    class MountTrip : public v31np::Trip
    {
    public:
        MountTrip(const Plan& plan, const std::function<int(Work&)>& apply, std::shared_ptr<Snapshot> base) : plan(plan), apply(apply), base(std::move(base)) {}
        ~MountTrip() override
        {
            if (out >= 0) close(out);
        }

        int error = 0;

        bool Look(const v31np::Shadow&) override
        {
            snap = Load(true, false);
            if (!snap)
            {
                error = -EIO;
                return false;
            }
            if (!base) return true;
            for (const auto& k : plan.keys)
                if (Digest(*base, plan.chain, k) != Digest(*snap, plan.chain, k))
                {
                    error = -ESTALE;
                    return false;
                }
            return true;
        }

        bool Build(const std::vector<v31np::Record>& records, const std::function<bool()>& renew) override
        {
            shadow = snap->shadow;
            for (const auto& part : shadow.parts)
                for (const auto& r : part.second)
                    if (r.kind == 'D') had.insert({ part.first, r.key });
            w.snap = snap;
            w.shadow = &shadow;
            if (int r = apply(w))
            {
                error = r;
                return false;
            }
            change::Prune(shadow.parts);
            std::vector<v31np::Row>* rows = Level(w, plan.chain);
            if (!rows)
            {
                error = -EIO;
                return false;
            }
            std::vector<std::string> keys;
            for (const auto& r : *rows)
                if (v31np::Stored(r)) keys.push_back(r.key);
            for (const auto& [key, text] : v31np::CustodyFiles(keys, records))
            {
                std::string temp;
                int fd = -1;
                if (NewTemp(temp, fd) != 0)
                {
                    error = -EIO;
                    return false;
                }
                custody.push_back(temp);
                const bool ok = WriteAll(fd, text.data(), text.size());
                close(fd);
                if (!ok)
                {
                    error = -EIO;
                    return false;
                }
                change::Put(*rows, { 'T', key, (uint64_t)text.size(), 0, v31np::Wide(temp), mount.plate, 0 });
            }
            tmp = mount.archive + "." + Hex32() + ".tmp";
            out = open(tmp.c_str(), O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
            if (out < 0)
            {
                tmp.clear();
                error = -EIO;
                return false;
            }
            std::vector<std::string> temps;
            const Outcome packed = Pack(shadow.parts, {}, { snap->archive->fd, 0, (uint64_t)snap->id.size }, out, total, temps, &renew);
            for (const auto& t : temps) unlink(t.c_str());
            bool ok = packed == Outcome::Done && fsync(out) == 0;
            if (ok)
            {
                const int fd = out;
                const v31np::ReadAt read = [fd](uint64_t pos, void* buf, uint32_t len) { return ReadAll(fd, pos, buf, len); };
                std::vector<v31np::Entry> check;
                ok = v31np::Parse(read, total, check) && v31np::Verify(read, total);
            }
            if (!ok) error = -EIO;
            return ok;
        }

        bool Land(v31np::Shadow& next, int64_t) override
        {
            Identity named;
            if (!Probe(mount.archive, named) || !(named == snap->id))
            {
                error = -ESTALE;
                return false;
            }
            snap->archive.reset();
            if (!Replace(tmp))
            {
                error = -EIO;
                return false;
            }
            tmp.clear();
            close(out);
            out = -1;
            std::map<std::pair<v31np::Chain, std::string>, std::pair<std::string, int64_t>> leases;
            for (const auto& part : next.parts)
                for (const auto& r : part.second)
                    if (r.kind == 'D') leases[{ part.first, r.key }] = { r.plate, r.until };
            next.parts.clear();
            const int fd = open(mount.archive.c_str(), O_RDONLY | O_CLOEXEC);
            struct stat now;
            if (fd >= 0 && fstat(fd, &now) == 0)
            {
                next.size = (uint64_t)now.st_size;
                next.stamp = FileTime(now.st_mtim);
                for (const auto& part : shadow.parts)
                {
                    std::vector<std::string> dirs;
                    for (const auto& row : part.second)
                    {
                        if (row.kind != 'D') continue;
                        const bool kept = leases.count({ part.first, row.key }) != 0;
                        const bool made = had.count({ part.first, row.key }) == 0;
                        if (kept || made) dirs.push_back(row.key);
                    }
                    if (dirs.empty()) continue;
                    std::vector<v31np::Entry> level;
                    if (!Entries(fd, 0, (uint64_t)now.st_size, part.first, 0, level)) continue;
                    auto rows = v31np::Listing(level, dirs);
                    bool any = false;
                    for (auto& r : rows)
                    {
                        if (r.kind != 'D') continue;
                        any = true;
                        auto it = leases.find({ part.first, r.key });
                        if (it == leases.end()) continue;
                        r.plate = it->second.first;
                        r.until = it->second.second;
                    }
                    if (any) next.parts[part.first] = std::move(rows);
                }
            }
            if (fd >= 0) close(fd);
            Tidy();
            return true;
        }

        void Abandon() override
        {
            if (out >= 0)
            {
                close(out);
                out = -1;
            }
            if (!tmp.empty()) unlink(tmp.c_str());
            tmp.clear();
            for (const auto& t : custody) unlink(t.c_str());
            custody.clear();
            for (const auto& t : w.made)
                if (!Busy(t) && !mount.held.count(t)) unlink(t.c_str());
        }

    private:
        void Tidy()
        {
            for (const auto& t : custody) unlink(t.c_str());
            custody.clear();
            for (const auto& part : shadow.parts)
                for (const auto& row : part.second)
                    if (row.kind == 'T')
                    {
                        const std::string t = v31np::Narrow(row.file);
                        if (Own(row.file) && !Busy(t) && !mount.held.count(t)) unlink(t.c_str());
                    }
            for (const auto& t : w.made)
                if (!Busy(t) && !mount.held.count(t)) unlink(t.c_str());
        }

        Plan plan;
        std::function<int(Work&)> apply;
        std::shared_ptr<Snapshot> base;
        std::shared_ptr<Snapshot> snap;
        v31np::Shadow shadow;
        Work w;
        std::set<std::pair<v31np::Chain, std::string>> had;
        std::vector<std::string> custody;
        std::string tmp;
        int out = -1;
        uint64_t total = 0;
    };

    bool Taken(const Snapshot* s, const v31np::Chain& chain, const change::Path& key)
    {
        if (!s) return false;
        for (const auto& n : s->nodes)
            if (!n.dir && n.chain == chain && n.stored == key) return true;
        return false;
    }

    std::vector<v31np::Step> Steps(const Plan& plan, const Snapshot* base)
    {
        const bool add = plan.action == "add" && plan.keys.size() == 1 && plan.lines.size() == 1;
        const bool moved = (plan.action == "rename" || plan.action == "move") && plan.keys.size() == 2 && plan.lines.size() == 2;
        if (add && Taken(base, plan.chain, plan.keys[0])) return { { "delete", { plan.lines[0] } }, { "add", { plan.lines[0] } } };
        if (moved && plan.keys[0] != plan.keys[1] && Taken(base, plan.chain, plan.keys[1])) return { { "delete", { plan.lines[1] } }, { plan.action, plan.lines } };
        return { { plan.action, plan.lines } };
    }

    int Car(const Plan& plan, const std::function<int(Work&)>& apply)
    {
        MountRoad road;
        auto base = Load(false, false);
        MountTrip trip(plan, apply, base);
        v31np::Request request;
        request.me = { mount.plate, mount.system };
        request.steps = Steps(plan, base.get());
        for (const auto& k : plan.keys)
        {
            change::Path full(plan.chain.begin(), plan.chain.end());
            full.insert(full.end(), k.begin(), k.end());
            request.keys.push_back(change::Join(full));
        }
        const v31np::Drove d = v31np::Drive(road, trip, request);
        mount.now = Load();
        switch (d)
        {
        case v31np::Drove::Done: return 0;
        case v31np::Drove::Refused: return trip.error ? trip.error : -ESTALE;
        case v31np::Drove::Failed: return trip.error ? trip.error : -EIO;
        case v31np::Drove::Lost: return -EAGAIN;
        case v31np::Drove::Skewed: return -ETIMEDOUT;
        case v31np::Drove::Cancelled: return -EINTR;
        }
        return -EIO;
    }

    bool Leased()
    {
        auto s = Current();
        if (!s) return false;
        for (const auto& part : s->shadow.parts)
            for (const auto& r : part.second)
                if (r.kind == 'D' && r.plate == mount.plate) return true;
        return false;
    }

    void Idle()
    {
        while (!mount.stop)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            bool leave = false;
            {
                std::lock_guard<std::recursive_mutex> g(mount.guard);
                const auto now = std::chrono::steady_clock::now();
                if (mount.local && mount.pending && mount.open.empty() && now - mount.active >= Wait)
                {
                    const Outcome o = Repack();
                    if (o == Outcome::Busy) Touch();
                    else mount.pending = false;
                }
                if (mount.road && now - mount.renewed >= std::chrono::seconds(v31np::Renewal))
                {
                    mount.renewed = now;
                    if (Leased()) v31np::Visit(*std::make_unique<MountRoad>(), { mount.plate, mount.system }, [](v31np::Shadow& s, int64_t at) { v31np::RenewFolders(s, mount.plate, at, v31np::Lease); });
                }
                leave = mount.leave.count() > 0 && mount.open.empty() && mount.held.empty() && !mount.pending && now - mount.seen >= mount.leave;
            }
            if (leave && !Unmount())
            {
                std::lock_guard<std::recursive_mutex> g(mount.guard);
                Seen();
            }
            if (leave) std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void Fill(const Snapshot& s, const Node& n, struct stat* st)
    {
        std::memset(st, 0, sizeof *st);
        st->st_uid = getuid();
        st->st_gid = getgid();
        st->st_mtim = s.id.mtime;
        st->st_ctim = s.id.mtime;
        st->st_atim = s.id.mtime;
        const bool w = s.editable;
        if (n.dir)
        {
            st->st_mode = S_IFDIR | (w ? 0755 : 0555);
            st->st_nlink = 2;
        }
        else
        {
            st->st_mode = S_IFREG | (w ? 0644 : 0444);
            st->st_nlink = 1;
            st->st_size = (off_t)n.size;
            if (n.kind != 'A' && !n.file.empty())
            {
                struct stat live;
                if (stat(n.file.c_str(), &live) == 0) st->st_size = live.st_size;
            }
        }
    }

    std::string Parent(const std::string& path, std::string& name)
    {
        const size_t slash = path.rfind('/');
        name = path.substr(slash + 1);
        return slash == 0 ? "/" : path.substr(0, slash);
    }

    int NewKey(const Snapshot& s, const char* path, v31np::Chain& chain, change::Path& key)
    {
        std::string name;
        const std::string parent = Parent(path, name);
        const Node* dir = Find(s, parent.c_str());
        if (!dir) return -ENOENT;
        if (!dir->dir) return -ENOTDIR;
        if (name.empty() || !v31np::ValidUtf8(name)) return -EILSEQ;
        chain = dir->chain;
        if (dir->nested)
        {
            chain.push_back(change::Join(dir->stored));
            key.clear();
        }
        else key = dir->stored;
        key.push_back(name);
        return 0;
    }

    Plan Planned(const std::string& action, const char* path, const char* to = nullptr)
    {
        Plan p;
        p.action = action;
        if (!mount.road) return p;
        auto s = Current();
        if (!s) return p;
        auto one = [&](const char* x, v31np::Chain& chain, change::Path& key)
        {
            if (const Node* n = Find(*s, x))
            {
                chain = n->chain;
                key = n->stored;
                return !key.empty();
            }
            return NewKey(*s, x, chain, key) == 0;
        };
        v31np::Chain c;
        change::Path k;
        if (!one(path, c, k)) return p;
        p.chain = c;
        p.keys.push_back(k);
        p.lines.push_back(change::Join(k));
        if (!to) return p;
        v31np::Chain c2;
        change::Path k2;
        if (!one(to, c2, k2) || c2 != c) return p;
        p.keys.push_back(k2);
        p.lines.push_back(change::Join(k2));
        if (action == "rename" && !std::equal(k.begin(), k.end() - 1, k2.begin(), k2.end() - 1)) p.action = "move";
        return p;
    }

    int Editable(const Snapshot& s, const Node* n)
    {
        if (!n) return -ENOENT;
        if (!s.editable) return -EROFS;
        return 0;
    }

    bool Room(uint64_t need)
    {
        struct statvfs v;
        if (statvfs(mount.cache.c_str(), &v) != 0) return false;
        return (uint64_t)v.f_bavail * (uint64_t)v.f_frsize > need + (16u << 20);
    }

    int CopyInto(const Snapshot& s, const Node& n, int out)
    {
        const bool shared = n.file.empty() && s.archive->fd >= 0;
        int in = shared ? s.archive->fd : n.file.empty() ? Borrow(s) : open(n.file.c_str(), O_RDONLY | O_CLOEXEC);
        if (in < 0) return -EIO;
        uint64_t size = n.size;
        if (n.kind != 'A')
        {
            struct stat st;
            if (fstat(in, &st) == 0) size = (uint64_t)st.st_size;
        }
        if (!Room(size))
        {
            if (!shared) close(in);
            return -ENOSPC;
        }
        std::vector<char> buf(1u << 20);
        uint64_t done = 0;
        int r = 0;
        while (done < size)
        {
            const uint32_t len = (uint32_t)std::min<uint64_t>(buf.size(), size - done);
            if (!ReadAll(in, n.at + done, buf.data(), len) || !WriteAll(out, buf.data(), len))
            {
                r = -EIO;
                break;
            }
            done += len;
        }
        if (!shared) close(in);
        return r;
    }

    v31np::Row* RowOf(change::Parts& parts, const std::string& temp)
    {
        const std::wstring w = v31np::Wide(temp);
        for (auto& part : parts)
            for (auto& r : part.second)
                if (r.kind == 'T' && r.file == w) return &r;
        return nullptr;
    }

    int Writable(Work& w, const Node& n, bool truncate, std::string& temp, int& fd)
    {
        std::vector<v31np::Row>* rows = Level(w, n.chain);
        if (!rows) return -EIO;
        if (n.kind == 'T' && Own(v31np::Wide(n.file)))
        {
            size_t refs = 0;
            for (const auto& part : w.shadow->parts)
                for (const auto& r : part.second)
                    if (r.file == v31np::Wide(n.file)) refs++;
            if (refs == 1)
            {
                temp = n.file;
                fd = open(temp.c_str(), O_RDWR | O_CLOEXEC);
                if (fd < 0) return -errno;
                if (truncate && ftruncate(fd, 0) != 0)
                {
                    close(fd);
                    fd = -1;
                    return -EIO;
                }
                v31np::Row* row = RowOf(w.shadow->parts, temp);
                if (row && truncate) row->size = 0;
                return 0;
            }
        }
        if (int r = NewTemp(temp, fd)) return r;
        w.made.push_back(temp);
        if (!truncate)
            if (int r = CopyInto(*w.snap, n, fd))
            {
                close(fd);
                fd = -1;
                return r;
            }
        struct stat st;
        const uint64_t size = fstat(fd, &st) == 0 ? (uint64_t)st.st_size : 0;
        change::Put(*rows, { 'T', change::Join(n.stored), size, 0, v31np::Wide(temp), {}, 0 });
        return 0;
    }

    Handle* Opened(const std::shared_ptr<Snapshot>& snap, int fd, const std::string& temp)
    {
        flock(fd, LOCK_SH);
        auto h = new Handle;
        h->snap = snap;
        h->fd = fd;
        h->write = true;
        h->temp = temp;
        mount.open.insert(h);
        return h;
    }

    void Settle(const std::string& temp)
    {
        Change({}, [&](Work& w)
            {
                v31np::Row* row = RowOf(w.shadow->parts, temp);
                if (!row) return -ENOENT;
                struct stat st;
                if (stat(temp.c_str(), &st) != 0) return -EIO;
                if (row->size == (uint64_t)st.st_size) return -EALREADY;
                row->size = (uint64_t)st.st_size;
                return 0;
            });
    }

    int Land(const std::string& temp)
    {
        auto it = mount.held.find(temp);
        if (it == mount.held.end() || !it->second.dirty) return 0;
        const Pending p = it->second;
        const int r = Car({ "add", p.chain, { p.key }, { change::Join(p.key) } }, [&](Work& w)
            {
                std::vector<v31np::Row>* rows = Level(w, p.chain);
                if (!rows) return -ENOENT;
                if (p.key.size() > 1)
                {
                    const change::Path parent(p.key.begin(), p.key.end() - 1);
                    bool there = false;
                    for (const auto& n : w.snap->nodes)
                        if (n.dir && n.chain == p.chain && n.stored == parent) there = true;
                    if (!there) return -ENOENT;
                }
                struct stat ts;
                if (stat(temp.c_str(), &ts) != 0) return -EIO;
                change::Put(*rows, { 'T', change::Join(p.key), (uint64_t)ts.st_size, 0, v31np::Wide(temp), mount.plate, 0 });
                return 0;
            });
        if (r != 0) return r;
        auto again = mount.held.find(temp);
        if (again != mount.held.end())
        {
            again->second.dirty = false;
            again->second.written = false;
            again->second.archived = true;
        }
        mount.now = Load();
        return 0;
    }

    void Dirty(const std::string& temp, bool wrote = false)
    {
        auto it = mount.held.find(temp);
        if (it == mount.held.end()) return;
        if (wrote) it->second.written = true;
        if (it->second.dirty) return;
        it->second.dirty = true;
        mount.now = Load();
    }

    void Follow(const v31np::Chain& chain, const change::Path& from, const change::Path* to)
    {
        for (auto it = mount.held.begin(); it != mount.held.end();)
        {
            Pending& p = it->second;
            if (p.chain == chain && change::Within(change::Join(p.key), from))
            {
                if (!to)
                {
                    it = mount.held.erase(it);
                    continue;
                }
                change::Path moved = *to;
                moved.insert(moved.end(), p.key.begin() + (std::ptrdiff_t)from.size(), p.key.end());
                p.key = moved;
            }
            ++it;
        }
    }

    const Pending* Unlanded(const Node* n)
    {
        if (!n || n->dir || n->kind != 'T') return nullptr;
        auto it = mount.held.find(n->file);
        return it != mount.held.end() && !it->second.archived ? &it->second : nullptr;
    }

    int RoadCreate(const char* path, struct fuse_file_info* fi)
    {
        auto s = Current();
        if (!s) return -EIO;
        if (Find(*s, path)) return -EEXIST;
        v31np::Chain chain;
        change::Path key;
        if (int e = NewKey(*s, path, chain, key)) return e;
        std::string temp;
        int fd = -1;
        if (int e = NewTemp(temp, fd)) return e;
        mount.held[temp] = { chain, key, true, false };
        fi->fh = (uint64_t)(uintptr_t)Opened(s, fd, temp);
        fi->direct_io = 1;
        mount.now = Load();
        return 0;
    }

    int RoadOpen(const char* path, struct fuse_file_info* fi)
    {
        auto s = Current();
        if (!s) return -EIO;
        const Node* n = Find(*s, path);
        if (int e = Editable(*s, n)) return e;
        if (n->dir) return -EISDIR;
        const bool truncate = (fi->flags & O_TRUNC) != 0;
        std::string temp;
        int fd = -1;
        if (n->kind == 'T' && mount.held.count(n->file))
        {
            temp = n->file;
            fd = open(temp.c_str(), O_RDWR | O_CLOEXEC);
            if (fd < 0) return -errno;
            if (truncate && ftruncate(fd, 0) != 0)
            {
                close(fd);
                return -EIO;
            }
        }
        else
        {
            if (int e = NewTemp(temp, fd)) return e;
            if (!truncate)
                if (int e = CopyInto(*s, *n, fd))
                {
                    close(fd);
                    unlink(temp.c_str());
                    return e;
                }
            mount.held[temp] = { n->chain, n->stored, false, true };
        }
        fi->fh = (uint64_t)(uintptr_t)Opened(s, fd, temp);
        fi->direct_io = 1;
        if (truncate) Dirty(temp);
        return 0;
    }

    void RoadRelease(const std::string& temp)
    {
        if (Busy(temp)) return;
        if (int r = Land(temp)) fprintf(stderr, "v31np-mount: %s could not land at close: %s\n", temp.c_str(), strerror(-r));
        mount.held.erase(temp);
        if (Own(v31np::Wide(temp))) unlink(temp.c_str());
        mount.now = Load();
    }

    int GetAttr(const char* path, struct stat* st, struct fuse_file_info* fi)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Seen();
        if (fi && fi->fh)
        {
            auto h = (Handle*)(uintptr_t)fi->fh;
            if (h->write)
            {
                struct stat live;
                if (fstat(h->fd, &live) != 0) return -EIO;
                std::memset(st, 0, sizeof *st);
                st->st_uid = getuid();
                st->st_gid = getgid();
                st->st_mode = S_IFREG | 0644;
                st->st_nlink = 1;
                st->st_size = live.st_size;
                st->st_mtim = st->st_ctim = st->st_atim = h->snap->id.mtime;
                return 0;
            }
        }
        auto s = Current();
        if (!s) return -EIO;
        const Node* n = Find(*s, path);
        if (!n) return -ENOENT;
        Fill(*s, *n, st);
        return 0;
    }

    int ReadDir(const char* path, void* buf, fuse_fill_dir_t fill, off_t, struct fuse_file_info*, enum fuse_readdir_flags)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Seen();
        auto s = Current();
        if (!s) return -EIO;
        const Node* n = Find(*s, path);
        if (!n) return -ENOENT;
        if (!n->dir) return -ENOTDIR;
        fill(buf, ".", nullptr, 0, (enum fuse_fill_dir_flags)0);
        fill(buf, "..", nullptr, 0, (enum fuse_fill_dir_flags)0);
        for (size_t k : n->kids)
        {
            struct stat st;
            Fill(*s, s->nodes[k], &st);
            if (fill(buf, s->nodes[k].name.c_str(), &st, 0, (enum fuse_fill_dir_flags)0)) break;
        }
        return 0;
    }

    int Open(const char* path, struct fuse_file_info* fi)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Touch();
        const bool wants = (fi->flags & O_ACCMODE) != O_RDONLY || (fi->flags & O_TRUNC);
        if (wants && mount.road) return RoadOpen(path, fi);
        if (wants)
        {
            std::shared_ptr<Snapshot> snap;
            std::string temp;
            int fd = -1;
            const int r = Change({}, [&](Work& w)
                {
                    const Node* n = Find(*w.snap, path);
                    if (int e = Editable(*w.snap, n)) return e;
                    if (n->dir) return -EISDIR;
                    snap = w.snap;
                    return Writable(w, *n, (fi->flags & O_TRUNC) != 0, temp, fd);
                });
            if (r != 0)
            {
                if (fd >= 0) close(fd);
                return r;
            }
            fi->fh = (uint64_t)(uintptr_t)Opened(snap, fd, temp);
            fi->direct_io = 1;
            return 0;
        }
        auto s = Current();
        if (!s) return -EIO;
        const Node* n = Find(*s, path);
        if (!n) return -ENOENT;
        if (n->dir) return -EISDIR;
        auto h = new Handle;
        h->snap = s;
        if (n->kind == 'A')
        {
            h->archive = true;
            h->at = n->at;
            h->size = n->size;
            if (!n->file.empty())
            {
                h->fd = open(n->file.c_str(), O_RDONLY | O_CLOEXEC);
                if (h->fd < 0)
                {
                    delete h;
                    return -EIO;
                }
            }
        }
        else
        {
            h->fd = open(n->file.c_str(), O_RDONLY | O_CLOEXEC);
            if (h->fd < 0)
            {
                delete h;
                return -EIO;
            }
        }
        mount.open.insert(h);
        fi->fh = (uint64_t)(uintptr_t)h;
        fi->keep_cache = h->archive ? 1 : 0;
        return 0;
    }

    int Create(const char* path, mode_t, struct fuse_file_info* fi)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Touch();
        if (mount.road) return RoadCreate(path, fi);
        std::shared_ptr<Snapshot> snap;
        std::string temp;
        int fd = -1;
        const int r = Change({}, [&](Work& w)
            {
                if (!w.snap->editable) return -EROFS;
                if (Find(*w.snap, path)) return -EEXIST;
                v31np::Chain chain;
                change::Path key;
                if (int e = NewKey(*w.snap, path, chain, key)) return e;
                std::vector<v31np::Row>* rows = Level(w, chain);
                if (!rows) return -EIO;
                if (int e = NewTemp(temp, fd)) return e;
                w.made.push_back(temp);
                change::Put(*rows, { 'T', change::Join(key), 0, 0, v31np::Wide(temp), {}, 0 });
                snap = w.snap;
                return 0;
            });
        if (r != 0)
        {
            if (fd >= 0) close(fd);
            return r;
        }
        fi->fh = (uint64_t)(uintptr_t)Opened(snap, fd, temp);
        fi->direct_io = 1;
        return 0;
    }

    int Read(const char*, char* buf, size_t size, off_t off, struct fuse_file_info* fi)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Seen();
        auto h = (Handle*)(uintptr_t)fi->fh;
        int fd = h->fd;
        uint64_t base = 0;
        size_t len = size;
        if (h->archive)
        {
            if ((uint64_t)off >= h->size) return 0;
            len = (size_t)std::min<uint64_t>(size, h->size - (uint64_t)off);
            fd = h->fd >= 0 ? h->fd : h->snap->archive->fd;
            base = h->at;
        }
        int borrowed = -1;
        if (fd < 0)
        {
            borrowed = Borrow(*h->snap);
            if (borrowed < 0) return -ESTALE;
            fd = borrowed;
        }
        size_t done = 0;
        int failed = 0;
        while (done < len)
        {
            ssize_t r = pread(fd, buf + done, len - done, (off_t)(base + (uint64_t)off + done));
            if (r < 0 && errno == EINTR) continue;
            if (r < 0)
            {
                failed = -errno;
                break;
            }
            if (r == 0) break;
            done += (size_t)r;
        }
        if (borrowed >= 0) close(borrowed);
        return failed ? failed : (int)done;
    }

    int Write(const char*, const char* buf, size_t size, off_t off, struct fuse_file_info* fi)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Touch();
        auto h = (Handle*)(uintptr_t)fi->fh;
        if (!h->write) return -EBADF;
        size_t done = 0;
        while (done < size)
        {
            ssize_t r = pwrite(h->fd, buf + done, size - done, off + (off_t)done);
            if (r < 0 && errno == EINTR) continue;
            if (r < 0) return -errno;
            done += (size_t)r;
        }
        if (mount.road) Dirty(h->temp, true);
        return (int)done;
    }

    int Truncate(const char* path, off_t size, struct fuse_file_info* fi)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Touch();
        if (fi && fi->fh)
        {
            auto h = (Handle*)(uintptr_t)fi->fh;
            if (!h->write) return -EBADF;
            if (ftruncate(h->fd, size) != 0) return -errno;
            if (mount.road) Dirty(h->temp, true);
            return 0;
        }
        return Change(Planned("add", path), [&](Work& w)
            {
                const Node* n = Find(*w.snap, path);
                if (int e = Editable(*w.snap, n)) return e;
                if (n->dir) return -EISDIR;
                std::string temp;
                int fd = -1;
                if (int e = Writable(w, *n, size == 0, temp, fd)) return e;
                const bool ok = ftruncate(fd, size) == 0;
                close(fd);
                if (!ok) return -EIO;
                if (v31np::Row* row = RowOf(w.shadow->parts, temp)) row->size = (uint64_t)size;
                return 0;
            });
    }

    int Flush(const char*, struct fuse_file_info* fi)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        auto h = (Handle*)(uintptr_t)fi->fh;
        if (h->write && mount.road)
        {
            auto it = mount.held.find(h->temp);
            return it != mount.held.end() && it->second.written ? Land(h->temp) : 0;
        }
        if (h->write) Settle(h->temp);
        return 0;
    }

    int Release(const char*, struct fuse_file_info* fi)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Touch();
        auto h = (Handle*)(uintptr_t)fi->fh;
        mount.open.erase(h);
        const std::string temp = h->write ? h->temp : std::string();
        if (h->fd >= 0) close(h->fd);
        delete h;
        if (!temp.empty() && mount.road)
        {
            RoadRelease(temp);
            return 0;
        }
        if (!temp.empty())
        {
            Settle(temp);
            auto s = Current();
            Lock lock;
            if (lock.Take())
            {
                const auto found = Scan(lock.id);
                const Found* use = Newest(found);
                const change::Parts parts = use ? use->shadow.parts : change::Parts{};
                if (use || !Blocked(found)) Unref(v31np::Wide(temp), parts);
            }
        }
        return 0;
    }

    int MakeDir(const char* path, mode_t)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Touch();
        return Change(Planned("add", path), [&](Work& w)
            {
                if (Find(*w.snap, path)) return -EEXIST;
                v31np::Chain chain;
                change::Path key;
                if (int e = NewKey(*w.snap, path, chain, key)) return e;
                std::vector<v31np::Row>* rows = Level(w, chain);
                if (!rows) return -EIO;
                change::MakeDir(*rows, key);
                return 0;
            });
    }

    int Unlink(const char* path)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Touch();
        v31np::Chain chain;
        change::Path stored;
        if (mount.road)
        {
            auto s = Current();
            if (!s) return -EIO;
            const Node* n = Find(*s, path);
            if (Unlanded(n))
            {
                const std::string temp = n->file;
                mount.held.erase(temp);
                if (!Busy(temp) && Own(v31np::Wide(temp))) unlink(temp.c_str());
                mount.now = Load();
                return 0;
            }
            if (n)
            {
                chain = n->chain;
                stored = n->stored;
            }
        }
        const int r = Change(Planned("delete", path), [&](Work& w)
            {
                const Node* n = Find(*w.snap, path);
                if (int e = Editable(*w.snap, n)) return e;
                if (n->dir) return -EISDIR;
                std::vector<v31np::Row>* rows = Level(w, n->chain);
                if (!rows) return -EIO;
                change::Remove(*rows, n->stored);
                return 0;
            });
        if (r == 0 && mount.road && !stored.empty()) Follow(chain, stored, nullptr);
        return r;
    }

    int RemoveDir(const char* path)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Touch();
        return Change(Planned("delete", path), [&](Work& w)
            {
                const Node* n = Find(*w.snap, path);
                if (int e = Editable(*w.snap, n)) return e;
                if (!n->dir) return -ENOTDIR;
                if (n->stored.empty()) return -EBUSY;
                if (!n->kids.empty()) return -ENOTEMPTY;
                std::vector<v31np::Row>* rows = Level(w, n->chain);
                if (!rows) return -EIO;
                change::Remove(*rows, n->stored);
                return 0;
            });
    }

    int Rename(const char* from, const char* to, unsigned int flags)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        Touch();
        if (flags & RENAME_EXCHANGE) return -EINVAL;
        v31np::Chain fromChain;
        change::Path fromKey;
        change::Path toKey;
        if (mount.road)
        {
            auto s = Current();
            if (!s) return -EIO;
            const Node* src = Find(*s, from);
            v31np::Chain chain;
            change::Path key;
            if (src && NewKey(*s, to, chain, key) == 0)
            {
                if (const Pending* p = Unlanded(src))
                {
                    if (chain != p->chain) return -EXDEV;
                    const Node* dst = Find(*s, to);
                    if (dst && (flags & RENAME_NOREPLACE)) return -EEXIST;
                    if (dst && dst->dir) return -EISDIR;
                    const std::string temp = src->file;
                    Pending& q = mount.held[temp];
                    q.key = key;
                    q.dirty = true;
                    return Land(temp);
                }
                fromChain = src->chain;
                fromKey = src->stored;
                toKey = key;
            }
        }
        const int r = Change(Planned("rename", from, to), [&](Work& w)
            {
                const Node* src = Find(*w.snap, from);
                if (int e = Editable(*w.snap, src)) return e;
                if (src->stored.empty()) return -EBUSY;
                v31np::Chain chain;
                change::Path key;
                if (int e = NewKey(*w.snap, to, chain, key)) return e;
                if (chain != src->chain) return -EXDEV;
                const std::string f = from, t = to;
                if (t.size() > f.size() && t.compare(0, f.size(), f) == 0 && t[f.size()] == '/') return -EINVAL;
                const Node* dst = Find(*w.snap, to);
                if (dst == src) return 0;
                if (src->nested && !Nested(key.back()))
                {
                    v31np::Chain inside = src->chain;
                    inside.push_back(change::Join(src->stored));
                    if (w.shadow->parts.count(inside)) return -EBUSY;
                }
                std::vector<v31np::Row>* rows = Level(w, chain);
                if (!rows) return -EIO;
                if (dst)
                {
                    if (flags & RENAME_NOREPLACE) return -EEXIST;
                    if (src->dir && !dst->dir) return -ENOTDIR;
                    if (!src->dir && dst->dir) return -EISDIR;
                    if (dst->dir && !dst->kids.empty()) return -ENOTEMPTY;
                    change::Remove(*rows, dst->stored);
                    change::Prune(w.shadow->parts);
                }
                change::Move(*rows, src->stored, key);
                change::Rekey(w.shadow->parts, chain, src->stored, key);
                return 0;
            });
        if (r == 0 && mount.road && !fromKey.empty()) Follow(fromChain, fromKey, &toKey);
        return r;
    }

    int Ignore(const char* path)
    {
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        auto s = Current();
        if (!s) return -EIO;
        const Node* n = Find(*s, path);
        if (!n) return -ENOENT;
        if (!s->editable) return -EROFS;
        return 0;
    }

    int ChangeMode(const char* path, mode_t, struct fuse_file_info*)
    {
        return Ignore(path);
    }

    int ChangeOwner(const char* path, uid_t, gid_t, struct fuse_file_info*)
    {
        return Ignore(path);
    }

    int Times(const char* path, const struct timespec*, struct fuse_file_info*)
    {
        return Ignore(path);
    }

    int Refuse(const char*, const char*)
    {
        return -EPERM;
    }

    int RefuseNode(const char*, mode_t, dev_t)
    {
        return -EPERM;
    }

    int Space(const char*, struct statvfs* st)
    {
        return statvfs(mount.cache.c_str(), st) == 0 ? 0 : -errno;
    }

    void* Init(struct fuse_conn_info*, struct fuse_config* cfg)
    {
        cfg->kernel_cache = 0;
        cfg->attr_timeout = 0.0;
        cfg->entry_timeout = 0.0;
        cfg->negative_timeout = 0.0;
        cfg->hard_remove = 1;
        if (mount.local || mount.leave.count() > 0) mount.idle = std::thread(Idle);
        return nullptr;
    }

    void Destroy(void*)
    {
        mount.stop = true;
        if (mount.idle.joinable()) mount.idle.join();
        std::lock_guard<std::recursive_mutex> g(mount.guard);
        if (mount.local && mount.open.empty()) Repack();
    }

    bool Cache(std::string& out)
    {
        std::string base;
        if (const char* x = std::getenv("XDG_CACHE_HOME"); x && x[0] == '/') base = x;
        else if (const char* h = std::getenv("HOME"); h && h[0] == '/') base = std::string(h) + "/.cache";
        else return false;
        mkdir(base.c_str(), 0700);
        const std::string dir = base + "/v31np";
        mkdir(dir.c_str(), 0700);
        char* real = realpath(dir.c_str(), nullptr);
        if (!real) return false;
        out = real;
        std::free(real);
        struct stat st;
        return stat(out.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }
}

int main(int argc, char** argv)
{
    std::vector<char*> given;
    bool tidy = false;
    for (int i = 0; i < argc; i++)
    {
        if (std::strcmp(argv[i], "--tidy") == 0)
        {
            tidy = true;
            continue;
        }
        if (std::strncmp(argv[i], "--idle=", 7) == 0)
        {
            const long seconds = std::strtol(argv[i] + 7, nullptr, 10);
            if (seconds > 0) mount.leave = std::chrono::seconds(seconds);
            continue;
        }
        given.push_back(argv[i]);
    }
    argc = (int)given.size();
    argv = given.data();
    if (argc < 3)
    {
        std::fprintf(stderr, "usage: v31np-mount [--idle=<seconds>] [--tidy] <archive.v31np> <mountpoint> [fuse options]\n");
        return 2;
    }
    if (char* point = realpath(argv[2], nullptr))
    {
        mount.point = point;
        std::free(point);
    }
    char* real = realpath(argv[1], nullptr);
    if (!real)
    {
        std::fprintf(stderr, "v31np-mount: cannot find %s\n", argv[1]);
        return 1;
    }
    mount.archive = real;
    std::free(real);
    const size_t slash = mount.archive.rfind('/');
    mount.folder = mount.archive.substr(0, slash == 0 ? 1 : slash);
    std::string base = mount.archive.substr(slash + 1);
    mount.stem = base.size() > 6 && strcasecmp(base.c_str() + base.size() - 6, ".v31np") == 0 ? base.substr(0, base.size() - 6) : base;
    mount.plate = v31np::Plate();
    mount.system = v31np::System();
    const bool shared = v31np::SharedRoad(v31np::Wide(mount.archive));
    const bool writable = Cache(mount.cache) && access(mount.folder.c_str(), W_OK) == 0;
    mount.local = !shared && writable;
    mount.road = shared && writable;
    mount.now = Load();
    if (!mount.now)
    {
        std::fprintf(stderr, "v31np-mount: %s is not a valid .v31np file\n", argv[1]);
        return 1;
    }
    mount.pending = mount.local && Newest(Scan(mount.now->id)) != nullptr;

    static struct fuse_operations ops = {};
    ops.getattr = GetAttr;
    ops.readdir = ReadDir;
    ops.open = Open;
    ops.read = Read;
    ops.release = Release;
    ops.init = Init;
    ops.destroy = Destroy;
    if (mount.local || mount.road)
    {
        ops.create = Create;
        ops.write = Write;
        ops.truncate = Truncate;
        ops.flush = Flush;
        ops.mkdir = MakeDir;
        ops.unlink = Unlink;
        ops.rmdir = RemoveDir;
        ops.rename = Rename;
        ops.chmod = ChangeMode;
        ops.chown = ChangeOwner;
        ops.utimens = Times;
        ops.symlink = Refuse;
        ops.link = Refuse;
        ops.mknod = RefuseNode;
        ops.statfs = Space;
    }

    std::vector<char*> args;
    args.push_back(argv[0]);
    for (int i = 2; i < argc; i++) args.push_back(argv[i]);
    std::string fsname = "-ofsname=v31np:" + base;
    args.push_back((char*)"-s");
    if (!mount.local && !mount.road) args.push_back((char*)"-oro");
    args.push_back(fsname.data());
    const int result = fuse_main((int)args.size(), args.data(), &ops, nullptr);
    if (tidy && !mount.point.empty() && rmdir(mount.point.c_str()) == 0)
    {
        const size_t cut = mount.point.rfind('/');
        if (cut != std::string::npos && cut > 0) rmdir(mount.point.substr(0, cut).c_str());
    }
    return result;
}
