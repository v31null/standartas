#pragma once

#include "shadow.h"

#include <cstdint>
#include <string>
#include <vector>

namespace v31np
{
    constexpr size_t LogKeep = 16;
    constexpr int64_t Lease = 120;
    constexpr int64_t Skew = 60;

    uint64_t TakeTicket(Officer& o, const Papers& me, int64_t now, int64_t lease, bool urgent, const std::vector<std::string>& keys, int64_t clock);

    enum class Verdict
    {
        Fine,
        Condemned,
        Refused
    };

    Verdict Assess(const Officer& o, uint64_t ticket);
    Verdict Judge(Officer& o, uint64_t ticket, int64_t now);
    void Withdraw(Officer& o, uint64_t ticket, int64_t now);
    void Sweep(Officer& o, int64_t now);
    void Sweep(Shadow& s, int64_t now);
    void Landed(std::vector<Row>& rows, const std::string& key);
    void RenewFolders(Shadow& s, const std::string& plate, int64_t now, int64_t lease);
    const Car* NextCar(const Officer& o);
    size_t Ahead(const Officer& o, uint64_t ticket);
    bool WaveOn(Officer& o, uint64_t ticket, int64_t now, int64_t lease);
    bool SameShift(const Officer& o, uint64_t ticket, uint64_t shift);
    bool Renew(Officer& o, uint64_t ticket, uint64_t shift, int64_t now, int64_t lease);
    bool Leave(Officer& o, uint64_t ticket, uint64_t shift, int64_t now);
    bool Override(Officer& o, uint64_t ticket, int64_t now, int64_t lease);
    bool Tag(Officer& o, uint64_t by, int64_t now);
    std::vector<std::string> Changed(const Shadow& before, const Shadow& now, const std::vector<Entry>& stored, const std::vector<std::string>& keys);

    struct Custody
    {
        uint64_t action = 0;
        uint64_t list = 0;
        std::string actionKey;
        std::string listKey;
    };

    Custody NextCustody(const std::vector<std::string>& keys, const std::string& plate, const std::string& year);

    struct Record
    {
        Papers papers;
        int64_t time = 0;
        uint64_t shift = 0;
        uint64_t ticket = 0;
        std::string action;
        std::vector<std::string> lines;
    };

    std::string CustodyText(const Record& r);
    bool ReadCustody(const std::string& text, Record& r);
    Record Citation(const Car& car, uint64_t shift, int64_t now, bool everyone);
}
