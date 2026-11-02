#pragma once

#include <cstdint>
#include <string>

namespace v31np
{
    std::string Stamp(int64_t unixSeconds);
    bool Unstamp(const std::string& stamp, int64_t& unixSeconds);
    int64_t Now();
}
