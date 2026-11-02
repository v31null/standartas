#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace v31np
{
    using ReadAt = std::function<bool(uint64_t pos, void* buf, uint32_t len)>;
    using Pull = std::function<bool(void* buf, uint32_t len)>;
    using Push = std::function<bool(const void* buf, uint32_t len)>;

    constexpr uint64_t Limit = 4503599627370496ull;
    constexpr size_t HeadCap = 256u << 20;

    struct Entry
    {
        std::string key;
        uint64_t at = 0;
        uint64_t size = 0;
    };

    struct Item
    {
        std::string key;
        uint64_t size = 0;
        Pull pull;
    };

    uint32_t Crc32(const void* data, size_t len, uint32_t crc);
    bool ValidUtf8(const std::string& s);
    bool Parse(const ReadAt& read, uint64_t total, std::vector<Entry>& out);
    bool Verify(const ReadAt& read, uint64_t total);
    bool Write(const std::vector<Item>& items, const Push& push);
    uint64_t Measure(const std::vector<Item>& items);
}
