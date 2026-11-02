#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace v31np
{
    constexpr uint64_t SignalAt = 1ull << 40;

    std::wstring SignalName(const std::wstring& archiveName);
    uint32_t PlateDigits(const std::string& machineId);
    std::string Plate();
    std::string System();
    bool SharedRoad(const std::wstring& archivePath);
#ifndef _WIN32
    void Hide(const std::string& path);
    struct Host
    {
        std::string user;
        std::string home;
    };

    struct Hosting
    {
        std::vector<std::string> paths;
        std::vector<std::string> includes;
        std::string usershares;
    };

    void ReadSamba(const std::string& text, const Host& host, Hosting& h);
    void ReadUsershare(const std::string& text, Hosting& h);
    void ReadExports(const std::string& text, Hosting& h);
    bool Within(const std::string& path, std::string dir);
    bool Hosted(const std::string& folder);
    std::string OsRelease(const std::string& text);
#endif

    class Signal
    {
    public:
        explicit Signal(const std::wstring& postPath);
        ~Signal();
        Signal(const Signal&) = delete;
        Signal& operator=(const Signal&) = delete;

        bool Green(uint32_t waitMs);
        void Red();
        bool IsGreen() const { return green; }
        bool ShowPapers(const std::string& papers);
        bool PapersPlease(std::string& papers) const;
        bool ServerNow(int64_t& unixSeconds);

    private:
        intptr_t post = -1;
        bool green = false;
    };
}
