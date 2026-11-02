#include "classes.h"

#include <algorithm>
#include <cstdint>
#include <iterator>

namespace v31np
{
    namespace
    {
        struct Range
        {
            uint16_t first;
            uint16_t last;
            bool space;
        };

        const Range Ranges[] = {
            { 0x0009, 0x000D, true },
            { 0x0020, 0x0020, true },
            { 0x0021, 0x002F, false },
            { 0x003A, 0x0040, false },
            { 0x005B, 0x0060, false },
            { 0x007B, 0x007E, false },
            { 0x0085, 0x0085, true },
            { 0x00A0, 0x00A0, true },
            { 0x00A1, 0x00BF, false },
            { 0x00D7, 0x00D7, false },
            { 0x00F7, 0x00F7, false },
            { 0x037E, 0x037E, false },
            { 0x0387, 0x0387, false },
            { 0x055A, 0x055F, false },
            { 0x0589, 0x058A, false },
            { 0x05BE, 0x05BE, false },
            { 0x05C0, 0x05C0, false },
            { 0x05C3, 0x05C3, false },
            { 0x05C6, 0x05C6, false },
            { 0x05F3, 0x05F4, false },
            { 0x0609, 0x060A, false },
            { 0x060C, 0x060D, false },
            { 0x061B, 0x061B, false },
            { 0x061E, 0x061F, false },
            { 0x066A, 0x066D, false },
            { 0x06D4, 0x06D4, false },
            { 0x0700, 0x070D, false },
            { 0x07F7, 0x07F9, false },
            { 0x0964, 0x0965, false },
            { 0x0970, 0x0970, false },
            { 0x0DF4, 0x0DF4, false },
            { 0x0E4F, 0x0E4F, false },
            { 0x0E5A, 0x0E5B, false },
            { 0x0F04, 0x0F12, false },
            { 0x0F3A, 0x0F3D, false },
            { 0x0F85, 0x0F85, false },
            { 0x0FD0, 0x0FD4, false },
            { 0x104A, 0x104F, false },
            { 0x10FB, 0x10FB, false },
            { 0x1361, 0x1368, false },
            { 0x166D, 0x166E, false },
            { 0x1680, 0x1680, true },
            { 0x169B, 0x169C, false },
            { 0x16EB, 0x16ED, false },
            { 0x1735, 0x1736, false },
            { 0x17D4, 0x17D6, false },
            { 0x17D8, 0x17DA, false },
            { 0x1800, 0x180A, false },
            { 0x180E, 0x180E, true },
            { 0x1944, 0x1945, false },
            { 0x19DE, 0x19DF, false },
            { 0x1A1E, 0x1A1F, false },
            { 0x1B5A, 0x1B60, false },
            { 0x1C3B, 0x1C3F, false },
            { 0x1C7E, 0x1C7F, false },
            { 0x2000, 0x200A, true },
            { 0x2010, 0x2027, false },
            { 0x2028, 0x2029, true },
            { 0x202F, 0x202F, true },
            { 0x2030, 0x2043, false },
            { 0x2045, 0x2051, false },
            { 0x2053, 0x205E, false },
            { 0x205F, 0x205F, true },
            { 0x207D, 0x207E, false },
            { 0x208D, 0x208E, false },
            { 0x2329, 0x232A, false },
            { 0x2768, 0x2775, false },
            { 0x27C5, 0x27C6, false },
            { 0x27E6, 0x27EF, false },
            { 0x2983, 0x2998, false },
            { 0x29D8, 0x29DB, false },
            { 0x29FC, 0x29FD, false },
            { 0x2CF9, 0x2CFC, false },
            { 0x2CFE, 0x2CFF, false },
            { 0x2E00, 0x2E2E, false },
            { 0x2E30, 0x2E30, false },
            { 0x3000, 0x3000, true },
            { 0x3001, 0x3003, false },
            { 0x3008, 0x3011, false },
            { 0x3014, 0x301F, false },
            { 0x3030, 0x3030, false },
            { 0x303D, 0x303D, false },
            { 0x30A0, 0x30A0, false },
            { 0x30FB, 0x30FB, false },
            { 0xA60D, 0xA60F, false },
            { 0xA673, 0xA673, false },
            { 0xA67E, 0xA67E, false },
            { 0xA874, 0xA877, false },
            { 0xA8CE, 0xA8CF, false },
            { 0xA92E, 0xA92F, false },
            { 0xA95F, 0xA95F, false },
            { 0xAA5C, 0xAA5F, false },
            { 0xF56B, 0xF56C, false },
            { 0xFD3E, 0xFD3F, false },
            { 0xFE10, 0xFE19, false },
            { 0xFE30, 0xFE52, false },
            { 0xFE54, 0xFE61, false },
            { 0xFE63, 0xFE63, false },
            { 0xFE68, 0xFE68, false },
            { 0xFE6A, 0xFE6B, false },
            { 0xFEFF, 0xFEFF, true },
            { 0xFF01, 0xFF03, false },
            { 0xFF05, 0xFF0A, false },
            { 0xFF0C, 0xFF0F, false },
            { 0xFF1A, 0xFF1B, false },
            { 0xFF1F, 0xFF20, false },
            { 0xFF3B, 0xFF3D, false },
            { 0xFF3F, 0xFF3F, false },
            { 0xFF5B, 0xFF5B, false },
            { 0xFF5D, 0xFF5D, false },
            { 0xFF5F, 0xFF65, false },
        };

        struct Shift
        {
            char32_t first;
            char32_t last;
            char32_t step;
            int32_t delta;
        };

        const Shift Shifts[] = {
#include "lower.inc"
        };

        const Range* Find(char32_t c)
        {
            if (c > 0xFFFF) return nullptr;
            auto it = std::upper_bound(std::begin(Ranges), std::end(Ranges), c, [](char32_t v, const Range& r) { return v < r.first; });
            if (it == std::begin(Ranges)) return nullptr;
            --it;
            return c <= it->last ? &*it : nullptr;
        }
    }

    bool Space(char32_t c)
    {
        const Range* r = Find(c);
        return r && r->space;
    }

    bool Loose(char32_t c)
    {
        return Find(c) != nullptr;
    }

    char32_t LowerCase(char32_t c)
    {
        auto it = std::upper_bound(std::begin(Shifts), std::end(Shifts), c, [](char32_t v, const Shift& r) { return v < r.first; });
        if (it == std::begin(Shifts)) return c;
        --it;
        if (c > it->last || (c - it->first) % it->step != 0) return c;
        return (char32_t)((int64_t)c + it->delta);
    }
}
