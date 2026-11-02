#pragma once

#include "shadow.h"
#include "traffic.h"

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace v31np
{
    constexpr int64_t Renewal = 40;
    constexpr int64_t Missed = 80;
    constexpr uint32_t Poll = 500;
    constexpr uint32_t SignalWait = 30000;

    class Road
    {
    public:
        virtual ~Road() = default;
        virtual bool Green(uint32_t waitMs) = 0;
        virtual void Red() = 0;
        virtual bool ShowPapers(const std::string& papers) = 0;
        virtual bool ServerNow(int64_t& now) = 0;
        virtual int64_t Clock() = 0;
        virtual uint64_t Ticks() = 0;
        virtual void Sleep(uint32_t ms) = 0;
        virtual bool Archive(uint64_t& size, uint64_t& stamp) = 0;
        virtual bool ReadJournal(std::string& text, bool& exists) = 0;
        virtual bool WriteJournal(const std::string& text) = 0;
        virtual bool Ask(const Papers& holder) = 0;
        virtual bool Cancelled() { return false; }
    };

    struct Step
    {
        std::string action;
        std::vector<std::string> lines;
    };

    struct Request
    {
        Papers me;
        bool urgent = false;
        std::vector<Step> steps;
        std::vector<std::string> keys;
    };

    class Trip
    {
    public:
        virtual ~Trip() = default;
        virtual bool Look(const Shadow& journal) = 0;
        virtual bool Build(const std::vector<Record>& records, const std::function<bool()>& renew) = 0;
        virtual bool Land(Shadow& next, int64_t now) = 0;
        virtual void Abandon() = 0;
    };

    enum class Drove
    {
        Done,
        Refused,
        Failed,
        Lost,
        Skewed,
        Cancelled
    };

    bool Visit(Road& road, const Papers& me, const std::function<void(Shadow&, int64_t)>& business);
    Drove Drive(Road& road, Trip& trip, const Request& request);

    std::string PapersLine(const Papers& p);
    std::string Year(int64_t officerTime);
    std::vector<std::pair<std::string, std::string>> CustodyFiles(std::vector<std::string> levelKeys, const std::vector<Record>& records);
}
