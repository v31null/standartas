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
    };

    using Chain = std::vector<std::string>;

    struct Shadow
    {
        uint64_t size = 0;
        uint64_t stamp = 0;
        std::map<Chain, std::vector<Row>> parts;
    };

    std::wstring JournalName(const std::wstring& archiveName);
    bool ReadJournal(const std::string& text, Shadow& out);
    std::string WriteJournal(const Shadow& s);
    bool Under(const std::string& key, const std::string& prefix);
    bool Stored(const Row& row);
    uint64_t Logical(const Shadow& s, const Chain& chain, const Row& row);
}
