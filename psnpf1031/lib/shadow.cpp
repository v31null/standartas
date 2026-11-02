#include "shadow.h"
#include "officer.h"
#include "propertime.h"
#include "tree.h"

#include <algorithm>
#include <cwchar>

#ifndef _WIN32
#define _wcsicmp wcscasecmp
#endif

namespace v31np
{
    namespace
    {
        const char Head1[] = "v31np shadow 1";
        const char Head2[] = "v31np shadow 2";

        std::string Escape(const std::string& s)
        {
            std::string r;
            r.reserve(s.size());
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
            return true;
        }

        std::vector<std::string> Fields(const std::string& line)
        {
            std::vector<std::string> out;
            size_t at = 0;
            for (;;)
            {
                size_t tab = line.find('\t', at);
                out.push_back(line.substr(at, tab == std::string::npos ? std::string::npos : tab - at));
                if (tab == std::string::npos) return out;
                at = tab + 1;
            }
        }

        bool Number(const std::string& s, uint64_t& v)
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

        bool Text(const std::string& s, std::string& out)
        {
            return Unescape(s, out) && ValidUtf8(out);
        }

        bool Path(const std::string& s, std::wstring& out)
        {
            std::string t;
            if (!Text(s, t) || t.empty()) return false;
            out = Wide(t);
            return true;
        }

        bool Time(const std::string& s, int64_t& out)
        {
            return Unstamp(s, out);
        }

        bool Flag(const std::string& s, bool& out)
        {
            if (s != "0" && s != "1") return false;
            out = s == "1";
            return true;
        }

        bool Plate(const std::vector<std::string>& f, size_t at, Papers& p)
        {
            return Text(f[at], p.plate) && !p.plate.empty() && Text(f[at + 1], p.os);
        }

        bool OfficerLine(const std::vector<std::string>& f, Officer& o, std::string& seen)
        {
            const std::string& k = f[0];
            if (k.size() != 1 || (k != "Q" && k != "W" && seen.find(k) != std::string::npos)) return false;
            seen += k;
            if (k == "G") return f.size() == 2 && Number(f[1], o.shift);
            if (k == "N") return f.size() == 2 && Number(f[1], o.next) && o.next > 0;
            if (k == "S") return f.size() == 2 && Flag(f[1], o.shared);
            if (k == "L")
            {
                Lane& l = o.lane;
                o.held = true;
                return f.size() == 7 && Number(f[1], l.ticket) && Plate(f, 2, l.papers) && Time(f[4], l.granted) && Time(f[5], l.until) && Number(f[6], l.shift);
            }
            if (k == "Q")
            {
                Car c;
                if (f.size() < 10 || !Number(f[1], c.ticket) || !Plate(f, 2, c.papers) || !Time(f[4], c.joined) || !Time(f[5], c.until) || !Flag(f[6], c.urgent) || !Number(f[7], c.seen) || !Time(f[8], c.checked) || !Time(f[9], c.clock)) return false;
                for (size_t i = 10; i < f.size(); i++)
                {
                    std::string key;
                    if (!Text(f[i], key) || key.empty()) return false;
                    c.keys.push_back(std::move(key));
                }
                o.queue.push_back(std::move(c));
                return true;
            }
            if (k == "W")
            {
                Event e;
                if (f.size() != 6 || !Number(f[1], e.ticket) || !Plate(f, 2, e.papers) || !Time(f[4], e.time) || !Text(f[5], e.kind) || e.kind.empty()) return false;
                o.log.push_back(std::move(e));
                return true;
            }
            return false;
        }

        std::string PapersText(const Papers& p)
        {
            return Escape(p.plate) + "\t" + Escape(p.os);
        }
    }

    std::wstring JournalName(const std::wstring& archiveName)
    {
        std::wstring stem = archiveName;
        if (stem.size() > 6 && _wcsicmp(stem.c_str() + stem.size() - 6, L".v31np") == 0) stem.resize(stem.size() - 6);
        return L"FUTALUZTU UERSIIA UUTI " + stem + L".v31np.txt";
    }

    bool ReadJournal(const std::string& text, Shadow& out)
    {
        out = Shadow{};
        std::vector<Row>* part = nullptr;
        size_t at = 0;
        bool first = true;
        std::string seen;
        while (at < text.size())
        {
            size_t end = text.find('\n', at);
            if (end == std::string::npos) end = text.size();
            std::string line = text.substr(at, end - at);
            at = end + 1;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            const auto f = Fields(line);
            if (first)
            {
                if (f.size() != 3 || (f[0] != Head1 && f[0] != Head2) || !Number(f[1], out.size) || !Number(f[2], out.stamp)) return false;
                out.version = f[0] == Head1 ? 1 : 2;
                first = false;
                continue;
            }
            if (!part && out.version == 2 && f[0] != "@")
            {
                if (!OfficerLine(f, out.officer, seen)) return false;
                continue;
            }
            if (f[0] == "@")
            {
                Chain chain;
                for (size_t i = 1; i < f.size(); i++)
                {
                    std::string k;
                    if (!Text(f[i], k) || k.empty()) return false;
                    chain.push_back(std::move(k));
                }
                if (out.parts.count(chain)) return false;
                part = &out.parts[chain];
                continue;
            }
            if (!part || f[0].size() != 1) return false;
            Row r;
            r.kind = f[0][0];
            if (r.kind == 'A')
            {
                if (f.size() != 4 || !Text(f[1], r.key) || r.key.empty() || !Number(f[2], r.size) || !Number(f[3], r.at)) return false;
            }
            else if (r.kind == 'F' || r.kind == 'M' || r.kind == 'T')
            {
                const size_t want = out.version == 1 ? 4 : 5;
                if (f.size() != want || !Text(f[1], r.key) || r.key.empty() || !Number(f[2], r.size) || !Path(f[3], r.file)) return false;
                if (want == 5 && (!Text(f[4], r.plate) || r.plate.empty())) return false;
            }
            else if (r.kind == 'D')
            {
                if (f.size() == 4 && out.version == 2)
                {
                    if (!Text(f[1], r.key) || r.key.empty() || !Text(f[2], r.plate) || r.plate.empty() || !Time(f[3], r.until)) return false;
                }
                else if (f.size() != 2 || !Text(f[1], r.key) || r.key.empty()) return false;
            }
            else if (r.kind == 'R' || r.kind == 'H')
            {
                const size_t want = out.version == 1 ? 2 : 3;
                if (f.size() != want || !Path(f[1], r.file)) return false;
                if (want == 3 && (!Text(f[2], r.plate) || r.plate.empty())) return false;
            }
            else return false;
            part->push_back(std::move(r));
        }
        return !first;
    }

    std::string WriteJournal(const Shadow& s)
    {
        std::string t = std::string(s.version == 1 ? Head1 : Head2) + "\t" + std::to_string(s.size) + "\t" + std::to_string(s.stamp) + "\r\n";
        if (s.version != 1)
        {
            const Officer& o = s.officer;
            t += "G\t" + std::to_string(o.shift) + "\r\n";
            t += "N\t" + std::to_string(o.next) + "\r\n";
            t += std::string("S\t") + (o.shared ? "1" : "0") + "\r\n";
            if (o.held)
            {
                const Lane& l = o.lane;
                t += "L\t" + std::to_string(l.ticket) + "\t" + PapersText(l.papers) + "\t" + Stamp(l.granted) + "\t" + Stamp(l.until) + "\t" + std::to_string(l.shift) + "\r\n";
            }
            for (const auto& c : o.queue)
            {
                t += "Q\t" + std::to_string(c.ticket) + "\t" + PapersText(c.papers) + "\t" + Stamp(c.joined) + "\t" + Stamp(c.until) + "\t" + (c.urgent ? "1" : "0") + "\t" + std::to_string(c.seen) + "\t" + Stamp(c.checked) + "\t" + Stamp(c.clock);
                for (const auto& k : c.keys) t += "\t" + Escape(k);
                t += "\r\n";
            }
            for (const auto& e : o.log)
                t += "W\t" + std::to_string(e.ticket) + "\t" + PapersText(e.papers) + "\t" + Stamp(e.time) + "\t" + Escape(e.kind) + "\r\n";
        }
        for (const auto& [chain, rows] : s.parts)
        {
            t += "@";
            for (const auto& k : chain) t += "\t" + Escape(k);
            t += "\r\n";
            for (const auto& r : rows)
            {
                t += r.kind;
                if (r.kind == 'A') t += "\t" + Escape(r.key) + "\t" + std::to_string(r.size) + "\t" + std::to_string(r.at);
                else if (r.kind == 'D')
                {
                    t += "\t" + Escape(r.key);
                    if (!r.plate.empty() && s.version != 1) t += "\t" + Escape(r.plate) + "\t" + Stamp(r.until);
                }
                else if (r.kind == 'R' || r.kind == 'H') t += "\t" + Escape(Narrow(r.file));
                else t += "\t" + Escape(r.key) + "\t" + std::to_string(r.size) + "\t" + Escape(Narrow(r.file));
                if (s.version != 1 && r.kind != 'A' && r.kind != 'D') t += "\t" + Escape(r.plate.empty() ? Plate() : r.plate);
                t += "\r\n";
            }
        }
        return t;
    }

    bool Under(const std::string& key, const std::string& prefix)
    {
        if (prefix.empty()) return true;
        if (key.size() < prefix.size() || key.compare(0, prefix.size(), prefix) != 0) return false;
        return key.size() == prefix.size() || key[prefix.size()] == '/';
    }

    bool Stored(const Row& row)
    {
        return row.kind == 'A' || row.kind == 'F' || row.kind == 'M' || row.kind == 'T';
    }

    uint64_t Logical(const Shadow& s, const Chain& chain, const Row& row)
    {
        Chain deeper = chain;
        deeper.push_back(row.key);
        auto it = s.parts.find(deeper);
        if (it == s.parts.end()) return row.size;
        std::vector<Item> items;
        for (const auto& r : it->second)
            if (Stored(r)) items.push_back({ r.key, Logical(s, deeper, r), {} });
        return Measure(items);
    }

    std::vector<Row> Listing(const std::vector<Entry>& entries, const std::vector<std::string>& dirs)
    {
        std::vector<Row> rows;
        for (const auto& e : entries) rows.push_back({ 'A', e.key, e.size, e.at, {}, {}, 0 });
        std::vector<std::string> empty;
        for (const auto& d : dirs)
        {
            if (d.empty()) continue;
            bool holds = false;
            for (const auto& e : entries)
                if (e.key != d && Under(e.key, d)) { holds = true; break; }
            if (!holds) empty.push_back(d);
        }
        std::sort(empty.begin(), empty.end());
        empty.erase(std::unique(empty.begin(), empty.end()), empty.end());
        for (const auto& d : empty) rows.push_back({ 'D', d, 0, 0, {}, {}, 0 });
        return rows;
    }

    bool Ours(const Row& row, const std::string& plate)
    {
        if (row.kind == 'A' || row.kind == 'D') return true;
        return row.plate.empty() || row.plate == plate;
    }

    bool AnyForeign(const Shadow& s, const std::string& plate)
    {
        for (const auto& part : s.parts)
            for (const auto& r : part.second)
                if (!Ours(r, plate)) return true;
        return false;
    }
}
