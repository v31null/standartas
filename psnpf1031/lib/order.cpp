#include "order.h"
#include "classes.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#pragma comment(lib, "normaliz.lib")
#else
#include <glib.h>
#endif

#include <unordered_map>
#include <vector>

#ifndef _WIN32
namespace v31np
{
    std::wstring Wide(const std::string& utf8);
    std::string Narrow(const std::wstring& wide);
}
#endif

namespace v31np
{
    namespace
    {
        struct Slot
        {
            int rank;
            int place;
        };

        const std::unordered_map<char32_t, Slot>& Slots()
        {
            static const std::unordered_map<char32_t, Slot> r = {
                { 0x30, { 0, 0 } },
                { 0x31, { 1, 0 } },
                { 0x32, { 2, 0 } },
                { 0x33, { 3, 0 } },
                { 0x34, { 4, 0 } },
                { 0x35, { 5, 0 } },
                { 0x36, { 6, 0 } },
                { 0x37, { 7, 0 } },
                { 0x38, { 8, 0 } },
                { 0x39, { 9, 0 } },
                { 0x61, { 10, 0 } }, { 0x430, { 10, 1 } },
                { 0x62, { 11, 0 } }, { 0x431, { 11, 1 } },
                { 0xFA, { 12, 0 } }, { 0x76, { 12, 1 } }, { 0x432, { 12, 2 } },
                { 0x75, { 13, 0 } }, { 0x443, { 13, 1 } },
                { 0xF9, { 14, 0 } }, { 0x77, { 14, 1 } },
                { 0x67, { 15, 0 } }, { 0x433, { 15, 1 } },
                { 0x64, { 16, 0 } }, { 0x434, { 16, 1 } }, { 0xFE, { 16, 2 } },
                { 0x65, { 17, 0 } }, { 0x435, { 17, 1 } }, { 0x44D, { 17, 2 } },
                { 0x17C, { 18, 0 } }, { 0x6A, { 18, 1 } }, { 0x436, { 18, 2 } },
                { 0x7A, { 19, 0 } }, { 0x437, { 19, 1 } },
                { 0x69, { 20, 0 } }, { 0x438, { 20, 1 } },
                { 0xED, { 21, 0 } }, { 0x79, { 21, 1 } }, { 0x439, { 21, 2 } },
                { 0x6B, { 22, 0 } }, { 0x43A, { 22, 1 } },
                { 0x6C, { 23, 0 } }, { 0x43B, { 23, 1 } },
                { 0x6D, { 24, 0 } }, { 0x43C, { 24, 1 } },
                { 0x6E, { 25, 0 } }, { 0x43D, { 25, 1 } },
                { 0x6F, { 26, 0 } }, { 0x43E, { 26, 1 } },
                { 0x70, { 27, 0 } }, { 0x43F, { 27, 1 } },
                { 0x72, { 28, 0 } }, { 0x440, { 28, 1 } },
                { 0x73, { 29, 0 } }, { 0x441, { 29, 1 } },
                { 0x74, { 30, 0 } }, { 0x442, { 30, 1 } },
                { 0x66, { 31, 0 } }, { 0x444, { 31, 1 } },
                { 0x21B, { 32, 0 } }, { 0x446, { 32, 1 } },
                { 0x10D, { 33, 0 } }, { 0x447, { 33, 1 } },
                { 0x219, { 34, 0 } }, { 0x448, { 34, 1 } },
                { 0x63, { 35, 0 } },
                { 0x71, { 36, 0 } },
                { 0x78, { 37, 0 } },
                { 0x68, { 38, 0 } }, { 0x445, { 38, 1 } }, { 0x2018, { 38, 2 } },
            };
            return r;
        }

#ifdef _WIN32
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
#else
        // wchar_t holds whole code points here (UTF-32), so no surrogates.
        enum NORM_FORM { NormalizationC, NormalizationD };

        std::u32string Points(const std::wstring& s)
        {
            return std::u32string(s.begin(), s.end());
        }

        std::wstring Units(const std::u32string& s)
        {
            return std::wstring(s.begin(), s.end());
        }

        std::wstring Normal(NORM_FORM form, const std::wstring& s)
        {
            if (s.empty()) return s;
            const std::string u = Narrow(s);
            gchar* n = g_utf8_normalize(u.data(), (gssize)u.size(), form == NormalizationD ? G_NORMALIZE_NFD : G_NORMALIZE_NFC);
            if (!n) return s;
            std::wstring out = Wide(n);
            g_free(n);
            return out;
        }
#endif

        std::wstring Lower(const std::wstring& w)
        {
            std::u32string p = Points(w);
            for (auto& c : p) c = LowerCase(c);
            return Units(p);
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

            return Points(Lower(Units(numbered)));
        }

        std::wstring Stem(const std::wstring& name)
        {
            const size_t dot = name.rfind(L'.');
            return dot == std::wstring::npos || dot == 0 ? name : name.substr(0, dot);
        }

        int Rank(const std::u32string& s1, const std::u32string& s2)
        {
            const auto& slots = Slots();
            const size_t min = s1.size() < s2.size() ? s1.size() : s2.size();
            for (size_t i = 0; i < min; i++)
            {
                const char32_t c1 = s1[i];
                const char32_t c2 = s2[i];
                if (c1 == c2) continue;
                auto r1 = slots.find(c1);
                auto r2 = slots.find(c2);
                if (r1 != slots.end() && r2 != slots.end())
                {
                    if (r1->second.rank != r2->second.rank) return r1->second.rank < r2->second.rank ? -1 : 1;
                }
                else if (r1 != slots.end()) return -1;
                else if (r2 != slots.end()) return 1;
                else return c1 < c2 ? -1 : 1;
            }
            return s1.size() == s2.size() ? 0 : s1.size() < s2.size() ? -1 : 1;
        }

        int Place(const std::u32string& s1, const std::u32string& s2)
        {
            const auto& slots = Slots();
            const size_t min = s1.size() < s2.size() ? s1.size() : s2.size();
            for (size_t i = 0; i < min; i++)
            {
                if (s1[i] == s2[i]) continue;
                auto r1 = slots.find(s1[i]);
                auto r2 = slots.find(s2[i]);
                if (r1 != slots.end() && r2 != slots.end() && r1->second.place != r2->second.place)
                    return r1->second.place < r2->second.place ? -1 : 1;
                return 0;
            }
            return 0;
        }

        bool Capital(char32_t c)
        {
            return LowerCase(c) != c;
        }

        int Case(const std::u32string& s1, const std::u32string& s2)
        {
            const size_t min = s1.size() < s2.size() ? s1.size() : s2.size();
            for (size_t i = 0; i < min; i++)
            {
                const bool c1 = Capital(s1[i]);
                const bool c2 = Capital(s2[i]);
                if (c1 != c2) return c1 ? -1 : 1;
            }
            return 0;
        }
    }

    int Order(const std::wstring& a, const std::wstring& b)
    {
        const std::u32string stemA = Fold(Stem(a));
        const std::u32string stemB = Fold(Stem(b));
        int c = Rank(stemA, stemB);
        if (c != 0) return c;
        const std::u32string foldA = Fold(a);
        const std::u32string foldB = Fold(b);
        c = Rank(foldA, foldB);
        if (c != 0) return c;
        c = Place(stemA, stemB);
        if (c != 0) return c;
        c = Place(foldA, foldB);
        if (c != 0) return c;
        const std::u32string pointsA = Points(a);
        const std::u32string pointsB = Points(b);
        c = Case(pointsA, pointsB);
        if (c != 0) return c;
        if (pointsA.size() != pointsB.size()) return pointsA.size() < pointsB.size() ? -1 : 1;
        return pointsA == pointsB ? 0 : pointsA < pointsB ? -1 : 1;
    }
}
