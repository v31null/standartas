#include "road.h"

#include "propertime.h"

#include <algorithm>
#include <set>

namespace v31np
{
    namespace
    {
        class Light
        {
        public:
            explicit Light(Road& road) : road(road) {}
            ~Light()
            {
                if (green) road.Red();
            }
            bool Take()
            {
                green = road.Green(SignalWait);
                return green;
            }

        private:
            Road& road;
            bool green = false;
        };

        bool Clean(const Shadow& s)
        {
            for (const auto& [chain, rows] : s.parts)
                for (const auto& r : rows)
                    if (r.kind != 'A' && r.kind != 'D') return false;
            return true;
        }

        bool Load(Road& road, Shadow& s)
        {
            uint64_t size = 0;
            uint64_t stamp = 0;
            if (!road.Archive(size, stamp)) return false;
            std::string text;
            bool exists = false;
            if (!road.ReadJournal(text, exists)) return false;
            Shadow j;
            if (exists && ReadJournal(text, j) && Clean(j))
            {
                if (j.size != size || j.stamp != stamp) j.parts.clear();
                s = std::move(j);
            }
            else s = Shadow{};
            s.version = 2;
            s.size = size;
            s.stamp = stamp;
            s.officer.shared = true;
            return true;
        }

        const Car* Mine(const Officer& o, uint64_t ticket)
        {
            for (const auto& c : o.queue)
                if (c.ticket == ticket) return &c;
            return nullptr;
        }

        bool Stuck(const Officer& o, int64_t now)
        {
            return o.held && now - (o.lane.until - Lease) >= Missed;
        }

        size_t Machines(const Officer& o)
        {
            std::set<std::string> plates;
            if (o.held) plates.insert(o.lane.papers.plate);
            for (const auto& c : o.queue) plates.insert(c.papers.plate);
            return plates.size();
        }

        std::string Escaped(const std::string& s)
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
    }

    bool Visit(Road& road, const Papers& me, const std::function<void(Shadow&, int64_t)>& business)
    {
        Light light(road);
        if (!light.Take()) return false;
        if (!road.ShowPapers(PapersLine(me))) return false;
        int64_t now = 0;
        if (!road.ServerNow(now)) return false;
        Shadow s;
        if (!Load(road, s)) return false;
        Sweep(s, now);
        business(s, now);
        return road.WriteJournal(WriteJournal(s));
    }

    Drove Drive(Road& road, Trip& trip, const Request& request)
    {
        const Papers& me = request.me;
        uint64_t ticket = 0;
        uint64_t shift = 0;
        int64_t wavedAt = 0;
        bool waved = false;
        bool overrode = false;
        bool skewed = false;
        bool skewing = false;
        int64_t skewSince = 0;
        bool wantOverride = false;
        uint64_t overrideShift = 0;
        bool asked = false;
        uint64_t askedShift = 0;
        bool tagged = false;
        uint64_t taggedShift = 0;
        Car car;
        bool citation = false;
        bool everyone = false;
        Papers holder;
        Shadow seen;

        for (;;)
        {
            const bool cancel = road.Cancelled();
            bool ask = false;
            Papers askHolder;
            uint64_t askShift = 0;
            const bool visited = Visit(road, me, [&](Shadow& s, int64_t now)
            {
                Officer& o = s.officer;
                if (cancel)
                {
                    if (ticket) Withdraw(o, ticket, now);
                    return;
                }
                ticket = TakeTicket(o, me, now, Lease, request.urgent, request.keys, road.Clock());
                const Verdict v = Judge(o, ticket, now);
                if (v == Verdict::Refused)
                {
                    if (!skewing)
                    {
                        skewing = true;
                        skewSince = now;
                    }
                    else if (now - skewSince >= Lease)
                    {
                        Withdraw(o, ticket, now);
                        skewed = true;
                        return;
                    }
                }
                else skewing = false;
                const Car* c = Mine(o, ticket);
                if (c) car = *c;
                const bool alone = o.queue.size() == 1;
                if (wantOverride && o.held && o.lane.shift == overrideShift && Stuck(o, now) && v != Verdict::Refused)
                {
                    holder = o.lane.papers;
                    overrode = Override(o, ticket, now, Lease);
                    waved = overrode;
                }
                else waved = WaveOn(o, ticket, now, Lease);
                wantOverride = false;
                if (waved)
                {
                    shift = o.lane.shift;
                    wavedAt = now;
                    citation = v == Verdict::Condemned;
                    everyone = citation && !alone;
                    seen = s;
                    return;
                }
                if (!Stuck(o, now) || v == Verdict::Refused) return;
                if (Machines(o) <= 2 && o.queue.size() == 1)
                {
                    if (!asked || askedShift != o.lane.shift)
                    {
                        ask = true;
                        askHolder = o.lane.papers;
                        askShift = o.lane.shift;
                    }
                }
                else if (!tagged || taggedShift != o.lane.shift)
                {
                    if (Tag(o, ticket, now))
                    {
                        tagged = true;
                        taggedShift = o.lane.shift;
                    }
                }
            });
            if (!visited) return Drove::Failed;
            if (cancel) return Drove::Cancelled;
            if (skewed) return Drove::Skewed;
            if (waved) break;
            if (ask)
            {
                asked = true;
                askedShift = askShift;
                if (road.Ask(askHolder))
                {
                    wantOverride = true;
                    overrideShift = askShift;
                    continue;
                }
            }
            road.Sleep(Poll);
        }

        auto leave = [&]()
        {
            Visit(road, me, [&](Shadow& s, int64_t now) { Leave(s.officer, ticket, shift, now); });
        };

        std::vector<Record> records;
        if (citation) records.push_back(Citation(car, shift, wavedAt, everyone));
        if (overrode) records.push_back({ me, wavedAt, shift, ticket, "override", { holder.plate, holder.os } });
        for (const auto& step : request.steps) records.push_back({ me, wavedAt, shift, ticket, step.action, step.lines });

        if (!trip.Look(seen))
        {
            leave();
            return Drove::Refused;
        }

        uint64_t renewed = road.Ticks();
        bool lost = false;
        const std::function<bool()> renew = [&]() -> bool
        {
            if (lost) return false;
            if (road.Ticks() - renewed < (uint64_t)Renewal * 1000) return true;
            bool kept = true;
            if (!Visit(road, me, [&](Shadow& s, int64_t now) { kept = Renew(s.officer, ticket, shift, now, Lease); })) return true;
            if (!kept)
            {
                lost = true;
                return false;
            }
            renewed = road.Ticks();
            return true;
        };

        if (!trip.Build(records, renew))
        {
            trip.Abandon();
            if (lost) return Drove::Lost;
            leave();
            return Drove::Failed;
        }

        bool same = false;
        bool landed = false;
        const bool visited = Visit(road, me, [&](Shadow& s, int64_t now)
        {
            if (!SameShift(s.officer, ticket, shift)) return;
            same = true;
            Shadow next = s;
            landed = trip.Land(next, now);
            if (landed)
            {
                Officer o = std::move(s.officer);
                s = std::move(next);
                s.officer = std::move(o);
                for (auto& [chain, rows] : s.parts)
                    for (auto& r : rows)
                        if (r.kind == 'D' && r.plate.empty())
                        {
                            r.plate = me.plate;
                            r.until = now + Lease;
                        }
            }
            Leave(s.officer, ticket, shift, now);
        });
        if (landed) return Drove::Done;
        trip.Abandon();
        if (!visited) return Drove::Failed;
        return same ? Drove::Failed : Drove::Lost;
    }

    std::string PapersLine(const Papers& p)
    {
        return "plate=" + p.plate + " os=" + p.os;
    }

    std::string Year(int64_t officerTime)
    {
        return Stamp(officerTime).substr(0, 4);
    }

    std::vector<std::pair<std::string, std::string>> CustodyFiles(std::vector<std::string> levelKeys, const std::vector<Record>& records)
    {
        std::vector<std::pair<std::string, std::string>> out;
        for (const auto& r : records)
        {
            const Custody c = NextCustody(levelKeys, r.papers.plate, Year(r.time));
            const std::string text = CustodyText(r);
            out.push_back({ c.actionKey, text });
            out.push_back({ c.listKey, Escaped(c.actionKey) + "\r\n" + text });
            levelKeys.push_back(c.actionKey);
            levelKeys.push_back(c.listKey);
        }
        return out;
    }
}
