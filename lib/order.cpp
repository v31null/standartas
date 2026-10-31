#include "order.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <unordered_map>
#include <vector>

#pragma comment(lib, "normaliz.lib")

namespace v31np
{
    namespace
    {
        const std::unordered_map<char32_t, int>& Ranks()
        {
            static const std::unordered_map<char32_t, int> r = {
                { 0x30, 0 }, { 0x31, 1 }, { 0x32, 2 }, { 0x33, 3 }, { 0x34, 4 },
                { 0x35, 5 }, { 0x36, 6 }, { 0x37, 7 }, { 0x38, 8 }, { 0x39, 9 },
                { 0x61, 10 }, { 0x430, 10 },
                { 0x62, 11 }, { 0x431, 11 },
                { 0xFA, 12 }, { 0x76, 12 }, { 0x432, 12 },
                { 0x75, 13 }, { 0x443, 13 },
                { 0xF9, 14 }, { 0x77, 14 },
                { 0x67, 15 }, { 0x433, 15 },
                { 0x64, 16 }, { 0x434, 16 }, { 0xFE, 16 },
                { 0x65, 17 }, { 0x435, 17 }, { 0x44D, 17 },
                { 0x17C, 18 }, { 0x6A, 18 }, { 0x436, 18 },
                { 0x7A, 19 }, { 0x437, 19 },
                { 0x69, 20 }, { 0x438, 20 },
                { 0xED, 21 }, { 0x79, 21 }, { 0x439, 21 },
                { 0x6B, 22 }, { 0x43A, 22 },
                { 0x6C, 23 }, { 0x43B, 23 },
                { 0x6D, 24 }, { 0x43C, 24 },
                { 0x6E, 25 }, { 0x43D, 25 },
                { 0x6F, 26 }, { 0x43E, 26 },
                { 0x70, 27 }, { 0x43F, 27 },
                { 0x72, 28 }, { 0x440, 28 },
                { 0x73, 29 }, { 0x441, 29 },
                { 0x74, 30 }, { 0x442, 30 },
                { 0x66, 31 }, { 0x444, 31 },
                { 0x21B, 32 }, { 0x446, 32 },
                { 0x10D, 33 }, { 0x447, 33 },
                { 0x219, 34 }, { 0x448, 34 },
                { 0x63, 35 },
                { 0x71, 36 },
                { 0x78, 37 },
                { 0x68, 38 }, { 0x445, 38 }, { 0x2018, 38 },
            };
            return r;
        }

        std::u32string Points(const std::wstring& s)
        {
            std::u32string out;
            for (size_t i = 0; i < s.size(); i++)
            {
                char32_t c = s[i];
                if (IS_HIGH_SURROGATE(s[i]) && i + 1 < s.size() && IS_LOW_SURROGATE(s[i + 1]))
                {
                    c = 0x10000 + (((char32_t)s[i] - 0xD800) << 10) + ((char32_t)s[i + 1] - 0xDC00);
                    i++;
                }
                out.push_back(c);
            }
            return out;
        }

        std::wstring Units(const std::u32string& s)
        {
            std::wstring out;
            for (char32_t c : s)
            {
                if (c >= 0x10000)
                {
                    c -= 0x10000;
                    out.push_back((wchar_t)(0xD800 + (c >> 10)));
                    out.push_back((wchar_t)(0xDC00 + (c & 0x3FF)));
                }
                else out.push_back((wchar_t)c);
            }
            return out;
        }

        std::wstring Normal(NORM_FORM form, const std::wstring& s)
        {
            if (s.empty()) return s;
            int n = NormalizeString(form, s.data(), (int)s.size(), nullptr, 0);
            for (int tries = 0; tries < 4 && n > 0; tries++)
            {
                std::wstring out((size_t)n, L'\0');
                int got = NormalizeString(form, s.data(), (int)s.size(), out.data(), n);
                if (got > 0)
                {
                    out.resize((size_t)got);
                    return out;
                }
                if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) break;
                n = -got;
            }
            return s;
        }

        bool Space(char32_t c)
        {
            if (c >= 0x10000) return false;
            WORD type = 0;
            wchar_t w = (wchar_t)c;
            if (!GetStringTypeW(CT_CTYPE1, &w, 1, &type)) return false;
            return (type & (C1_SPACE | C1_BLANK)) != 0;
        }

        bool Loose(char32_t c)
        {
            if (c >= 0x10000) return false;
            WORD type = 0;
            wchar_t w = (wchar_t)c;
            if (!GetStringTypeW(CT_CTYPE1, &w, 1, &type)) return false;
            return (type & (C1_PUNCT | C1_SPACE | C1_BLANK)) != 0;
        }

        std::u32string Fold(const std::wstring& name)
        {
            std::u32string t = Points(name);
            size_t i = 0;
            while (i < t.size() && Space(t[i])) i++;
            if (i < t.size() && t[i] == U'[')
            {
                size_t close = t.find(U']', i + 1);
                if (close != std::u32string::npos && close > i + 1) t = t.substr(i + 1, close - i - 1);
            }
            i = 0;
            while (i < t.size() && Loose(t[i])) i++;
            t.erase(0, i);

            std::u32string d = Points(Normal(NormalizationD, Units(t)));
            std::u32string doubled;
            for (size_t k = 0; k < d.size(); k++)
            {
                if (d[k] != U'\n' && k + 1 < d.size() && d[k + 1] == 0x308)
                {
                    doubled.push_back(d[k]);
                    doubled.push_back(d[k]);
                    k++;
                }
                else doubled.push_back(d[k]);
            }
            t = Points(Normal(NormalizationC, Units(doubled)));

            std::u32string numbered;
            for (size_t k = 0; k < t.size();)
            {
                if (t[k] == 0x2116)
                {
                    size_t j = k + 1;
                    while (j < t.size() && Space(t[j])) j++;
                    size_t start = j;
                    while (j < t.size() && t[j] >= U'0' && t[j] <= U'9') j++;
                    if (j > start)
                    {
                        size_t lead = start;
                        while (lead + 1 < j && t[lead] == U'0') lead++;
                        std::u32string digits = t.substr(lead, j - lead);
                        numbered.push_back(0x2116);
                        for (size_t p = digits.size(); p < 5; p++) numbered.push_back(U'0');
                        numbered += digits;
                        k = j;
                        continue;
                    }
                }
                numbered.push_back(t[k]);
                k++;
            }

            std::wstring w = Units(numbered);
            if (!w.empty())
            {
                int n = LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr, 0);
                if (n > 0)
                {
                    std::wstring low((size_t)n, L'\0');
                    LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE, w.data(), (int)w.size(), low.data(), n, nullptr, nullptr, 0);
                    w = low;
                }
            }
            return Points(w);
        }

        std::wstring Stem(const std::wstring& name)
        {
            const size_t dot = name.rfind(L'.');
            return dot == std::wstring::npos || dot == 0 ? name : name.substr(0, dot);
        }

        int Rank(const std::u32string& s1, const std::u32string& s2)
        {
            const auto& ranks = Ranks();
            const size_t min = s1.size() < s2.size() ? s1.size() : s2.size();
            for (size_t i = 0; i < min; i++)
            {
                const char32_t c1 = s1[i];
                const char32_t c2 = s2[i];
                if (c1 == c2) continue;
                auto r1 = ranks.find(c1);
                auto r2 = ranks.find(c2);
                if (r1 != ranks.end() && r2 != ranks.end())
                {
                    if (r1->second != r2->second) return r1->second < r2->second ? -1 : 1;
                }
                else if (r1 != ranks.end()) return -1;
                else if (r2 != ranks.end()) return 1;
                else return c1 < c2 ? -1 : 1;
            }
            return s1.size() == s2.size() ? 0 : s1.size() < s2.size() ? -1 : 1;
        }
    }

    int Order(const std::wstring& a, const std::wstring& b)
    {
        const int c = Rank(Fold(Stem(a)), Fold(Stem(b)));
        return c != 0 ? c : Rank(Fold(a), Fold(b));
    }
}
