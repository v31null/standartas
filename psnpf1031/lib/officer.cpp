#include "officer.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#pragma comment(lib, "advapi32.lib")
#else
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#endif

#include <cwctype>

namespace v31np
{
    std::wstring SignalName(const std::wstring& archiveName)
    {
        std::wstring stem = archiveName;
        if (stem.size() > 6)
        {
            std::wstring tail = stem.substr(stem.size() - 6);
            for (auto& c : tail) c = (wchar_t)std::towlower(c);
            if (tail == L".v31np") stem.resize(stem.size() - 6);
        }
        return L"NIEURFASMIT UUTI " + stem + L".v31np.lock";
    }

    uint32_t PlateDigits(const std::string& machineId)
    {
        uint64_t h = 14695981039346656037ull;
        for (unsigned char c : machineId)
        {
            h ^= c;
            h *= 1099511628211ull;
        }
        return (uint32_t)(h % 1000000000ull);
    }

#ifdef _WIN32
    namespace
    {
        std::string Utf8(const std::wstring& w)
        {
            if (w.empty()) return {};
            int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
            std::string s((size_t)(n > 0 ? n : 0), '\0');
            if (n > 0) WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), s.data(), n, nullptr, nullptr);
            return s;
        }

        OVERLAPPED At()
        {
            OVERLAPPED ov{};
            ov.Offset = (DWORD)(SignalAt & 0xFFFFFFFFull);
            ov.OffsetHigh = (DWORD)(SignalAt >> 32);
            return ov;
        }
    }

    std::string Plate()
    {
        wchar_t name[256];
        DWORD len = 256;
        std::wstring host;
        if (GetComputerNameExW(ComputerNameDnsHostname, name, &len)) host.assign(name, len);
        wchar_t guid[64];
        DWORD size = sizeof(guid);
        std::string id;
        if (RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid", RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY, nullptr, guid, &size) == ERROR_SUCCESS)
        {
            std::wstring g(guid);
            for (auto& c : g) c = (wchar_t)std::towlower(c);
            id = Utf8(g);
        }
        return Utf8(host) + "-" + std::to_string(PlateDigits(id));
    }

    std::string System()
    {
        wchar_t name[256];
        DWORD size = sizeof(name);
        std::wstring product = L"Windows";
        if (RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName", RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY, nullptr, name, &size) == ERROR_SUCCESS && name[0]) product = name;
        using Get = LONG(WINAPI*)(OSVERSIONINFOW*);
        const Get get = reinterpret_cast<Get>(reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion")));
        OSVERSIONINFOW v{};
        v.dwOSVersionInfoSize = sizeof(v);
        if (!get || get(&v) != 0) return Utf8(product);
        if (v.dwBuildNumber >= 22000 && product.compare(0, 11, L"Windows 10 ") == 0) product.replace(8, 2, L"11");
        return Utf8(product) + " " + std::to_string(v.dwMajorVersion) + "." + std::to_string(v.dwMinorVersion) + "." + std::to_string(v.dwBuildNumber);
    }

    namespace
    {
        bool Inside(const std::wstring& path, std::wstring dir)
        {
            while (!dir.empty() && (dir.back() == L'\\' || dir.back() == L'/')) dir.pop_back();
            if (dir.empty() || path.size() < dir.size()) return false;
            if (CompareStringOrdinal(path.c_str(), (int)dir.size(), dir.c_str(), (int)dir.size(), TRUE) != CSTR_EQUAL) return false;
            return path.size() == dir.size() || path[dir.size()] == L'\\' || path[dir.size()] == L'/';
        }

        bool Served(const std::wstring& archivePath)
        {
            DWORD n = GetFullPathNameW(archivePath.c_str(), 0, nullptr, nullptr);
            if (!n) return false;
            std::wstring full((size_t)n, L'\0');
            n = GetFullPathNameW(archivePath.c_str(), n, full.data(), nullptr);
            full.resize(n);
            HKEY key;
            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Shares", 0, KEY_READ, &key) != ERROR_SUCCESS) return false;
            bool hit = false;
            for (DWORD i = 0; !hit; i++)
            {
                wchar_t name[256];
                DWORD nameLen = 256;
                DWORD type = 0;
                DWORD bytes = 0;
                if (RegEnumValueW(key, i, name, &nameLen, nullptr, &type, nullptr, &bytes) != ERROR_SUCCESS) break;
                if (type != REG_MULTI_SZ || bytes == 0) continue;
                std::wstring data(bytes / sizeof(wchar_t) + 1, L'\0');
                nameLen = 256;
                if (RegEnumValueW(key, i, name, &nameLen, nullptr, &type, (BYTE*)data.data(), &bytes) != ERROR_SUCCESS) continue;
                for (const wchar_t* p = data.c_str(); *p; p += wcslen(p) + 1)
                    if (wcsncmp(p, L"Path=", 5) == 0 && Inside(full, p + 5)) hit = true;
            }
            RegCloseKey(key);
            return hit;
        }
    }

    bool SharedRoad(const std::wstring& archivePath)
    {
        wchar_t volume[MAX_PATH + 1];
        if (!GetVolumePathNameW(archivePath.c_str(), volume, MAX_PATH + 1)) return false;
        return GetDriveTypeW(volume) == DRIVE_REMOTE || Served(archivePath);
    }

    Signal::Signal(const std::wstring& postPath)
    {
        HANDLE h = CreateFileW(postPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN, nullptr);
        post = h == INVALID_HANDLE_VALUE ? -1 : (intptr_t)h;
    }

    Signal::~Signal()
    {
        Red();
        if (post != -1) CloseHandle((HANDLE)post);
    }

    bool Signal::Green(uint32_t waitMs)
    {
        if (post == -1) return false;
        if (green) return true;
        const ULONGLONG end = GetTickCount64() + waitMs;
        for (;;)
        {
            OVERLAPPED ov = At();
            if (LockFileEx((HANDLE)post, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &ov))
            {
                green = true;
                return true;
            }
            const DWORD e = GetLastError();
            if (e != ERROR_LOCK_VIOLATION && e != ERROR_IO_PENDING) return false;
            if (GetTickCount64() >= end) return false;
            Sleep(25);
        }
    }

    void Signal::Red()
    {
        if (!green) return;
        OVERLAPPED ov = At();
        UnlockFileEx((HANDLE)post, 0, 1, 0, &ov);
        green = false;
    }

    bool Signal::ShowPapers(const std::string& papers)
    {
        if (!green) return false;
        LARGE_INTEGER zero{};
        if (!SetFilePointerEx((HANDLE)post, zero, nullptr, FILE_BEGIN)) return false;
        DWORD put = 0;
        if (!papers.empty() && (!WriteFile((HANDLE)post, papers.data(), (DWORD)papers.size(), &put, nullptr) || put != papers.size())) return false;
        return SetEndOfFile((HANDLE)post) != FALSE;
    }

    bool Signal::ServerNow(int64_t& unixSeconds)
    {
        if (!green) return false;
        if (!FlushFileBuffers((HANDLE)post)) return false;
        FILE_BASIC_INFO info{};
        if (!GetFileInformationByHandleEx((HANDLE)post, FileBasicInfo, &info, sizeof(info))) return false;
        const int64_t ft = info.LastWriteTime.QuadPart;
        if (ft < 116444736000000000ll) return false;
        unixSeconds = (ft - 116444736000000000ll) / 10000000ll;
        return true;
    }

    bool Signal::PapersPlease(std::string& papers) const
    {
        if (post == -1) return false;
        char buf[4096];
        OVERLAPPED ov{};
        DWORD got = 0;
        if (!ReadFile((HANDLE)post, buf, sizeof(buf), &got, &ov))
        {
            if (GetLastError() != ERROR_HANDLE_EOF) return false;
            got = 0;
        }
        papers.assign(buf, got);
        return true;
    }
#else
    namespace
    {
        std::string Narrow8(const std::wstring& w)
        {
            std::string r;
            for (wchar_t wc : w)
            {
                const uint32_t c = (uint32_t)wc;
                if (c < 0x80) r += (char)c;
                else if (c < 0x800)
                {
                    r += (char)(0xC0 | (c >> 6));
                    r += (char)(0x80 | (c & 0x3F));
                }
                else if (c < 0x10000)
                {
                    r += (char)(0xE0 | (c >> 12));
                    r += (char)(0x80 | ((c >> 6) & 0x3F));
                    r += (char)(0x80 | (c & 0x3F));
                }
                else
                {
                    r += (char)(0xF0 | (c >> 18));
                    r += (char)(0x80 | ((c >> 12) & 0x3F));
                    r += (char)(0x80 | ((c >> 6) & 0x3F));
                    r += (char)(0x80 | (c & 0x3F));
                }
            }
            return r;
        }

        struct flock At(short type)
        {
            struct flock fl;
            std::memset(&fl, 0, sizeof fl);
            fl.l_type = type;
            fl.l_whence = SEEK_SET;
            fl.l_start = (off_t)SignalAt;
            fl.l_len = 1;
            return fl;
        }

        std::string Folder(const std::string& path)
        {
            const size_t slash = path.rfind('/');
            if (slash == std::string::npos) return ".";
            return slash == 0 ? "/" : path.substr(0, slash);
        }

        bool Remote(const std::string& folder)
        {
            struct statfs st;
            if (statfs(folder.c_str(), &st) != 0) return false;
            switch ((uint64_t)(uint32_t)st.f_type)
            {
            case 0xFF534D42u:
            case 0xFE534D42u:
            case 0x517Bu:
            case 0x6969u:
            case 0x00C36400u:
                return true;
            case 0x65735546u:
                break;
            default:
                return false;
            }
            std::ifstream mounts("/proc/self/mounts");
            std::string dev, dir, type, best, bestType;
            std::string rest;
            while (mounts >> dev >> dir >> type && std::getline(mounts, rest))
            {
                std::string d;
                for (size_t i = 0; i < dir.size(); i++)
                {
                    if (dir[i] == '\\' && i + 3 < dir.size())
                    {
                        d += (char)std::strtol(dir.substr(i + 1, 3).c_str(), nullptr, 8);
                        i += 3;
                    }
                    else d += dir[i];
                }
                const bool under = folder == d || (folder.compare(0, d.size(), d) == 0 && (d == "/" || folder[d.size()] == '/'));
                if (under && d.size() >= best.size())
                {
                    best = d;
                    bestType = type;
                }
            }
            return bestType == "fuse.gvfsd-fuse" || bestType == "fuse.sshfs" || bestType == "fuse.rclone";
        }
    }

    std::string Plate()
    {
        char host[256] = {};
        gethostname(host, sizeof host - 1);
        std::string id;
        std::ifstream in("/etc/machine-id");
        std::getline(in, id);
        while (!id.empty() && (id.back() == ' ' || id.back() == '\r' || id.back() == '\n' || id.back() == '\t')) id.pop_back();
        for (auto& c : id) c = (char)std::tolower((unsigned char)c);
        return std::string(host) + "-" + std::to_string(PlateDigits(id));
    }

    bool SharedRoad(const std::wstring& archivePath)
    {
        const std::string folder = Folder(Narrow8(archivePath));
        return Remote(folder) || Hosted(folder);
    }

    Signal::Signal(const std::wstring& postPath)
    {
        const std::string path = Narrow8(postPath);
        int fd = open(path.c_str(), O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
        if (fd >= 0) Hide(path);
        else if (errno == EEXIST) fd = open(path.c_str(), O_RDWR | O_CLOEXEC);
        post = fd;
    }

    Signal::~Signal()
    {
        Red();
        if (post != -1) close((int)post);
    }

    bool Signal::Green(uint32_t waitMs)
    {
        if (post == -1) return false;
        if (green) return true;
        const auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(waitMs);
        for (;;)
        {
            struct flock fl = At(F_WRLCK);
            if (fcntl((int)post, F_OFD_SETLK, &fl) == 0)
            {
                green = true;
                return true;
            }
            if (errno != EAGAIN && errno != EACCES && errno != EINTR) return false;
            if (std::chrono::steady_clock::now() >= end) return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }
    }

    void Signal::Red()
    {
        if (!green) return;
        struct flock fl = At(F_UNLCK);
        fcntl((int)post, F_OFD_SETLK, &fl);
        green = false;
    }

    bool Signal::ShowPapers(const std::string& papers)
    {
        if (!green) return false;
        size_t done = 0;
        while (done < papers.size())
        {
            ssize_t n = pwrite((int)post, papers.data() + done, papers.size() - done, (off_t)done);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) return false;
            done += (size_t)n;
        }
        return ftruncate((int)post, (off_t)papers.size()) == 0 && fsync((int)post) == 0;
    }

    bool Signal::PapersPlease(std::string& papers) const
    {
        if (post == -1) return false;
        char self[64];
        std::snprintf(self, sizeof self, "/proc/self/fd/%d", (int)post);
        int fd = open(self, O_RDONLY | O_DIRECT | O_CLOEXEC);
        const bool direct = fd >= 0;
        if (!direct) fd = open(self, O_RDONLY | O_CLOEXEC);
        if (fd < 0) return false;
        void* buf = nullptr;
        if (posix_memalign(&buf, 4096, 4096) != 0)
        {
            close(fd);
            return false;
        }
        ssize_t n = pread(fd, buf, 4096, 0);
        if (n < 0 && direct && errno == EINVAL)
        {
            close(fd);
            fd = open(self, O_RDONLY | O_CLOEXEC);
            n = fd >= 0 ? pread(fd, buf, 4096, 0) : -1;
        }
        if (n >= 0) papers.assign((const char*)buf, (size_t)n);
        std::free(buf);
        if (fd >= 0) close(fd);
        return n >= 0;
    }

    bool Signal::ServerNow(int64_t& unixSeconds)
    {
        if (!green || fsync((int)post) != 0) return false;
        struct statx sx;
        if (statx((int)post, "", AT_EMPTY_PATH | AT_STATX_FORCE_SYNC, STATX_MTIME, &sx) != 0 || !(sx.stx_mask & STATX_MTIME)) return false;
        unixSeconds = (int64_t)sx.stx_mtime.tv_sec;
        return true;
    }

    namespace
    {
        std::vector<std::string> Lines(const std::string& text)
        {
            std::vector<std::string> out;
            std::string cur;
            size_t at = 0;
            while (at <= text.size())
            {
                size_t end = text.find('\n', at);
                if (end == std::string::npos) end = text.size();
                std::string line = text.substr(at, end - at);
                at = end + 1;
                if (!line.empty() && line.back() == '\r') line.pop_back();
                if (!line.empty() && line.back() == '\\')
                {
                    line.pop_back();
                    cur += line;
                    continue;
                }
                out.push_back(cur + line);
                cur.clear();
            }
            if (!cur.empty()) out.push_back(cur);
            return out;
        }

        std::string Trim(const std::string& s)
        {
            size_t a = s.find_first_not_of(" \t");
            if (a == std::string::npos) return {};
            size_t b = s.find_last_not_of(" \t");
            return s.substr(a, b - a + 1);
        }

        std::string Key(const std::string& s)
        {
            std::string k;
            for (char c : s)
                if (c != ' ' && c != '\t') k += (char)std::tolower((unsigned char)c);
            return k;
        }

        bool Yes(const std::string& v)
        {
            const std::string k = Key(v);
            return k == "yes" || k == "true" || k == "1";
        }

        bool No(const std::string& v)
        {
            const std::string k = Key(v);
            return k == "no" || k == "false" || k == "0";
        }

        bool Expand(const std::string& in, const std::string& section, const Host& host, std::string& out)
        {
            out.clear();
            for (size_t i = 0; i < in.size(); i++)
            {
                if (in[i] != '%')
                {
                    out += in[i];
                    continue;
                }
                if (i + 1 >= in.size()) return false;
                const char c = in[++i];
                if (c == 'U' || c == 'u') out += host.user;
                else if (c == 'H') out += host.home;
                else if (c == 'S') out += section;
                else if (c == '%') out += '%';
                else return false;
            }
            return !out.empty() && out[0] == '/';
        }

        void Section(const std::string& name, bool seenPath, const std::string& path, bool printable, bool available, const Host& host, Hosting& h)
        {
            const std::string k = Key(name);
            if (k.empty() || k == "global" || printable || !available) return;
            std::string p;
            if (seenPath)
            {
                if (Expand(path, k == "homes" ? host.user : name, host, p)) h.paths.push_back(p);
            }
            else if (k == "homes" && !host.home.empty()) h.paths.push_back(host.home);
        }
    }

    void ReadSamba(const std::string& text, const Host& host, Hosting& h)
    {
        std::string section;
        std::string path;
        bool seenPath = false;
        bool printable = false;
        bool available = true;
        bool any = false;
        for (const auto& raw : Lines(text))
        {
            const std::string line = Trim(raw);
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;
            if (line[0] == '[')
            {
                if (any) Section(section, seenPath, path, printable, available, host, h);
                const size_t close = line.find(']');
                section = Trim(line.substr(1, close == std::string::npos ? std::string::npos : close - 1));
                path.clear();
                seenPath = false;
                printable = false;
                available = true;
                any = true;
                continue;
            }
            const size_t eq = line.find('=');
            if (eq == std::string::npos) continue;
            const std::string k = Key(line.substr(0, eq));
            const std::string v = Trim(line.substr(eq + 1));
            if (k == "include")
            {
                std::string file;
                if (Expand(v, section, host, file)) h.includes.push_back(file);
            }
            else if (!any || Key(section) == "global")
            {
                if (k == "usersharepath") h.usershares = v;
            }
            else if (k == "path" || k == "directory")
            {
                path = v;
                seenPath = true;
            }
            else if (k == "printable" || k == "printok") printable = Yes(v);
            else if (k == "available") available = !No(v);
        }
        if (any) Section(section, seenPath, path, printable, available, host, h);
    }

    void ReadUsershare(const std::string& text, Hosting& h)
    {
        for (const auto& raw : Lines(text))
        {
            const std::string line = Trim(raw);
            if (line.compare(0, 5, "path=") == 0 && line.size() > 6 && line[5] == '/') h.paths.push_back(line.substr(5));
        }
    }

    void ReadExports(const std::string& text, Hosting& h)
    {
        for (const auto& raw : Lines(text))
        {
            const std::string line = Trim(raw);
            if (line.empty() || line[0] == '#') continue;
            std::string p;
            size_t i = 0;
            const bool quoted = line[0] == '"';
            if (quoted) i = 1;
            for (; i < line.size(); i++)
            {
                const char c = line[i];
                if (quoted ? c == '"' : (c == ' ' || c == '\t')) break;
                if (c == '\\' && i + 3 < line.size() && line[i + 1] >= '0' && line[i + 1] <= '3' && line[i + 2] >= '0' && line[i + 2] <= '7' && line[i + 3] >= '0' && line[i + 3] <= '7')
                {
                    p += (char)(((line[i + 1] - '0') << 6) | ((line[i + 2] - '0') << 3) | (line[i + 3] - '0'));
                    i += 3;
                    continue;
                }
                p += c;
            }
            if (!p.empty() && p[0] == '/') h.paths.push_back(p);
        }
    }

    bool Within(const std::string& path, std::string dir)
    {
        while (dir.size() > 1 && dir.back() == '/') dir.pop_back();
        if (dir.empty() || path.size() < dir.size() || path.compare(0, dir.size(), dir) != 0) return false;
        return path.size() == dir.size() || dir == "/" || path[dir.size()] == '/';
    }

    namespace
    {
        bool Slurp(const std::string& file, std::string& text)
        {
            std::ifstream in(file, std::ios::binary);
            if (!in) return false;
            text.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            return true;
        }

        void Samba(const std::string& file, const Host& host, Hosting& h, int depth)
        {
            std::string text;
            if (depth > 4 || !Slurp(file, text)) return;
            Hosting here;
            ReadSamba(text, host, here);
            h.paths.insert(h.paths.end(), here.paths.begin(), here.paths.end());
            if (!here.usershares.empty()) h.usershares = here.usershares;
            for (const auto& inc : here.includes) Samba(inc, host, h, depth + 1);
        }

        std::vector<std::string> Listed(const std::string& dir)
        {
            std::vector<std::string> out;
            DIR* d = opendir(dir.c_str());
            if (!d) return out;
            while (dirent* e = readdir(d))
                if (e->d_name[0] != '.') out.push_back(dir + "/" + e->d_name);
            closedir(d);
            return out;
        }
    }

    bool Hosted(const std::string& folder)
    {
        char* real = realpath(folder.c_str(), nullptr);
        if (!real) return false;
        const std::string here = real;
        free(real);
        Host host;
        if (const passwd* pw = getpwuid(getuid()))
        {
            host.user = pw->pw_name ? pw->pw_name : "";
            host.home = pw->pw_dir ? pw->pw_dir : "";
        }
        Hosting h;
        h.usershares = "/var/lib/samba/usershares";
        Samba("/etc/samba/smb.conf", host, h, 0);
        for (const auto& f : Listed(h.usershares))
        {
            std::string text;
            if (Slurp(f, text)) ReadUsershare(text, h);
        }
        std::string text;
        if (Slurp("/etc/exports", text)) ReadExports(text, h);
        for (const auto& f : Listed("/etc/exports.d"))
            if (f.size() > 8 && f.compare(f.size() - 8, 8, ".exports") == 0 && Slurp(f, text)) ReadExports(text, h);
        for (const auto& p : h.paths)
        {
            char* r = realpath(p.c_str(), nullptr);
            if (!r) continue;
            const bool in = Within(here, r);
            free(r);
            if (in) return true;
        }
        return false;
    }

    std::string OsRelease(const std::string& text)
    {
        size_t at = 0;
        while (at < text.size())
        {
            size_t end = text.find('\n', at);
            if (end == std::string::npos) end = text.size();
            std::string line = text.substr(at, end - at);
            at = end + 1;
            if (line.compare(0, 12, "PRETTY_NAME=") != 0) continue;
            std::string v = line.substr(12);
            std::string out;
            char quote = 0;
            if (!v.empty() && (v[0] == '"' || v[0] == '\''))
            {
                quote = v[0];
                v.erase(0, 1);
            }
            for (size_t i = 0; i < v.size(); i++)
            {
                if (quote && v[i] == quote) break;
                if (quote == '"' && v[i] == '\\' && i + 1 < v.size()) i++;
                out += v[i];
            }
            while (!out.empty() && (out.back() == ' ' || out.back() == '\r' || out.back() == '\t')) out.pop_back();
            return out;
        }
        return {};
    }

    std::string System()
    {
        for (const char* file : { "/etc/os-release", "/usr/lib/os-release" })
        {
            std::ifstream in(file, std::ios::binary);
            if (!in) continue;
            const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            const std::string name = OsRelease(text);
            if (!name.empty()) return name;
        }
        struct utsname u;
        if (uname(&u) == 0) return std::string(u.sysname) + " " + u.release;
        return "Linux";
    }

    void Hide(const std::string& path)
    {
        const size_t slash = path.rfind('/');
        const std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
        const std::string list = Folder(path) + "/.hidden";
        std::ifstream in(list, std::ios::binary);
        std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();
        size_t at = 0;
        while (at < text.size())
        {
            size_t end = text.find('\n', at);
            if (end == std::string::npos) end = text.size();
            std::string line = text.substr(at, end - at);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line == name) return;
            at = end + 1;
        }
        std::ofstream out(list, std::ios::app | std::ios::binary);
        if (!text.empty() && text.back() != '\n') out << "\n";
        out << name << "\n";
    }
#endif
}
