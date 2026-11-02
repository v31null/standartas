// v31np: a small command-line tool over the shared core.
//   v31np pack <archive.v31np> <folder>       pack the files of a folder
//   v31np list <archive.v31np>                list the keys, sizes and addresses
//   v31np test <archive.v31np>                check the layout and the checksum
//   v31np extract <archive.v31np> <folder>    extract with the GNU/Linux naming rules

#include "tree.h"
#include "v31np.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace
{
    struct File
    {
        int fd = -1;
        explicit File(int f) : fd(f) {}
        ~File()
        {
            if (fd >= 0) close(fd);
        }
        File(const File&) = delete;
        File& operator=(const File&) = delete;
    };

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

    bool WriteAll(int fd, const void* buf, uint32_t len)
    {
        const uint8_t* p = (const uint8_t*)buf;
        while (len > 0)
        {
            ssize_t n = write(fd, p, len);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) return false;
            p += n;
            len -= (uint32_t)n;
        }
        return true;
    }

    int Fail(const std::string& msg)
    {
        std::fprintf(stderr, "v31np: %s\n", msg.c_str());
        return 1;
    }

    bool Open(const char* path, int& fd, uint64_t& size)
    {
        fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0) return false;
        struct stat st;
        if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode))
        {
            close(fd);
            fd = -1;
            return false;
        }
        size = (uint64_t)st.st_size;
        return true;
    }

    using Sources = std::vector<std::pair<std::string, fs::path>>;

    bool Walk(const fs::path& root, const std::string& prefix, bool strict, Sources& out, std::string& error)
    {
        std::error_code ec;
        for (auto it = fs::recursive_directory_iterator(root, fs::directory_options::none, ec); !ec && it != fs::recursive_directory_iterator(); it.increment(ec))
        {
            const auto st = it->symlink_status(ec);
            if (ec) break;
            if (fs::is_symlink(st)) continue;
            if (fs::is_directory(st))
            {
                if (strict && fs::is_empty(it->path(), ec))
                {
                    error = "empty folder " + it->path().string() + ": the format stores no empty folders";
                    return false;
                }
                continue;
            }
            if (fs::is_regular_file(st)) out.push_back({ prefix + fs::relative(it->path(), root).generic_string(), it->path() });
        }
        if (ec) error = ec.message();
        return !ec;
    }

    int Write(const char* out, Sources sources, bool keep)
    {
        std::sort(sources.begin(), sources.end());
        sources.erase(std::unique(sources.begin(), sources.end()), sources.end());
        std::vector<std::unique_ptr<File>> held;
        std::vector<v31np::Item> items;
        for (const auto& [key, p] : sources)
        {
            int fd;
            uint64_t size;
            if (!Open(p.c_str(), fd, size)) return Fail("cannot read " + p.string());
            held.push_back(std::make_unique<File>(fd));
            auto pos = std::make_shared<uint64_t>(0);
            items.push_back({ key, size, [fd, pos](void* buf, uint32_t len)
                {
                    if (!ReadAll(fd, *pos, buf, len)) return false;
                    *pos += len;
                    return true;
                } });
        }
        if (items.empty()) return Fail("nothing to pack");

        const std::string tmp = std::string(out) + ".tmp" + std::to_string(getpid());
        int fd = open(tmp.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
        if (fd < 0) return Fail("cannot create " + tmp + ": " + std::strerror(errno));
        bool ok;
        {
            File w(fd);
            ok = v31np::Write(items, [fd](const void* buf, uint32_t len) { return WriteAll(fd, buf, len); }) && fsync(fd) == 0;
        }
        if (ok && keep)
        {
            ok = link(tmp.c_str(), out) == 0;
            const int why = errno;
            unlink(tmp.c_str());
            if (!ok) return Fail(std::string("cannot create ") + out + ": " + std::strerror(why));
        }
        else if (!ok || rename(tmp.c_str(), out) != 0)
        {
            unlink(tmp.c_str());
            return Fail("writing failed (a key or size may be invalid)");
        }
        std::printf("%zu files packed into %s\n", items.size(), out);
        return 0;
    }

    int Pack(const char* out, const char* dir)
    {
        std::error_code ec;
        const fs::path root = fs::path(dir);
        if (!fs::is_directory(root, ec)) return Fail(std::string(dir) + " is not a folder");
        Sources sources;
        std::string error;
        if (!Walk(root, "", true, sources, error)) return Fail(error);
        return Write(out, std::move(sources), false);
    }

    int Free(const char* dir, const char* name)
    {
        const std::wstring base = v31np::Wide(name);
        for (size_t k = 0;; k++)
        {
            const std::string cand = v31np::Narrow(k == 0 ? base : v31np::Suffix(base, k));
            struct stat st;
            if (lstat((std::string(dir) + "/" + cand).c_str(), &st) != 0)
            {
                std::printf("%s\n", cand.c_str());
                return 0;
            }
        }
    }

    int Make(const char* out, int count, char** paths)
    {
        Sources sources;
        for (int i = 0; i < count; i++)
        {
            std::error_code ec;
            fs::path p = fs::path(paths[i]).lexically_normal();
            if (p.has_filename() == false) p = p.parent_path();
            const std::string name = p.filename().string();
            const auto st = fs::symlink_status(p, ec);
            if (ec || name.empty()) return Fail(std::string("cannot read ") + paths[i]);
            if (fs::is_symlink(st)) continue;
            if (fs::is_directory(st))
            {
                std::string error;
                if (!Walk(p, name + "/", false, sources, error)) return Fail(error);
            }
            else if (fs::is_regular_file(st)) sources.push_back({ name, p });
        }
        return Write(out, std::move(sources), true);
    }

    bool Load(const char* path, int& fd, uint64_t& size, std::vector<v31np::Entry>& entries)
    {
        if (!Open(path, fd, size)) return false;
        return v31np::Parse([fd](uint64_t pos, void* buf, uint32_t len) { return ReadAll(fd, pos, buf, len); }, size, entries);
    }

    int List(const char* path)
    {
        int fd;
        uint64_t size;
        std::vector<v31np::Entry> entries;
        if (!Load(path, fd, size, entries)) return Fail(std::string(path) + " is not a valid .v31np file");
        File f(fd);
        for (const auto& e : entries) std::printf("%12llu  %12llu  %s\n", (unsigned long long)e.size, (unsigned long long)e.at, e.key.c_str());
        std::printf("%zu files, %llu bytes\n", entries.size(), (unsigned long long)size);
        return 0;
    }

    int Test(const char* path)
    {
        int fd;
        uint64_t size;
        std::vector<v31np::Entry> entries;
        if (!Load(path, fd, size, entries)) return Fail(std::string(path) + " is not a valid .v31np file");
        File f(fd);
        if (!v31np::Verify([fd](uint64_t pos, void* buf, uint32_t len) { return ReadAll(fd, pos, buf, len); }, size))
            return Fail("checksum mismatch");
        std::printf("OK: %zu files, layout and checksum valid\n", entries.size());
        return 0;
    }

    // Writes a file without replacing anything; a taken name receives a suffix of Table G-2.
    int Create(const std::string& dir, std::wstring name, std::string& used)
    {
        for (size_t k = 0;; k++)
        {
            std::wstring cand = k == 0 ? name : v31np::Suffix(name, k);
            used = dir + "/" + v31np::Narrow(cand);
            int fd = open(used.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
            if (fd >= 0 || errno != EEXIST) return fd;
        }
    }

    // A folder: merged with a folder of the same name, a suffix where a file holds the name.
    bool Folder(const std::string& dir, const std::wstring& name, std::string& used)
    {
        for (size_t k = 0;; k++)
        {
            std::wstring cand = k == 0 ? name : v31np::Suffix(name, k);
            used = dir + "/" + v31np::Narrow(cand);
            if (mkdir(used.c_str(), 0755) == 0) return true;
            if (errno != EEXIST) return false;
            struct stat st;
            if (lstat(used.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return true;
        }
    }

    int Extract(const char* path, const char* dest)
    {
        int fd;
        uint64_t size;
        std::vector<v31np::Entry> entries;
        if (!Load(path, fd, size, entries)) return Fail(std::string(path) + " is not a valid .v31np file");
        File f(fd);
        std::error_code ec;
        fs::create_directories(dest, ec);
        if (ec) return Fail(ec.message());

        v31np::Tree t;
        t.Build(entries);
        std::vector<std::string> real(t.nodes.size());
        real[0] = dest;
        std::vector<uint8_t> buf(1u << 20);
        size_t done = 0;
        for (size_t i = 1; i < t.nodes.size(); i++)
        {
            const auto& n = t.nodes[i];
            const std::string& parent = real[n.parent];
            if (parent.empty()) continue;
            if (n.dir)
            {
                if (!Folder(parent, n.natural, real[i])) real[i].clear();
                continue;
            }
            int out = Create(parent, n.natural, real[i]);
            if (out < 0) return Fail("cannot create " + real[i] + ": " + std::strerror(errno));
            File w(out);
            const auto& e = entries[n.entry];
            for (uint64_t at = 0; at < e.size;)
            {
                uint32_t len = (uint32_t)std::min<uint64_t>(buf.size(), e.size - at);
                if (!ReadAll(fd, e.at + at, buf.data(), len) || !WriteAll(out, buf.data(), len)) return Fail("cannot write " + real[i]);
                at += len;
            }
            done++;
        }
        std::printf("%zu files extracted into %s\n", done, dest);
        return 0;
    }
}

int main(int argc, char** argv)
{
    if (argc == 4 && std::strcmp(argv[1], "pack") == 0) return Pack(argv[2], argv[3]);
    if (argc == 3 && std::strcmp(argv[1], "list") == 0) return List(argv[2]);
    if (argc == 3 && std::strcmp(argv[1], "test") == 0) return Test(argv[2]);
    if (argc == 4 && std::strcmp(argv[1], "extract") == 0) return Extract(argv[2], argv[3]);
    if (argc >= 4 && std::strcmp(argv[1], "make") == 0) return Make(argv[2], argc - 3, argv + 3);
    if (argc == 4 && std::strcmp(argv[1], "name") == 0) return Free(argv[2], argv[3]);
    std::fprintf(stderr, "usage: v31np pack <archive> <folder> | make <new archive> <path>... | name <folder> <name> | list <archive> | test <archive> | extract <archive> <folder>\n");
    return 2;
}
