#include "v31np.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace v31np
{
    namespace
    {
        using Bytes = std::vector<uint8_t>;

        const uint8_t Magic[] = { 'V', '3', '1', 'N', 'S', 'P', 'A', 'C', 'K', 'F', 'O', 'R', 'M', 'A', 'T' };
        const uint8_t Pi[] = { 0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3, 0x13, 0x19, 0x8A, 0x2E };
        const uint8_t Phi[] = { 0x9E, 0x37, 0x79, 0xB9, 0x7F, 0x4A, 0x7C, 0x15, 0xF3, 0x9C, 0xC0, 0x60 };
        constexpr size_t Mark = sizeof(Pi);
        constexpr size_t Lead = sizeof(Magic) + Mark;
        constexpr size_t Foot = Mark + 4;
        constexpr uint32_t Block = 1u << 20;

        using Tables = std::array<std::array<uint32_t, 256>, 16>;

        const Tables& Table()
        {
            static const Tables t = []
            {
                Tables r{};
                for (uint32_t n = 0; n < 256; n++)
                {
                    uint32_t c = n;
                    for (int k = 0; k < 8; k++) c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
                    r[0][n] = c;
                }
                for (size_t k = 1; k < 16; k++)
                    for (uint32_t n = 0; n < 256; n++) r[k][n] = (r[k - 1][n] >> 8) ^ r[0][r[k - 1][n] & 0xFF];
                return r;
            }();
            return t;
        }

        uint32_t Load32(const uint8_t* p)
        {
            uint32_t v;
            std::memcpy(&v, p, 4);
            return v;
        }

        ptrdiff_t Find(const Bytes& buf, const uint8_t* pat, size_t from, size_t to)
        {
            if (to > buf.size()) to = buf.size();
            if (from >= to || to - from < Mark) return -1;
            auto first = buf.begin() + (ptrdiff_t)from;
            auto last = buf.begin() + (ptrdiff_t)to;
            auto hit = std::search(first, last, std::boyer_moore_horspool_searcher(pat, pat + Mark));
            return hit == last ? -1 : hit - buf.begin();
        }

        bool Rows(const Bytes& buf, size_t from, size_t to, std::vector<std::vector<Bytes>>& out)
        {
            out.clear();
            bool open = false;
            size_t at = from;
            for (;;)
            {
                ptrdiff_t hit = Find(buf, Phi, at, to);
                if (hit >= 0 && (size_t)hit <= at) return false;
                size_t end = hit < 0 ? to : (size_t)hit - 1;
                if (open) out.back().emplace_back(buf.begin() + (ptrdiff_t)at, buf.begin() + (ptrdiff_t)end);
                else if (end != from) return false;
                if (hit < 0) return true;
                uint8_t marker = buf[(size_t)hit - 1];
                if (marker == 0)
                {
                    out.emplace_back();
                    open = true;
                }
                else if (marker != 1 || !open) return false;
                at = (size_t)hit + Mark;
            }
        }

        bool Num(const Bytes& b, uint64_t& v)
        {
            if (b.empty() || b.size() > 7) return false;
            v = 0;
            for (size_t i = b.size(); i-- > 0;) v = (v << 8) | b[i];
            return v < Limit;
        }

        void Put(Bytes& out, const uint8_t* p, size_t n)
        {
            out.insert(out.end(), p, p + n);
        }

        void Put(Bytes& out, const std::string& s)
        {
            out.insert(out.end(), s.begin(), s.end());
        }

        void PutNum(Bytes& out, uint64_t v)
        {
            do
            {
                out.push_back((uint8_t)(v & 0xFF));
                v >>= 8;
            } while (v > 0);
        }

        void PutSep(Bytes& out, uint8_t marker)
        {
            out.push_back(marker);
            Put(out, Phi, Mark);
        }

        std::array<std::string, 3> Split(const std::string& key)
        {
            const size_t slash = key.rfind('/');
            const size_t base = slash == std::string::npos ? 0 : slash + 1;
            const size_t dot = key.rfind('.');
            const size_t ext = dot == std::string::npos || dot < base ? key.size() : dot;
            return { key.substr(0, base), key.substr(base, ext - base), key.substr(ext) };
        }
    }

    uint32_t Crc32(const void* data, size_t len, uint32_t crc)
    {
        const auto& t = Table();
        const uint8_t* p = (const uint8_t*)data;
        uint32_t c = ~crc;
        for (; len >= 16; p += 16, len -= 16)
        {
            const uint32_t a = Load32(p) ^ c;
            const uint32_t b = Load32(p + 4);
            const uint32_t d = Load32(p + 8);
            const uint32_t e = Load32(p + 12);
            c = t[15][a & 0xFF] ^ t[14][(a >> 8) & 0xFF] ^ t[13][(a >> 16) & 0xFF] ^ t[12][a >> 24]
                ^ t[11][b & 0xFF] ^ t[10][(b >> 8) & 0xFF] ^ t[9][(b >> 16) & 0xFF] ^ t[8][b >> 24]
                ^ t[7][d & 0xFF] ^ t[6][(d >> 8) & 0xFF] ^ t[5][(d >> 16) & 0xFF] ^ t[4][d >> 24]
                ^ t[3][e & 0xFF] ^ t[2][(e >> 8) & 0xFF] ^ t[1][(e >> 16) & 0xFF] ^ t[0][e >> 24];
        }
        for (; len > 0; p++, len--) c = t[0][(c ^ *p) & 0xFF] ^ (c >> 8);
        return ~c;
    }

    bool ValidUtf8(const std::string& s)
    {
        const uint8_t* p = (const uint8_t*)s.data();
        size_t n = s.size();
        size_t i = 0;
        while (i < n)
        {
            uint8_t c = p[i];
            if (c < 0x80)
            {
                i++;
                continue;
            }
            size_t len;
            uint32_t cp;
            if (c >= 0xC2 && c <= 0xDF)
            {
                len = 2;
                cp = c & 0x1F;
            }
            else if (c >= 0xE0 && c <= 0xEF)
            {
                len = 3;
                cp = c & 0x0F;
            }
            else if (c >= 0xF0 && c <= 0xF4)
            {
                len = 4;
                cp = c & 0x07;
            }
            else return false;
            if (n - i < len) return false;
            for (size_t k = 1; k < len; k++)
            {
                if ((p[i + k] & 0xC0) != 0x80) return false;
                cp = (cp << 6) | (p[i + k] & 0x3F);
            }
            if (len == 3 && (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF))) return false;
            if (len == 4 && (cp < 0x10000 || cp > 0x10FFFF)) return false;
            i += len;
        }
        return true;
    }

    bool Parse(const ReadAt& read, uint64_t total, std::vector<Entry>& out)
    {
        out.clear();
        if (total < Lead + 2 * Mark + Foot || total >= Limit) return false;
        const uint64_t body = total - Foot;
        Bytes head;
        ptrdiff_t b = -1;
        ptrdiff_t c = -1;
        size_t scan = Lead;
        while (c < 0)
        {
            size_t have = head.size();
            if (have >= body || have >= HeadCap) return false;
            uint32_t len = (uint32_t)std::min<uint64_t>(Block, body - have);
            head.resize(have + len);
            if (!read(have, head.data() + have, len)) return false;
            if (have == 0 && (!std::equal(Magic, Magic + sizeof(Magic), head.begin()) || !std::equal(Pi, Pi + Mark, head.begin() + sizeof(Magic)))) return false;
            if (b < 0)
            {
                b = Find(head, Pi, scan, head.size());
                if (b < 0)
                {
                    scan = std::max(Lead, head.size() - Mark + 1);
                    continue;
                }
                scan = (size_t)b + Mark;
            }
            c = Find(head, Pi, scan, head.size());
            if (c < 0) scan = std::max(scan, head.size() - Mark + 1);
        }
        const size_t headerLen = (size_t)c + Mark;

        std::vector<std::vector<Bytes>> t1;
        std::vector<std::vector<Bytes>> t2;
        if (!Rows(head, Lead, (size_t)b, t1) || !Rows(head, (size_t)b + Mark, (size_t)c, t2) || t1.size() != t2.size()) return false;

        std::unordered_map<std::string, uint64_t> sizes;
        for (const auto& r : t2)
        {
            if (r.size() != 5) return false;
            std::string key(r[0].begin(), r[0].end());
            uint64_t size;
            if (!ValidUtf8(key) || !Num(r[1], size) || sizes.count(key)) return false;
            const auto parts = Split(key);
            for (size_t k = 0; k < 3; k++)
                if (std::string(r[k + 2].begin(), r[k + 2].end()) != parts[k]) return false;
            sizes.emplace(std::move(key), size);
        }

        uint64_t next = headerLen;
        out.reserve(t1.size());
        for (const auto& r : t1)
        {
            if (r.size() != 2) return false;
            std::string key(r[0].begin(), r[0].end());
            uint64_t at;
            if (!Num(r[1], at) || at != next) return false;
            auto it = sizes.find(key);
            if (it == sizes.end() || it->second > body - next) return false;
            next += it->second;
            out.push_back({ std::move(key), at, it->second });
            sizes.erase(it);
        }
        if (next != body)
        {
            out.clear();
            return false;
        }

        uint8_t foot[Foot];
        if (!read(body, foot, (uint32_t)Foot) || !std::equal(Pi, Pi + Mark, foot))
        {
            out.clear();
            return false;
        }
        return true;
    }

    bool Verify(const ReadAt& read, uint64_t total)
    {
        if (total < Foot) return false;
        const uint64_t body = total - Foot;
        uint32_t crc = 0;
        Bytes buf(Block);
        for (uint64_t pos = 0; pos < body;)
        {
            uint32_t len = (uint32_t)std::min<uint64_t>(Block, body - pos);
            if (!read(pos, buf.data(), len)) return false;
            crc = Crc32(buf.data(), len, crc);
            pos += len;
        }
        uint8_t foot[Foot];
        if (!read(body, foot, (uint32_t)Foot) || !std::equal(Pi, Pi + Mark, foot)) return false;
        crc = Crc32(Pi, Mark, crc);
        uint32_t want = (uint32_t)foot[Mark] | ((uint32_t)foot[Mark + 1] << 8) | ((uint32_t)foot[Mark + 2] << 16) | ((uint32_t)foot[Mark + 3] << 24);
        return crc == want;
    }

    bool Write(const std::vector<Item>& items, const Push& push)
    {
        std::unordered_set<std::string> seen;
        uint64_t sum = 0;
        for (const auto& it : items)
        {
            if (!ValidUtf8(it.key) || !seen.insert(it.key).second || it.size >= Limit - sum) return false;
            sum += it.size;
        }

        Bytes t2;
        for (const auto& it : items)
        {
            const auto parts = Split(it.key);
            PutSep(t2, 0);
            Put(t2, it.key);
            PutSep(t2, 1);
            PutNum(t2, it.size);
            for (const auto& p : parts)
            {
                PutSep(t2, 1);
                Put(t2, p);
            }
        }

        Bytes header;
        uint64_t start = 0;
        uint64_t previous;
        do
        {
            previous = start;
            header.clear();
            Put(header, Magic, sizeof(Magic));
            Put(header, Pi, Mark);
            uint64_t address = previous;
            for (const auto& it : items)
            {
                PutSep(header, 0);
                Put(header, it.key);
                PutSep(header, 1);
                PutNum(header, address);
                address += it.size;
            }
            Put(header, Pi, Mark);
            Put(header, t2.data(), t2.size());
            Put(header, Pi, Mark);
            start = header.size();
        } while (start != previous);

        if (sum >= Limit - start - Foot) return false;

        uint32_t crc = Crc32(header.data(), header.size(), 0);
        for (size_t at = 0; at < header.size(); at += Block)
        {
            uint32_t len = (uint32_t)std::min<size_t>(Block, header.size() - at);
            if (!push(header.data() + at, len)) return false;
        }
        header.clear();
        header.shrink_to_fit();

        Bytes buf;
        for (const auto& it : items)
        {
            if (it.size > 0 && buf.empty()) buf.resize(Block);
            for (uint64_t left = it.size; left > 0;)
            {
                uint32_t len = (uint32_t)std::min<uint64_t>(Block, left);
                if (!it.pull(buf.data(), len)) return false;
                crc = Crc32(buf.data(), len, crc);
                if (!push(buf.data(), len)) return false;
                left -= len;
            }
        }

        crc = Crc32(Pi, Mark, crc);
        uint8_t foot[Foot];
        std::copy(Pi, Pi + Mark, foot);
        foot[Mark] = (uint8_t)(crc & 0xFF);
        foot[Mark + 1] = (uint8_t)((crc >> 8) & 0xFF);
        foot[Mark + 2] = (uint8_t)((crc >> 16) & 0xFF);
        foot[Mark + 3] = (uint8_t)((crc >> 24) & 0xFF);
        return push(foot, (uint32_t)Foot);
    }
}
