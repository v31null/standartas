#pragma once

#include "v31np.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace v31np
{
    constexpr size_t None = (size_t)-1;
    constexpr size_t NameMax = 255;
    constexpr size_t PathMax = 259;

    struct Node
    {
        std::wstring stored;
        std::wstring natural;
        bool dir = false;
        size_t parent = None;
        size_t entry = None;
        std::vector<size_t> kids;
    };

    class Tree
    {
    public:
        std::vector<Node> nodes;

        void Build(const std::vector<Entry>& entries);
        std::wstring Path(size_t node, bool natural, wchar_t sep) const;
        void Files(size_t node, std::vector<size_t>& out) const;
        bool Fit(size_t root, size_t prefix, bool longPaths);
    };

    std::wstring Wide(const std::string& utf8);
    std::string Narrow(const std::wstring& wide);
    std::wstring Fold(const std::wstring& s);
    std::wstring Natural(const std::wstring& segment);
    std::wstring Suffix(const std::wstring& name, size_t k);
    bool LongPaths();
}
