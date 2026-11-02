#pragma once

#include "v31np.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace v31np
{
    struct Row
    {
        char kind = 'A';
        std::string key;
        uint64_t size = 0;
        uint64_t at = 0;
        std::wstring file;
        std::string plate;
        int64_t until = 0;
    };

    using Chain = std::vector<std::string>;

    struct Papers
    {
        std::string plate;
        std::string os;
    };

    struct Car
    {
        uint64_t ticket = 0;
        Papers papers;
        int64_t joined = 0;
        int64_t until = 0;
        bool urgent = false;
        uint64_t seen = 0;
        int64_t checked = 0;
        int64_t clock = 0;
        std::vector<std::string> keys;
    };

    struct Lane
    {
        uint64_t ticket = 0;
        Papers papers;
        int64_t granted = 0;
        int64_t until = 0;
        uint64_t shift = 0;
    };

    struct Event
    {
        uint64_t ticket = 0;
        Papers papers;
        int64_t time = 0;
        std::string kind;
    };

    struct Officer
    {
        uint64_t shift = 0;
        uint64_t next = 1;
        bool shared = false;
        bool held = false;
        Lane lane;
        std::vector<Car> queue;
        std::vector<Event> log;
    };

    struct Shadow
    {
        int version = 2;
        uint64_t size = 0;
        uint64_t stamp = 0;
        Officer officer;
        std::map<Chain, std::vector<Row>> parts;
    };

    std::wstring JournalName(const std::wstring& archiveName);
    bool ReadJournal(const std::string& text, Shadow& out);
    std::string WriteJournal(const Shadow& s);
    bool Under(const std::string& key, const std::string& prefix);
    bool Stored(const Row& row);
    uint64_t Logical(const Shadow& s, const Chain& chain, const Row& row);
    bool Ours(const Row& row, const std::string& plate);
    bool AnyForeign(const Shadow& s, const std::string& plate);
    std::vector<Row> Listing(const std::vector<Entry>& entries, const std::vector<std::string>& dirs);
}
