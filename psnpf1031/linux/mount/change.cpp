#include "change.h"

#include <algorithm>

namespace change
{
    namespace
    {
        bool Keyed(const v31np::Row& r)
        {
            return r.kind == 'A' || r.kind == 'F' || r.kind == 'M' || r.kind == 'T' || r.kind == 'D';
        }

        bool Starts(const Path& whole, const Path& head)
        {
            return whole.size() >= head.size() && std::equal(head.begin(), head.end(), whole.begin());
        }

        bool Above(const std::string& key, const Path& path)
        {
            const Path p = Split(key);
            return p.size() < path.size() && Starts(path, p);
        }

        void LeaveParent(std::vector<v31np::Row>& rows, const Path& path)
        {
            if (path.size() < 2) return;
            const Path parent(path.begin(), path.end() - 1);
            for (const auto& r : rows)
                if (Keyed(r) && Within(r.key, parent)) return;
            rows.push_back({ 'D', Join(parent), 0, 0, {}, {}, 0 });
        }
    }

    Path Split(const std::string& key)
    {
        Path out;
        size_t i = 0;
        while (i <= key.size())
        {
            size_t j = key.find('/', i);
            if (j == std::string::npos) j = key.size();
            if (j > i) out.push_back(key.substr(i, j - i));
            i = j + 1;
        }
        return out;
    }

    std::string Join(const Path& path)
    {
        std::string out;
        for (size_t i = 0; i < path.size(); i++)
        {
            if (i) out += '/';
            out += path[i];
        }
        return out;
    }

    bool Within(const std::string& key, const Path& path)
    {
        return Starts(Split(key), path);
    }

    bool Below(const std::string& key, const Path& path)
    {
        const Path p = Split(key);
        return p.size() > path.size() && Starts(p, path);
    }

    void Put(std::vector<v31np::Row>& rows, const v31np::Row& row)
    {
        const Path p = Split(row.key);
        rows.erase(std::remove_if(rows.begin(), rows.end(), [&](const v31np::Row& r)
            {
                if (!Keyed(r)) return false;
                if (Within(r.key, p)) return true;
                return Above(r.key, p) && (r.kind == 'D' || v31np::Stored(r));
            }), rows.end());
        rows.push_back(row);
    }

    void Remove(std::vector<v31np::Row>& rows, const Path& path)
    {
        rows.erase(std::remove_if(rows.begin(), rows.end(), [&](const v31np::Row& r) { return Keyed(r) && Within(r.key, path); }), rows.end());
        LeaveParent(rows, path);
    }

    void MakeDir(std::vector<v31np::Row>& rows, const Path& path)
    {
        rows.erase(std::remove_if(rows.begin(), rows.end(), [&](const v31np::Row& r) { return r.kind == 'D' && (Above(r.key, path) || Within(r.key, path)); }), rows.end());
        rows.push_back({ 'D', Join(path), 0, 0, {}, {}, 0 });
    }

    void Move(std::vector<v31np::Row>& rows, const Path& from, const Path& to)
    {
        for (auto& r : rows)
        {
            if (!Keyed(r) || !Within(r.key, from)) continue;
            const Path p = Split(r.key);
            Path moved = to;
            moved.insert(moved.end(), p.begin() + (std::ptrdiff_t)from.size(), p.end());
            r.key = Join(moved);
        }
        rows.erase(std::remove_if(rows.begin(), rows.end(), [&](const v31np::Row& r) { return r.kind == 'D' && Above(r.key, to); }), rows.end());
        LeaveParent(rows, from);
    }

    bool Refers(const std::vector<v31np::Row>& rows, const std::wstring& file)
    {
        for (const auto& r : rows)
            if (!r.file.empty() && r.file == file) return true;
        return false;
    }

    bool Refers(const Parts& parts, const std::wstring& file)
    {
        for (const auto& part : parts)
            if (Refers(part.second, file)) return true;
        return false;
    }

    void Rekey(Parts& parts, const v31np::Chain& level, const Path& from, const Path& to)
    {
        Parts out;
        for (auto& part : parts)
        {
            v31np::Chain chain = part.first;
            if (chain.size() > level.size() && Starts(chain, level) && Within(chain[level.size()], from))
            {
                const Path p = Split(chain[level.size()]);
                Path moved = to;
                moved.insert(moved.end(), p.begin() + (std::ptrdiff_t)from.size(), p.end());
                chain[level.size()] = Join(moved);
            }
            out[chain] = std::move(part.second);
        }
        parts = std::move(out);
    }

    void Prune(Parts& parts)
    {
        for (auto it = parts.begin(); it != parts.end();)
        {
            if (it->first.empty())
            {
                ++it;
                continue;
            }
            const v31np::Chain parent(it->first.begin(), it->first.end() - 1);
            const std::string& key = it->first.back();
            auto up = parts.find(parent);
            bool held = false;
            if (up != parts.end())
                for (const auto& r : up->second)
                    if (v31np::Stored(r) && r.key == key) held = true;
            if (held) ++it;
            else it = parts.erase(it);
        }
    }
}
