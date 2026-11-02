#include "tree.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#pragma comment(lib, "advapi32.lib")
#else
// wchar_t holds whole code points here (UTF-32), so no surrogates occur.
#define IS_HIGH_SURROGATE(c) false
#define IS_LOW_SURROGATE(c) false
#endif

#include <cwchar>
#include <map>
#include <unordered_set>

namespace v31np
{
    namespace
    {
        const wchar_t* const Letters[] = { L"a", L"b", L"ú", L"u", L"ù", L"g", L"d", L"e", L"ż", L"z", L"i", L"í", L"k", L"l", L"m", L"n", L"o", L"p", L"r", L"s", L"t", L"f", L"ț", L"č", L"ș", L"c", L"q", L"x", L"h" };
        constexpr size_t LetterCount = sizeof(Letters) / sizeof(Letters[0]);

        void TrimEnd(std::wstring& s)
        {
            while (!s.empty() && (s.back() == L' ' || s.back() == L'.')) s.pop_back();
        }

#ifdef _WIN32
        // Names are limited in UTF-16 units.
        size_t Length(const std::wstring& s)
        {
            return s.size();
        }

        size_t CutPoint(const std::wstring& s, size_t len)
        {
            if (len >= s.size()) return s.size();
            if (len > 0 && IS_HIGH_SURROGATE(s[len - 1])) return len - 1;
            return len;
        }
#else
        // Names are limited in bytes of UTF-8.
        size_t Bytes(wchar_t c)
        {
            const uint32_t u = (uint32_t)c;
            return u < 0x80 ? 1 : u < 0x800 ? 2 : u < 0x10000 ? 3 : 4;
        }

        size_t Length(const std::wstring& s)
        {
            size_t n = 0;
            for (wchar_t c : s) n += Bytes(c);
            return n;
        }

        // The number of characters of s that fit in len bytes.
        size_t CutPoint(const std::wstring& s, size_t len)
        {
            size_t n = 0;
            for (size_t i = 0; i < s.size(); i++)
            {
                n += Bytes(s[i]);
                if (n > len) return i;
            }
            return s.size();
        }
#endif

#ifdef _WIN32
        bool Reserved(const std::wstring& name)
        {
            std::wstring base = name.substr(0, name.find(L'.'));
            while (!base.empty() && base.back() == L' ') base.pop_back();
            std::wstring u = Fold(base);
            if (u == L"CON" || u == L"PRN" || u == L"AUX" || u == L"NUL" || u == L"CONIN$" || u == L"CONOUT$") return true;
            if (u.size() == 4 && (u.compare(0, 3, L"COM") == 0 || u.compare(0, 3, L"LPT") == 0))
            {
                wchar_t d = u[3];
                return (d >= L'0' && d <= L'9') || d == L'¹' || d == L'²' || d == L'³';
            }
            return false;
        }
#else
        bool Reserved(const std::wstring&)
        {
            return false;
        }
#endif

        std::wstring Clip(const std::wstring& name, size_t max)
        {
            if (Length(name) <= max) return name;
            size_t dot = name.rfind(L'.');
            if (dot != std::wstring::npos && Length(name.substr(dot)) < max)
            {
                std::wstring ext = name.substr(dot);
                return name.substr(0, CutPoint(name, max - Length(ext))) + ext;
            }
            std::wstring r = name.substr(0, CutPoint(name, max));
#ifdef _WIN32
            TrimEnd(r);
#endif
            return r;
        }

        std::vector<std::wstring> Segments(const std::wstring& key)
        {
            std::vector<std::wstring> out;
            size_t i = 0;
            while (i < key.size() && key[i] == L'/') i++;
#ifdef _WIN32
            if (key.size() - i >= 2 && key[i + 1] == L':' && ((key[i] >= L'A' && key[i] <= L'Z') || (key[i] >= L'a' && key[i] <= L'z')))
            {
                out.push_back(key.substr(i, 2));
                i += 2;
            }
#endif
            while (i < key.size())
            {
                size_t j = key.find(L'/', i);
                if (j == std::wstring::npos) j = key.size();
                if (j > i) out.push_back(key.substr(i, j - i));
                i = j + 1;
            }
            if (out.empty()) out.push_back(L"");
            return out;
        }

        void Name(Tree& t, size_t d)
        {
            const auto kids = t.nodes[d].kids;
            std::vector<std::wstring> base(kids.size());
            std::unordered_set<std::wstring> taken;
            std::unordered_set<std::wstring> used;
            for (size_t i = 0; i < kids.size(); i++)
            {
                base[i] = Natural(t.nodes[kids[i]].stored);
                taken.insert(Fold(base[i]));
            }
            for (size_t i = 0; i < kids.size(); i++)
            {
                std::wstring name = base[i];
                std::wstring f = Fold(name);
                if (used.count(f))
                {
                    for (size_t k = 1;; k++)
                    {
                        std::wstring cand = Suffix(base[i], k);
                        std::wstring fc = Fold(cand);
                        if (!taken.count(fc) && !used.count(fc))
                        {
                            name = cand;
                            f = fc;
                            break;
                        }
                    }
                }
                used.insert(f);
                t.nodes[kids[i]].natural = name;
            }
        }

        bool Shorten(Tree& t, size_t x)
        {
            Node& n = t.nodes[x];
            std::wstring base = n.natural;
            std::wstring ext;
            if (!n.dir)
            {
                size_t dot = base.rfind(L'.');
                if (dot != std::wstring::npos)
                {
                    ext = base.substr(dot);
                    base.resize(dot);
                }
            }
            for (;;)
            {
                if (base.empty()) return false;
                size_t cut = base.size() - 1;
                if (cut > 0 && IS_LOW_SURROGATE(base[cut]) && IS_HIGH_SURROGATE(base[cut - 1])) cut--;
                base.resize(cut);
                if (base.empty() && !ext.empty()) return false;
                std::wstring cand = base + ext;
                TrimEnd(cand);
                if (cand.empty() || Reserved(cand)) continue;
                std::wstring fc = Fold(cand);
                bool clash = false;
                for (size_t s : t.nodes[n.parent].kids)
                    if (s != x && Fold(t.nodes[s].natural) == fc) clash = true;
                if (clash) continue;
                n.natural = cand;
                return true;
            }
        }
    }

#ifdef _WIN32
    std::wstring Wide(const std::string& utf8)
    {
        if (utf8.empty()) return {};
        int n = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), nullptr, 0);
        std::wstring r((size_t)n, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), r.data(), n);
        return r;
    }

    std::string Narrow(const std::wstring& wide)
    {
        if (wide.empty()) return {};
        int n = WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
        std::string r((size_t)n, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(), r.data(), n, nullptr, nullptr);
        return r;
    }

    std::wstring Fold(const std::wstring& s)
    {
        if (s.empty()) return s;
        int n = LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_UPPERCASE, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr, 0);
        std::wstring r((size_t)n, L'\0');
        LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_UPPERCASE, s.data(), (int)s.size(), r.data(), n, nullptr, nullptr, 0);
        return r;
    }

    std::wstring Natural(const std::wstring& segment)
    {
        if (segment == L"." || segment == L"..") return L"AN";
        std::wstring r;
        for (wchar_t c : segment)
        {
            if (c == L':' || c == L'\\') continue;
            if (c == L'*' || c == L'?' || c == L'<' || c == L'>' || c == L'|' || c == L'"') r += L'!';
            else if (c < 0x20)
            {
                wchar_t hex[8];
                swprintf_s(hex, L"0x%02X", (unsigned)c);
                r += hex;
            }
            else r += c;
        }
        TrimEnd(r);
        if (r.empty()) return L"NÍ";
        if (Reserved(r)) r.insert(0, 1, L'!');
        return Clip(r, NameMax);
    }

    bool LongPaths()
    {
        DWORD v = 0;
        DWORD size = sizeof(v);
        if (RegGetValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\FileSystem", L"LongPathsEnabled", RRF_RT_REG_DWORD, nullptr, &v, &size) != ERROR_SUCCESS) return false;
        return v == 1;
    }

#else
    std::wstring Wide(const std::string& utf8)
    {
        std::wstring r;
        r.reserve(utf8.size());
        const unsigned char* p = (const unsigned char*)utf8.data();
        const size_t n = utf8.size();
        for (size_t i = 0; i < n;)
        {
            uint32_t c = p[i];
            size_t len = c < 0x80 ? 1 : c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 1;
            if (len > 1 && i + len <= n)
            {
                c &= 0xFF >> (len + 1);
                for (size_t k = 1; k < len; k++) c = (c << 6) | (p[i + k] & 0x3F);
            }
            else if (len > 1)
            {
                c = 0xFFFD;
                len = 1;
            }
            r.push_back((wchar_t)c);
            i += len;
        }
        return r;
    }

    std::string Narrow(const std::wstring& wide)
    {
        std::string r;
        r.reserve(wide.size());
        for (wchar_t w : wide)
        {
            const uint32_t c = (uint32_t)w;
            if (c < 0x80) r += (char)c;
            else if (c < 0x800)
            {
                r += (char)(0xC0 | (c >> 6));
                r += (char)(0x80 | (c & 0x3F));
            }
            else if (c < 0x10000)
            {
                r += (char)(0xE0 | (c >> 12));
                r += (char)(0x80 | ((c >> 6) & 0x3F));
                r += (char)(0x80 | (c & 0x3F));
            }
            else
            {
                r += (char)(0xF0 | (c >> 18));
                r += (char)(0x80 | ((c >> 12) & 0x3F));
                r += (char)(0x80 | ((c >> 6) & 0x3F));
                r += (char)(0x80 | (c & 0x3F));
            }
        }
        return r;
    }

    // Names on GNU/Linux are compared exactly, so folding leaves them as they are.
    std::wstring Fold(const std::wstring& s)
    {
        return s;
    }

    // GNU/Linux rules: only what the system needs or what breaks terminals.
    // Control characters become 0xHH, . and .. become AN, an empty name becomes NÍ,
    // and names are cut at 255 bytes with the extension kept.
    std::wstring Natural(const std::wstring& segment)
    {
        if (segment == L"." || segment == L"..") return L"AN";
        std::wstring r;
        for (wchar_t c : segment)
        {
            const uint32_t u = (uint32_t)c;
            if (u < 0x20 || u == 0x7F || (u >= 0x80 && u <= 0x9F))
            {
                wchar_t hex[8];
                swprintf(hex, 8, L"0x%02X", (unsigned)u);
                r += hex;
            }
            else r += c;
        }
        if (r.empty()) return L"NÍ";
        return Clip(r, NameMax);
    }

    // Paths are not limited the way Windows limits them; each name is held to NameMax by Natural.
    bool LongPaths()
    {
        return true;
    }
#endif

    std::wstring Suffix(const std::wstring& name, size_t k)
    {
        std::wstring s;
        for (size_t n = k; n > 0; n = (n - 1) / LetterCount) s = Letters[(n - 1) % LetterCount] + s;
        size_t dot = name.rfind(L'.');
        size_t cut = dot == std::wstring::npos ? name.size() : dot;
        std::wstring base = name.substr(0, cut);
        std::wstring ext = name.substr(cut);
        const size_t tail = Length(s) + Length(ext);
        if (Length(base) + tail > NameMax && NameMax > tail)
            base.resize(CutPoint(base, NameMax - tail));
        return base + s + ext;
    }

    void Tree::Build(const std::vector<Entry>& entries, const std::vector<std::string>& extra)
    {
        nodes.clear();
        nodes.emplace_back();
        nodes[0].dir = true;
        std::map<std::pair<size_t, std::wstring>, size_t> dirs;
        for (size_t i = 0; i < entries.size() + extra.size(); i++)
        {
            const bool phantom = i >= entries.size();
            auto segs = Segments(Wide(phantom ? extra[i - entries.size()] : entries[i].key));
            if (phantom && segs.empty()) continue;
            size_t cur = 0;
            for (size_t k = 0; k + (phantom ? 0 : 1) < segs.size(); k++)
            {
                auto it = dirs.find({ cur, segs[k] });
                if (it == dirs.end())
                {
                    Node n;
                    n.stored = segs[k];
                    n.dir = true;
                    n.parent = cur;
                    nodes.push_back(std::move(n));
                    size_t id = nodes.size() - 1;
                    nodes[cur].kids.push_back(id);
                    it = dirs.emplace(std::make_pair(cur, segs[k]), id).first;
                }
                cur = it->second;
            }
            if (phantom) continue;
            Node f;
            f.stored = segs.back();
            f.parent = cur;
            f.entry = i;
            nodes.push_back(std::move(f));
            nodes[cur].kids.push_back(nodes.size() - 1);
        }
        for (size_t d = 0; d < nodes.size(); d++)
            if (nodes[d].dir) Name(*this, d);
    }

    std::wstring Tree::Path(size_t node, bool natural, wchar_t sep) const
    {
        std::vector<const std::wstring*> parts;
        for (size_t x = node; x != 0 && x != None; x = nodes[x].parent) parts.push_back(natural ? &nodes[x].natural : &nodes[x].stored);
        std::wstring r;
        for (size_t i = parts.size(); i-- > 0;)
        {
            r += *parts[i];
            if (i > 0) r += sep;
        }
        return r;
    }

    void Tree::Files(size_t node, std::vector<size_t>& out) const
    {
        if (!nodes[node].dir)
        {
            out.push_back(node);
            return;
        }
        for (size_t k : nodes[node].kids) Files(k, out);
    }

    bool Tree::Fit(size_t root, size_t prefix, bool longPaths)
    {
        if (longPaths) return true;
        std::vector<size_t> files;
        Files(root, files);
        auto length = [&](size_t f)
        {
            size_t n = prefix;
            for (size_t x = f; x != root; x = nodes[x].parent) n += nodes[x].natural.size() + 1;
            return n - 1;
        };
        std::vector<bool> stuck(nodes.size(), false);
        for (;;)
        {
            size_t worst = None;
            size_t most = 0;
            for (size_t f : files)
            {
                size_t l = length(f);
                if (l > most)
                {
                    most = l;
                    worst = f;
                }
            }
            if (worst == None || most <= PathMax) return true;
            size_t pick = None;
            for (size_t x = nodes[worst].parent; x != root; x = nodes[x].parent)
                if (!stuck[x] && (pick == None || nodes[x].natural.size() > nodes[pick].natural.size())) pick = x;
            if (pick == None)
            {
                if (stuck[worst]) return false;
                pick = worst;
            }
            if (!Shorten(*this, pick)) stuck[pick] = true;
        }
    }
}
