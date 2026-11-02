#pragma once

#include "shadow.h"

#include <map>
#include <string>
#include <vector>

namespace change
{
    using Path = std::vector<std::string>;
    using Parts = std::map<v31np::Chain, std::vector<v31np::Row>>;

    Path Split(const std::string& key);
    std::string Join(const Path& path);
    bool Within(const std::string& key, const Path& path);
    bool Below(const std::string& key, const Path& path);

    void Put(std::vector<v31np::Row>& rows, const v31np::Row& row);
    void Remove(std::vector<v31np::Row>& rows, const Path& path);
    void MakeDir(std::vector<v31np::Row>& rows, const Path& path);
    void Move(std::vector<v31np::Row>& rows, const Path& from, const Path& to);
    bool Refers(const std::vector<v31np::Row>& rows, const std::wstring& file);

    bool Refers(const Parts& parts, const std::wstring& file);
    void Rekey(Parts& parts, const v31np::Chain& level, const Path& from, const Path& to);
    void Prune(Parts& parts);
}
