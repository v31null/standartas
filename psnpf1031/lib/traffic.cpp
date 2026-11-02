#include "traffic.h"
#include "tree.h"

#include "propertime.h"

#include <algorithm>

namespace v31np
{
    namespace
    {
        void Note(Officer& o, uint64_t ticket, const Papers& p, int64_t now, const char* kind)
        {
            o.log.push_back({ ticket, p, now, kind });
            if (o.log.size() > LogKeep) o.log.erase(o.log.begin(), o.log.end() - (ptrdiff_t)LogKeep);
        }

        std::vector<Car>::iterator Find(Officer& o, uint64_t ticket)
        {
            return std::find_if(o.queue.begin(), o.queue.end(), [&](const Car& c) { return c.ticket == ticket; });
        }

        bool Before(const Car& a, const Car& b)
        {
            if (a.urgent != b.urgent) return a.urgent;
            return a.ticket < b.ticket;
        }

        void Take(Officer& o, const Car& c, int64_t now, int64_t lease)
        {
            o.shift++;
            o.lane = { c.ticket, c.papers, now, now + lease, o.shift };
            o.held = true;
            Note(o, c.ticket, c.papers, now, "wave");
        }

        std::string Text(const Row& r)
        {
            std::string t(1, r.kind);
            t += '\t';
            t += r.key;
            t += '\t' + std::to_string(r.size) + '\t' + std::to_string(r.at) + '\t' + Narrow(r.file);
            return t;
        }

        std::vector<std::string> Level(const Shadow& s, const std::vector<Entry>& stored, const std::string& key)
        {
            std::vector<std::string> rows;
            auto it = s.parts.find(Chain{});
            if (it != s.parts.end())
            {
                for (const auto& r : it->second)
                    if (!r.key.empty() && Under(r.key, key)) rows.push_back(Text(r));
            }
            else
            {
                for (const auto& e : stored)
                    if (Under(e.key, key)) rows.push_back(Text({ 'A', e.key, e.size, e.at, {}, {}, 0 }));
            }
            for (const auto& [chain, part] : s.parts)
            {
                if (chain.empty() || !Under(chain[0], key)) continue;
                std::string head = "@";
                for (const auto& c : chain) head += '\t' + c;
                rows.push_back(head);
                for (const auto& r : part) rows.push_back(Text(r));
            }
            std::sort(rows.begin(), rows.end());
            return rows;
        }

        uint64_t Leading(const std::string& s, size_t at, char stop, bool& ok)
        {
            uint64_t v = 0;
            size_t i = at;
            while (i < s.size() && s[i] >= '0' && s[i] <= '9' && i - at < 19) v = v * 10 + (uint64_t)(s[i++] - '0');
            ok = i > at && i < s.size() && s[i] == stop;
            return v;
        }
    }

    uint64_t TakeTicket(Officer& o, const Papers& me, int64_t now, int64_t lease, bool urgent, const std::vector<std::string>& keys, int64_t clock)
    {
        for (auto& c : o.queue)
        {
            if (c.papers.plate != me.plate) continue;
            c.until = now + lease;
            c.urgent = urgent;
            c.keys = keys;
            c.checked = now;
            c.clock = clock;
            return c.ticket;
        }
        Car c;
        c.ticket = o.next++;
        c.papers = me;
        c.joined = now;
        c.until = now + lease;
        c.urgent = urgent;
        c.seen = o.shift;
        c.checked = now;
        c.clock = clock;
        c.keys = keys;
        o.queue.push_back(std::move(c));
        return o.queue.back().ticket;
    }

    void Withdraw(Officer& o, uint64_t ticket, int64_t now)
    {
        auto it = Find(o, ticket);
        if (it == o.queue.end()) return;
        Note(o, it->ticket, it->papers, now, "withdraw");
        o.queue.erase(it);
    }

    void Sweep(Officer& o, int64_t now)
    {
        if (o.held && o.lane.until < now)
        {
            Note(o, o.lane.ticket, o.lane.papers, now, "expired");
            o.held = false;
            o.shift++;
        }
        for (auto it = o.queue.begin(); it != o.queue.end();)
        {
            if (it->until < now)
            {
                Note(o, it->ticket, it->papers, now, "expired");
                it = o.queue.erase(it);
            }
            else ++it;
        }
    }

    void Sweep(Shadow& s, int64_t now)
    {
        Sweep(s.officer, now);
        for (auto& [chain, rows] : s.parts)
            rows.erase(std::remove_if(rows.begin(), rows.end(), [&](const Row& r) { return r.kind == 'D' && !r.plate.empty() && r.until < now; }), rows.end());
    }

    void Landed(std::vector<Row>& rows, const std::string& key)
    {
        rows.erase(std::remove_if(rows.begin(), rows.end(), [&](const Row& r) { return r.kind == 'D' && Under(key, r.key); }), rows.end());
    }

    void RenewFolders(Shadow& s, const std::string& plate, int64_t now, int64_t lease)
    {
        for (auto& [chain, rows] : s.parts)
            for (auto& r : rows)
                if (r.kind == 'D' && r.plate == plate && r.until >= now) r.until = now + lease;
    }

    Verdict Assess(const Officer& o, uint64_t ticket)
    {
        const Car* me = nullptr;
        for (const auto& c : o.queue)
            if (c.ticket == ticket) me = &c;
        if (!me) return Verdict::Refused;
        auto off = [](const Car& c) { return c.clock - c.checked; };
        auto far = [](int64_t d) { return d > Skew || d < -Skew; };
        const int64_t mine = off(*me);
        if (!far(mine)) return Verdict::Fine;
        bool everyone = true;
        bool alone = true;
        for (const auto& c : o.queue)
        {
            if (c.ticket == ticket) continue;
            alone = false;
            const int64_t d = off(c);
            if (!far(d) || far(d - mine)) everyone = false;
        }
        return alone || everyone ? Verdict::Condemned : Verdict::Refused;
    }

    Verdict Judge(Officer& o, uint64_t ticket, int64_t now)
    {
        const Verdict v = Assess(o, ticket);
        if (v == Verdict::Fine) return v;
        auto me = Find(o, ticket);
        if (me == o.queue.end()) return v;
        const char* kind = v == Verdict::Condemned ? "condemn" : "skew";
        const bool repeat = !o.log.empty() && o.log.back().ticket == ticket && o.log.back().kind == kind;
        if (!repeat) Note(o, me->ticket, me->papers, now, kind);
        return v;
    }

    const Car* NextCar(const Officer& o)
    {
        const Car* best = nullptr;
        for (const auto& c : o.queue)
            if ((!best || Before(c, *best)) && Assess(o, c.ticket) != Verdict::Refused) best = &c;
        return best;
    }

    size_t Ahead(const Officer& o, uint64_t ticket)
    {
        const Car* me = nullptr;
        for (const auto& c : o.queue)
            if (c.ticket == ticket) me = &c;
        if (!me) return 0;
        size_t n = o.held ? 1 : 0;
        for (const auto& c : o.queue)
            if (c.ticket != ticket && Before(c, *me)) n++;
        return n;
    }

    bool WaveOn(Officer& o, uint64_t ticket, int64_t now, int64_t lease)
    {
        if (o.held) return false;
        const Car* next = NextCar(o);
        if (!next || next->ticket != ticket) return false;
        auto it = Find(o, ticket);
        const Car c = *it;
        o.queue.erase(it);
        Take(o, c, now, lease);
        return true;
    }

    bool SameShift(const Officer& o, uint64_t ticket, uint64_t shift)
    {
        return o.held && o.lane.ticket == ticket && o.lane.shift == shift && o.shift == shift;
    }

    bool Renew(Officer& o, uint64_t ticket, uint64_t shift, int64_t now, int64_t lease)
    {
        if (!SameShift(o, ticket, shift) || o.lane.until < now) return false;
        o.lane.until = now + lease;
        return true;
    }

    bool Leave(Officer& o, uint64_t ticket, uint64_t shift, int64_t now)
    {
        if (!SameShift(o, ticket, shift)) return false;
        Note(o, o.lane.ticket, o.lane.papers, now, "leave");
        o.held = false;
        return true;
    }

    bool Override(Officer& o, uint64_t ticket, int64_t now, int64_t lease)
    {
        auto it = Find(o, ticket);
        if (it == o.queue.end()) return false;
        const Car c = *it;
        o.queue.erase(it);
        if (o.held)
        {
            Note(o, o.lane.ticket, o.lane.papers, now, "overridden");
            o.held = false;
        }
        Note(o, c.ticket, c.papers, now, "override");
        Take(o, c, now, lease);
        return true;
    }

    bool Tag(Officer& o, uint64_t by, int64_t now)
    {
        if (!o.held) return false;
        auto it = Find(o, by);
        if (it == o.queue.end()) return false;
        Note(o, it->ticket, it->papers, now, "tag");
        return true;
    }

    std::vector<std::string> Changed(const Shadow& before, const Shadow& now, const std::vector<Entry>& stored, const std::vector<std::string>& keys)
    {
        std::vector<std::string> out;
        for (const auto& k : keys)
            if (Level(before, stored, k) != Level(now, stored, k)) out.push_back(k);
        return out;
    }

    Custody NextCustody(const std::vector<std::string>& keys, const std::string& plate, const std::string& year)
    {
        const std::string root = "ALICEV31NP/";
        const std::string mine = root + plate + "\\";
        const std::string yearly = root + year + "\\";
        Custody c;
        for (const auto& k : keys)
        {
            bool ok = false;
            if (k.size() > mine.size() && k.compare(0, mine.size(), mine) == 0)
            {
                uint64_t n = Leading(k, mine.size(), '.', ok);
                if (ok && k.compare(k.size() - 4, 4, ".txt") == 0 && n > c.action) c.action = n;
            }
            if (k.size() > yearly.size() && k.compare(0, yearly.size(), yearly) == 0)
            {
                uint64_t n = Leading(k, yearly.size(), '-', ok);
                if (ok && n > c.list) c.list = n;
            }
        }
        c.action++;
        c.list++;
        c.actionKey = mine + std::to_string(c.action) + ".txt";
        c.listKey = yearly + std::to_string(c.list) + "-" + std::to_string(c.action) + ".txt";
        return c;
    }

    namespace
    {
        std::string Escape(const std::string& s)
        {
            std::string r;
            for (char c : s)
            {
                if (c == '\\') r += "\\\\";
                else if (c == '\t') r += "\\t";
                else if (c == '\n') r += "\\n";
                else if (c == '\r') r += "\\r";
                else r += c;
            }
            return r;
        }

        bool Unescape(const std::string& s, std::string& r)
        {
            r.clear();
            for (size_t i = 0; i < s.size(); i++)
            {
                if (s[i] != '\\')
                {
                    r += s[i];
                    continue;
                }
                if (++i >= s.size()) return false;
                if (s[i] == '\\') r += '\\';
                else if (s[i] == 't') r += '\t';
                else if (s[i] == 'n') r += '\n';
                else if (s[i] == 'r') r += '\r';
                else return false;
            }
            return ValidUtf8(r);
        }

        bool Decimal(const std::string& s, uint64_t& v)
        {
            if (s.empty() || s.size() > 20) return false;
            v = 0;
            for (char c : s)
            {
                if (c < '0' || c > '9') return false;
                const uint64_t d = (uint64_t)(c - '0');
                if (v > (UINT64_MAX - d) / 10) return false;
                v = v * 10 + d;
            }
            return true;
        }

        const char* const Actions[] = { "add", "copy", "move", "rename", "delete", "override", "citation" };
    }

    std::string CustodyText(const Record& r)
    {
        std::string t;
        auto line = [&](const std::string& s) { t += s + "\r\n"; };
        line(Escape(r.papers.plate));
        line(Escape(r.papers.os));
        line(Stamp(r.time));
        line(std::to_string(r.shift));
        line(std::to_string(r.ticket));
        line(Escape(r.action));
        for (const auto& l : r.lines) line(Escape(l));
        return t;
    }

    bool ReadCustody(const std::string& text, Record& r)
    {
        r = Record{};
        if (text.size() < 2 || text.compare(text.size() - 2, 2, "\r\n") != 0) return false;
        std::vector<std::string> raw;
        size_t at = 0;
        while (at < text.size())
        {
            size_t end = text.find("\r\n", at);
            if (end == std::string::npos) return false;
            raw.push_back(text.substr(at, end - at));
            at = end + 2;
        }
        if (raw.size() < 6) return false;
        for (const auto& l : raw)
            if (l.find('\r') != std::string::npos || l.find('\n') != std::string::npos) return false;
        if (!Unescape(raw[0], r.papers.plate) || r.papers.plate.empty() || !Unescape(raw[1], r.papers.os)) return false;
        if (!Unstamp(raw[2], r.time) || !Decimal(raw[3], r.shift) || !Decimal(raw[4], r.ticket) || !Unescape(raw[5], r.action)) return false;
        if (std::find(std::begin(Actions), std::end(Actions), r.action) == std::end(Actions)) return false;
        for (size_t i = 6; i < raw.size(); i++)
        {
            std::string l;
            if (!Unescape(raw[i], l)) return false;
            r.lines.push_back(std::move(l));
        }
        return true;
    }

    Record Citation(const Car& car, uint64_t shift, int64_t now, bool everyone)
    {
        const int64_t d = car.clock - car.checked;
        const int64_t n = d < 0 ? -d : d;
        Record r;
        r.papers = car.papers;
        r.time = now;
        r.shift = shift;
        r.ticket = car.ticket;
        r.action = "citation";
        r.lines.push_back(std::string("WARNING - CLOCK ") + (d > 0 ? "AHEAD " : "BEHIND ") + std::to_string(n) + "s" + (everyone ? " ALL" : ""));
        return r;
    }
}
