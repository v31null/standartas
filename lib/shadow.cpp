#include "shadow.h"
#include "tree.h"

#include <cwchar>

namespace v31np
{
    namespace
    {
        const char Head[] = "v31np shadow 1";

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
                if (f.size() != 3 || f[0] != Head || !Number(f[1], out.size) || !Number(f[2], out.stamp)) return false;
                first = false;
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
                if (f.size() != 4 || !Text(f[1], r.key) || r.key.empty() || !Number(f[2], r.size) || !Path(f[3], r.file)) return false;
            }
            else if (r.kind == 'D')
            {
                if (f.size() != 2 || !Text(f[1], r.key) || r.key.empty()) return false;
            }
            else if (r.kind == 'R' || r.kind == 'H')
            {
                if (f.size() != 2 || !Path(f[1], r.file)) return false;
            }
            else return false;
            part->push_back(std::move(r));
        }
        return !first;
    }

    std::string WriteJournal(const Shadow& s)
    {
        std::string t = std::string(Head) + "\t" + std::to_string(s.size) + "\t" + std::to_string(s.stamp) + "\r\n";
        for (const auto& [chain, rows] : s.parts)
        {
            t += "@";
            for (const auto& k : chain) t += "\t" + Escape(k);
            t += "\r\n";
            for (const auto& r : rows)
            {
                t += r.kind;
                if (r.kind == 'A') t += "\t" + Escape(r.key) + "\t" + std::to_string(r.size) + "\t" + std::to_string(r.at);
                else if (r.kind == 'D') t += "\t" + Escape(r.key);
                else if (r.kind == 'R' || r.kind == 'H') t += "\t" + Escape(Narrow(r.file));
                else t += "\t" + Escape(r.key) + "\t" + std::to_string(r.size) + "\t" + Escape(Narrow(r.file));
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
}
