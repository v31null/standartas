// Tests of the shared core (../lib) on GNU/Linux, checked against the tables of readme.md.

#include "officer.h"
#include "order.h"
#include "propertime.h"
#include "road.h"
#include "shadow.h"
#include "traffic.h"
#include "tree.h"
#include "v31np.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <unistd.h>
#include <string>
#include <vector>

namespace
{
    int failures = 0;
    int checks = 0;

    void Check(bool ok, const char* what, int line)
    {
        checks++;
        if (!ok)
        {
            failures++;
            std::printf("FAIL line %d: %s\n", line, what);
        }
    }

#define CHECK(x) Check((x), #x, __LINE__)

    using Bytes = std::vector<uint8_t>;

    struct Memory
    {
        std::string data;
        size_t at = 0;
    };

    v31np::Item Of(const std::string& key, const std::string& data, Memory& m)
    {
        m.data = data;
        m.at = 0;
        Memory* p = &m;
        return { key, data.size(), [p](void* buf, uint32_t len)
            {
                if (p->at + len > p->data.size()) return false;
                std::memcpy(buf, p->data.data() + p->at, len);
                p->at += len;
                return true;
            } };
    }

    Bytes Pack(const std::vector<v31np::Item>& items)
    {
        Bytes out;
        bool ok = v31np::Write(items, [&](const void* buf, uint32_t len)
            {
                const uint8_t* b = (const uint8_t*)buf;
                out.insert(out.end(), b, b + len);
                return true;
            });
        if (!ok) out.clear();
        return out;
    }

    v31np::ReadAt Reader(const Bytes& b)
    {
        return [&b](uint64_t pos, void* buf, uint32_t len)
        {
            if (pos + len > b.size()) return false;
            std::memcpy(buf, b.data() + pos, len);
            return true;
        };
    }

    Bytes Hex(const char* s)
    {
        Bytes out;
        unsigned v;
        int n;
        while (std::sscanf(s, " %2x%n", &v, &n) == 1)
        {
            out.push_back((uint8_t)v);
            s += n;
        }
        return out;
    }

    const char* PiHex = "24 3F 6A 88 85 A3 08 D3 13 19 8A 2E";
    const char* PhiHex = "9E 37 79 B9 7F 4A 7C 15 F3 9C C0 60";

    void Append(Bytes& to, const Bytes& b)
    {
        to.insert(to.end(), b.begin(), b.end());
    }

    void Append(Bytes& to, const std::string& s)
    {
        to.insert(to.end(), s.begin(), s.end());
    }

    // Table B-6: a file holding no files.
    void EmptyArchive()
    {
        Bytes got = Pack({});
        Bytes want;
        Append(want, std::string("V31NSPACKFORMAT"));
        for (int i = 0; i < 4; i++) Append(want, Hex(PiHex));
        Append(want, Hex("46 AA 21 E5"));
        CHECK(got.size() == 67);
        CHECK(got == want);
        std::vector<v31np::Entry> e;
        CHECK(v31np::Parse(Reader(got), got.size(), e));
        CHECK(e.empty());
        CHECK(v31np::Verify(Reader(got), got.size()));
    }

    // Table B-3: a file holding docs/note.txt with the text Hello.
    Bytes NoteArchive()
    {
        Bytes want;
        Append(want, std::string("V31NSPACKFORMAT"));
        Append(want, Hex(PiHex));
        want.push_back(0); Append(want, Hex(PhiHex));
        Append(want, std::string("docs/note.txt"));
        want.push_back(1); Append(want, Hex(PhiHex));
        want.push_back(0xB7);
        Append(want, Hex(PiHex));
        want.push_back(0); Append(want, Hex(PhiHex));
        Append(want, std::string("docs/note.txt"));
        want.push_back(1); Append(want, Hex(PhiHex));
        want.push_back(0x05);
        want.push_back(1); Append(want, Hex(PhiHex));
        Append(want, std::string("docs/"));
        want.push_back(1); Append(want, Hex(PhiHex));
        Append(want, std::string("note"));
        want.push_back(1); Append(want, Hex(PhiHex));
        Append(want, std::string(".txt"));
        Append(want, Hex(PiHex));
        Append(want, std::string("Hello"));
        Append(want, Hex(PiHex));
        Append(want, Hex("8D 37 9C 9C"));
        return want;
    }

    void OneFile()
    {
        Memory m;
        std::vector<v31np::Item> items = { Of("docs/note.txt", "Hello", m) };
        CHECK(v31np::Measure(items) == 204);
        Bytes got = Pack(items);
        Bytes want = NoteArchive();
        CHECK(want.size() == 204);
        CHECK(got == want);

        std::vector<v31np::Entry> e;
        CHECK(v31np::Parse(Reader(got), got.size(), e));
        CHECK(e.size() == 1 && e[0].key == "docs/note.txt" && e[0].at == 183 && e[0].size == 5);
        CHECK(v31np::Verify(Reader(got), got.size()));

        // A changed data byte: still read, but the test reports a mismatch (§ B/(a.v.)).
        Bytes bad = got;
        bad[183] ^= 1;
        CHECK(v31np::Parse(Reader(bad), bad.size(), e));
        CHECK(!v31np::Verify(Reader(bad), bad.size()));

        // A wrong address breaks the chain of addresses (§ B/(a.iv.)).
        Bytes chain = got;
        chain[66] = 0xB8;
        CHECK(!v31np::Parse(Reader(chain), chain.size(), e));

        // A truncated file, as in an incomplete download.
        Bytes cut(got.begin(), got.end() - 10);
        CHECK(!v31np::Parse(Reader(cut), cut.size(), e));
    }

    // § B/(a.vi.): the start address grows from one byte to two.
    void HeaderLength()
    {
        Memory m1, m2;
        std::string key(200, 'k');
        std::vector<v31np::Item> items = { Of(key + ".bin", "abc", m1), Of("b/" + key, "", m2) };
        Bytes got = Pack(items);
        CHECK(!got.empty());
        CHECK(got.size() == v31np::Measure(items));
        std::vector<v31np::Entry> e;
        CHECK(v31np::Parse(Reader(got), got.size(), e));
        CHECK(e.size() == 2 && e[0].at > 255 && e[1].at == e[0].at + 3 && e[1].size == 0);
        CHECK(v31np::Verify(Reader(got), got.size()));
    }

    void Refusals()
    {
        Memory a, b;
        CHECK(Pack({ Of("x", "1", a), Of("x", "2", b) }).empty());
        CHECK(Pack({ Of(std::string("\xC3\x28", 2), "1", a) }).empty());
        // Every valid UTF-8 string is a legal key (§ B/(b.i.)).
        CHECK(!Pack({ Of("../C:\\*?", "1", a), Of("", "2", b) }).empty());
    }

    // Table G-2.
    void Suffixes()
    {
        CHECK(v31np::Suffix(L"photo.jpg", 1) == L"photoa.jpg");
        CHECK(v31np::Suffix(L"photo.jpg", 3) == L"photoú.jpg");
        CHECK(v31np::Suffix(L"photo.jpg", 29) == L"photoh.jpg");
        CHECK(v31np::Suffix(L"photo.jpg", 30) == L"photoaa.jpg");
        CHECK(v31np::Suffix(L"photo.jpg", 58) == L"photoah.jpg");
        CHECK(v31np::Suffix(L"photo.jpg", 59) == L"photoba.jpg");
        CHECK(v31np::Suffix(L"photo.jpg", 842) == L"photoha.jpg");
        CHECK(v31np::Suffix(L"photo.jpg", 870) == L"photohh.jpg");
        CHECK(v31np::Suffix(L"photo.jpg", 871) == L"photoaaa.jpg");
        CHECK(v31np::Suffix(L".gitignore", 1) == L"a.gitignore");
        CHECK(v31np::Suffix(L"README", 1) == L"READMEa");
        CHECK(v31np::Suffix(L"eayk.png", 2) == L"eaykb.png");
        // The 255-byte limit holds after the suffix.
        std::wstring longName(253, L'x');
        std::wstring s = v31np::Suffix(longName + L".png", 1);
        CHECK(v31np::Narrow(s).size() <= 255 && s.size() >= 5 && s.compare(s.size() - 5, 5, L"a.png") == 0);
    }

    // GNU/Linux naming rules, as decided for this programme.
    void Names()
    {
        CHECK(v31np::Natural(L".") == L"AN");
        CHECK(v31np::Natural(L"..") == L"AN");
        CHECK(v31np::Natural(L"") == L"NÍ");
        CHECK(v31np::Natural(L"a\tb") == L"a0x09b");
        CHECK(v31np::Natural(std::wstring(L"a\0b", 3)) == L"a0x00b");
        CHECK(v31np::Natural(L"bell\x7F") == L"bell0x7F");
        CHECK(v31np::Natural(L"csi\x9B") == L"csi0x9B");
        CHECK(v31np::Natural(L"C:") == L"C:");
        CHECK(v31np::Natural(L"a*?<>|\"\\:") == L"a*?<>|\"\\:");
        CHECK(v31np::Natural(L"CON") == L"CON");
        CHECK(v31np::Natural(L"trailing. ") == L"trailing. ");
        CHECK(v31np::Natural(L"2026\\1-1.txt") == L"2026\\1-1.txt");

        std::wstring n = v31np::Natural(std::wstring(300, L'x') + L".txt");
        CHECK(v31np::Narrow(n).size() == 255);
        CHECK(n.compare(n.size() - 4, 4, L".txt") == 0);

        std::wstring wide;
        for (int i = 0; i < 200; i++) wide += L"é";
        std::wstring m = v31np::Natural(wide + L".png");
        CHECK(v31np::Narrow(m).size() <= 255);
        CHECK(m.size() == 125 + 4);
        CHECK(m.compare(m.size() - 4, 4, L".png") == 0);

        std::wstring emoji;
        for (int i = 0; i < 70; i++) emoji += L"\U0001F427";
        std::wstring p = v31np::Natural(emoji);
        CHECK(v31np::Narrow(p).size() == 252 && p.size() == 63);

        CHECK(v31np::Wide("\xF0\x9F\x90\xA7") == L"\U0001F427");
        CHECK(v31np::Narrow(L"\U0001F427 ú") == "\xF0\x9F\x90\xA7 \xC3\xBA");
    }

    std::vector<v31np::Entry> Entries(const std::vector<std::string>& keys)
    {
        std::vector<v31np::Entry> e;
        for (const auto& k : keys) e.push_back({ k, 0, 0 });
        return e;
    }

    size_t Child(const v31np::Tree& t, size_t parent, const std::wstring& stored)
    {
        for (size_t k : t.nodes[parent].kids)
            if (t.nodes[k].stored == stored) return k;
        return v31np::None;
    }

    void Trees()
    {
        v31np::Tree t;
        // Clashes are case-sensitive on GNU/Linux.
        t.Build(Entries({ "A.txt", "a.txt" }));
        CHECK(t.nodes[Child(t, 0, L"A.txt")].natural == L"A.txt");
        CHECK(t.nodes[Child(t, 0, L"a.txt")].natural == L"a.txt");

        // . and .. both become AN; the later one receives a suffix.
        t.Build(Entries({ "x/.", "x/.." }));
        size_t x = Child(t, 0, L"x");
        CHECK(x != v31np::None && t.nodes[x].dir);
        CHECK(t.nodes[Child(t, x, L".")].natural == L"AN");
        CHECK(t.nodes[Child(t, x, L"..")].natural == L"ANa");

        // A leading / and empty parts of a key are disregarded.
        t.Build(Entries({ "/a//b" }));
        size_t a = Child(t, 0, L"a");
        CHECK(a != v31np::None && Child(t, a, L"b") != v31np::None);
        CHECK(t.Path(Child(t, a, L"b"), true, L'/') == L"a/b");

        // No drive letters on GNU/Linux: C: is an ordinary name.
        t.Build(Entries({ "C:foo/bar" }));
        CHECK(Child(t, 0, L"C:foo") != v31np::None);

        // ALICEV31NP records: \ is an ordinary character inside one flat name.
        t.Build(Entries({ "ALICEV31NP/DESKTOP AAAA-BBBB\\1.txt", "ALICEV31NP/2026\\1-1.txt" }));
        size_t al = Child(t, 0, L"ALICEV31NP");
        CHECK(al != v31np::None && t.nodes[al].kids.size() == 2);
        CHECK(Child(t, al, L"2026\\1-1.txt") != v31np::None);

        // Empty folders recorded in a journal are presented.
        t.Build(Entries({ "f.txt" }), { "empty/inner" });
        size_t em = Child(t, 0, L"empty");
        CHECK(em != v31np::None && Child(t, em, L"inner") != v31np::None);
        CHECK(t.Fit(0, 10, v31np::LongPaths()));
    }

    // § G/(b): V31N order.
    void Order()
    {
        using v31np::Order;
        CHECK(Order(L"v", L"u") < 0);
        CHECK(Order(L"ú", L"u") < 0);
        CHECK(Order(L"s", L"c") < 0);
        CHECK(Order(L"9", L"a") < 0);
        CHECK(Order(L"h", L"x") > 0);
        CHECK(Order(L"z", L"é") < 0);
        CHECK(Order(L"a", L"а") < 0);
        CHECK(Order(L"Ä", L"AB") < 0);
        CHECK(Order(L"Äb", L"aab") < 0);
        CHECK(Order(L"[beta] x", L"alpha") > 0);
        // № inside a name; a leading № is removed by step 2 as a symbol before step 4 applies.
        CHECK(Order(L"a№7", L"a№12") < 0);
        CHECK(Order(L"a№ 7", L"a№007") < 0);
        CHECK(Order(L"a 7", L"a 12") > 0);
        CHECK(Order(L"№7", L"№12") < 0);
        CHECK(Order(L"№ 7", L"№007") < 0);
        CHECK(Order(L"ªb", L"b") > 0);
        CHECK(Order(L"²z", L"z") > 0);
        CHECK(Order(L"\u0085x", L"x") > 0);
        CHECK(Order(L"€x", L"x") != 0);
        CHECK(Order(L"photo.avi", L"photo.zip") < 0);
        CHECK(Order(L"photo.zip", L"photob.avi") < 0);
        CHECK(Order(L"  _note", L"note") > 0);
        CHECK(Order(L"ab", L"abc") < 0);
        CHECK(Order(L"B", L"a") > 0);
        CHECK(Order(L"A.png", L"a.png") < 0);
        CHECK(Order(L"ú.png", L"V.png") < 0);
        CHECK(Order(L"V.png", L"v.png") < 0);
        CHECK(Order(L"v", L"в") < 0);
        CHECK(Order(L"Ab", L"aB") < 0);
        CHECK(Order(L"-a.png", L"_a.png") < 0);
        CHECK(Order(L"a.png", L"a.png") == 0);
        CHECK(Order(L"İa", L"ib") < 0);
        CHECK(Order(L"İ", L"i") < 0);
        CHECK(Order(L"b", L"a") > 0);
        CHECK(Order(L"", L"") > 0);
    }

    void Journal()
    {
        CHECK(v31np::JournalName(L"photos.v31np") == L"FUTALUZTU UERSIIA UUTI photos.v31np.txt");
        CHECK(v31np::JournalName(L"photos.V31NP") == L"FUTALUZTU UERSIIA UUTI photos.v31np.txt");

        const std::string text =
            "v31np shadow 1\t204\t133000000000000000\r\n"
            "@\r\n"
            "A\tdocs/note.txt\t5\t183\r\n"
            "F\tnew.txt\t3\t/home/a/new.txt\r\n"
            "D\tempty\r\n"
            "M\tinner.v31np\t67\t/home/a/inner.v31np\r\n"
            "R\t/home/a/moved\r\n"
            "@\tinner.v31np\r\n"
            "T\tx\\ty\t2\t/tmp/t1\r\n"
            "H\t/home/a/hidden\r\n";
        v31np::Shadow s;
        CHECK(v31np::ReadJournal(text, s));
        CHECK(s.size == 204 && s.stamp == 133000000000000000ull);
        CHECK(s.parts.size() == 2);
        const auto& top = s.parts[{}];
        CHECK(top.size() == 5 && top[0].kind == 'A' && top[0].at == 183 && top[1].file == L"/home/a/new.txt");
        const auto& inner = s.parts[{ "inner.v31np" }];
        CHECK(inner.size() == 2 && inner[0].key == "x\ty" && inner[1].kind == 'H');
        CHECK(v31np::WriteJournal(s) == text);

        // The nested archive is presented with its length after the repack.
        Memory m;
        CHECK(v31np::Logical(s, {}, top[3]) == v31np::Measure({ Of("x\ty", "12", m) }));

        // A line feed alone and empty lines are accepted.
        v31np::Shadow lf;
        CHECK(v31np::ReadJournal("\n\nv31np shadow 1\t1\t2\n@\nD\tk\n\n", lf));
        CHECK(lf.parts.size() == 1);

        v31np::Shadow bad;
        CHECK(!v31np::ReadJournal("v31np shadow 1\t1\t2\r\n@\r\nD\tbad\\q\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 1\t1\t2\r\nD\tk\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 1\t1\t2\r\n@\r\nZ\tk\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 1\t1\t2\r\n@\r\nA\tk\t1\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 1\t-1\t2\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 1\t1\t2\r\n@\r\n@\r\n", bad));
        CHECK(!v31np::ReadJournal("", bad));
    }

    void SharedListing()
    {
        Memory m1, m2, m3;
        std::vector<v31np::Item> items = { Of("docs/note.txt", "Hello", m1), Of("b.txt", "bee", m2), Of("ab/x", "x", m3) };
        Bytes got = Pack(items);
        std::vector<v31np::Entry> e;
        CHECK(v31np::Parse(Reader(got), got.size(), e));
        const std::vector<std::string> dirs = { "empty", "docs", "a", "", "b", "empty", "ab", "docs/sub" };
        const auto rows = v31np::Listing(e, dirs);
        CHECK(rows.size() == e.size() + 4);
        bool same = rows.size() >= e.size();
        for (size_t i = 0; same && i < e.size(); i++)
            same = rows[i].kind == 'A' && rows[i].key == e[i].key && rows[i].size == e[i].size && rows[i].at == e[i].at && rows[i].file.empty();
        CHECK(same);
        if (rows.size() == e.size() + 4)
        {
            CHECK(rows[e.size()].kind == 'D' && rows[e.size()].key == "a");
            CHECK(rows[e.size() + 1].key == "b");
            CHECK(rows[e.size() + 2].key == "docs/sub");
            CHECK(rows[e.size() + 3].key == "empty" && rows[e.size() + 3].plate.empty() && rows[e.size() + 3].until == 0);
        }

        v31np::Shadow s;
        s.size = got.size();
        s.stamp = 1;
        s.parts[{}] = rows;
        v31np::Shadow back;
        CHECK(v31np::ReadJournal(v31np::WriteJournal(s), back));
        const auto& again = back.parts[{}];
        bool kept = again.size() == rows.size();
        for (size_t i = 0; kept && i < rows.size(); i++)
            kept = again[i].kind == rows[i].kind && again[i].key == rows[i].key && again[i].size == rows[i].size && again[i].at == rows[i].at;
        CHECK(kept);

        std::vector<v31np::Entry> fromSection;
        std::vector<std::string> extra;
        for (const auto& r : again)
            if (r.kind == 'A') fromSection.push_back({ r.key, r.at, r.size });
            else extra.push_back(r.key);
        v31np::Tree viaSection, viaArchive;
        viaSection.Build(fromSection, extra);
        viaArchive.Build(e, { "a", "b", "docs/sub", "empty" });
        bool tree = viaSection.nodes.size() == viaArchive.nodes.size();
        for (size_t i = 0; tree && i < viaSection.nodes.size(); i++)
            tree = viaSection.Path(i, false, L'/') == viaArchive.Path(i, false, L'/') && viaSection.nodes[i].dir == viaArchive.nodes[i].dir;
        CHECK(tree);
        bool data = true;
        for (const auto& f : fromSection)
        {
            const std::string bytes(got.begin() + f.at, got.begin() + f.at + f.size);
            data = data && ((f.key == "docs/note.txt" && bytes == "Hello") || (f.key == "b.txt" && bytes == "bee") || (f.key == "ab/x" && bytes == "x"));
        }
        CHECK(data && fromSection.size() == 3);

        CHECK(v31np::Listing({}, {}).empty());
        CHECK(v31np::Listing(e, { "docs" }).size() == e.size());
    }

    int64_t Jst(int y, unsigned m, unsigned d, unsigned h, unsigned mi, unsigned s)
    {
        int64_t t;
        char buf[32];
        std::snprintf(buf, sizeof buf, "%04d%02u%02uAM000101", y, m, d);
        if (!v31np::Unstamp(buf, t)) return -1;
        return t + h * 3600 + mi * 60 + s;
    }

    void Propertime()
    {
        using v31np::Stamp;
        CHECK(Stamp(0) == "19700101AM090101");
        CHECK(Stamp(Jst(2026, 11, 1, 0, 0, 0)) == "20261101AM000101");
        CHECK(Stamp(Jst(2026, 11, 1, 11, 59, 59)) == "20261101AM116060");
        CHECK(Stamp(Jst(2026, 11, 1, 12, 0, 0)) == "20261101PM000101");
        CHECK(Stamp(Jst(2026, 11, 1, 20, 52, 49)) == "20261101PM085350");
        CHECK(Stamp(Jst(2026, 11, 1, 23, 55, 59)) == "20261101PM115660");
        CHECK(Stamp(Jst(2026, 11, 1, 23, 56, 0)) == "20261101PM115701");
        CHECK(Stamp(Jst(2026, 11, 1, 23, 57, 38)) == "20261101PM115799");
        CHECK(Stamp(Jst(2026, 11, 1, 23, 57, 39)) == "20261101P1157100");
        CHECK(Stamp(Jst(2026, 11, 1, 23, 59, 59)) == "20261101P1157240");
        CHECK(Stamp(Jst(2026, 11, 2, 0, 0, 0)) == "20261102AM000101");
        CHECK(Stamp(Jst(2028, 2, 29, 13, 0, 0)) == "20280229PM010101");

        int64_t t = 0;
        CHECK(v31np::Unstamp("19390101000101AM", t) && Stamp(t) == "19390101AM000101");
        CHECK(v31np::Unstamp("20261101P1157240", t) && t == Jst(2026, 11, 1, 23, 59, 59));
        CHECK(v31np::Unstamp("20261101PM115701", t) && t == Jst(2026, 11, 1, 23, 56, 0));
        CHECK(!v31np::Unstamp("20261101PM115801", t));
        CHECK(!v31np::Unstamp("20261101AM000001", t));
        CHECK(!v31np::Unstamp("20261101AM120101", t));
        CHECK(!v31np::Unstamp("20261101P1157241", t));
        CHECK(!v31np::Unstamp("20261101A1157100", t));
        CHECK(!v31np::Unstamp("20261131AM000101", t));
        CHECK(!v31np::Unstamp("20270229AM000101", t));
        CHECK(!v31np::Unstamp("", t));

        bool round = true;
        const int64_t start = Jst(2026, 12, 31, 0, 0, 0);
        for (int64_t x = start; x < start + 2 * 86400; x++)
        {
            int64_t back = 0;
            if (!v31np::Unstamp(Stamp(x), back) || back != x) round = false;
        }
        CHECK(round);
    }

    void Signals()
    {
        CHECK(v31np::SignalName(L"photos.v31np") == L"NIEURFASMIT UUTI photos.v31np.lock");
        CHECK(v31np::SignalName(L"photos.V31NP") == L"NIEURFASMIT UUTI photos.v31np.lock");
        CHECK(v31np::PlateDigits("") == 346656037u);
        CHECK(v31np::OsRelease("NAME=\"Ubuntu\"\nPRETTY_NAME=\"Ubuntu 26.04.1 LTS\"\nID=ubuntu\n") == "Ubuntu 26.04.1 LTS");
        CHECK(v31np::OsRelease("PRETTY_NAME='Arch Linux'\n") == "Arch Linux");
        CHECK(v31np::OsRelease("PRETTY_NAME=Gentoo\r\n") == "Gentoo");
        CHECK(v31np::OsRelease("PRETTY_NAME=\"a \\\"b\\\"\"\n") == "a \"b\"");
        CHECK(v31np::OsRelease("NAME=x\nXPRETTY_NAME=y\n").empty());
        CHECK(!v31np::System().empty());
        {
            v31np::Host host{ "pingu", "/home/pingu" };
            v31np::Hosting h;
            v31np::ReadSamba(
                "[global]\n   usershare path = /var/lib/x\n   include = /etc/samba/%U.conf\n"
                "[printers]\n   path = /var/tmp\n   printable = yes\n"
                "[print$]\n   Path = /var/lib/samba/printers\n"
                "; [old]\n;   path = /nope\n"
                "[off]\n   path = /off\n   available = no\n"
                "[homes]\n   browseable = no\n"
                "[team]\n   PATH = /srv/%S\\\n/data\n"
                "[guest]\n   directory = /srv/guest\n"
                "[bad]\n   path = /srv/%m\n"
                "[rel]\n   path = relative\n", host, h);
            CHECK(h.usershares == "/var/lib/x");
            CHECK(h.includes.size() == 1 && h.includes[0] == "/etc/samba/pingu.conf");
            const std::vector<std::string> want = { "/var/lib/samba/printers", "/home/pingu", "/srv/team/data", "/srv/guest" };
            CHECK(h.paths == want);
            v31np::Hosting homes;
            v31np::ReadSamba("[homes]\npath = /export/%U\n", host, homes);
            CHECK(homes.paths.size() == 1 && homes.paths[0] == "/export/pingu");
            v31np::Hosting u;
            v31np::ReadUsershare("#VERSION 2\npath=/home/pingu/Public\ncomment=\nusershare_acl=S-1-1-0:F\n", u);
            CHECK(u.paths.size() == 1 && u.paths[0] == "/home/pingu/Public");
            v31np::Hosting e;
            v31np::ReadExports("# x\n/srv/nfs  10.0.0.0/8(rw)\n\"/srv/with space\" *(ro)\n/srv/oct\\040al host(rw)\nrel host\n", e);
            const std::vector<std::string> ex = { "/srv/nfs", "/srv/with space", "/srv/oct al" };
            CHECK(e.paths == ex);
            CHECK(v31np::Within("/srv/team", "/srv/team"));
            CHECK(v31np::Within("/srv/team/a/b", "/srv/team/"));
            CHECK(!v31np::Within("/srv/teamwork", "/srv/team"));
            CHECK(!v31np::Within("/srv", "/srv/team"));
            CHECK(v31np::Within("/srv", "/"));
            CHECK(!v31np::Within("/srv", ""));
        }
        CHECK(v31np::PlateDigits("a") < 1000000000u);
        const std::string plate = v31np::Plate();
        const size_t dash = plate.rfind('-');
        CHECK(dash != std::string::npos && dash > 0 && plate.size() - dash - 1 <= 9 && plate.size() - dash - 1 >= 1);
        CHECK(!v31np::SharedRoad(L"/home/a/x.v31np"));
        CHECK(!v31np::SharedRoad(L"/tmp/x.v31np"));

        char dir[] = "/tmp/v31np-officer-XXXXXX";
        CHECK(mkdtemp(dir) != nullptr);
        const std::string post = std::string(dir) + "/NIEURFASMIT UUTI t.v31np.lock";
        {
            v31np::Signal a(v31np::Wide(post));
            v31np::Signal b(v31np::Wide(post));
            CHECK(a.Green(0));
            CHECK(a.IsGreen());
            CHECK(!b.Green(60));
            CHECK(a.ShowPapers("plate=x os=Ubuntu"));
            std::string papers;
            CHECK(b.PapersPlease(papers) && papers == "plate=x os=Ubuntu");
            CHECK(!b.ShowPapers("nope"));
            a.Red();
            CHECK(!a.IsGreen());
            CHECK(b.Green(0));
            CHECK(b.ShowPapers("p2"));
            CHECK(a.PapersPlease(papers) && papers == "p2");
        }
        {
            v31np::Signal c(v31np::Wide(post));
            CHECK(c.Green(0));
        }
        std::ifstream hidden(std::string(dir) + "/.hidden");
        std::string line;
        int count = 0;
        while (std::getline(hidden, line))
            if (line == "NIEURFASMIT UUTI t.v31np.lock") count++;
        CHECK(count == 1);
        std::remove(post.c_str());
        std::remove((std::string(dir) + "/.hidden").c_str());
        rmdir(dir);
    }

    void Plates()
    {
        const std::string text =
            "v31np shadow 2\t204\t133000000000000000\r\n"
            "G\t0\r\n"
            "N\t1\r\n"
            "S\t0\r\n"
            "@\r\n"
            "A\tdocs/note.txt\t5\t183\r\n"
            "F\tnew.txt\t3\t/home/a/new.txt\tpingu-346656037\r\n"
            "M\tm.txt\t2\tC:\\\\Users\\\\bob\\\\x.jpg\tDESKTOP-1\r\n"
            "T\tt.txt\t1\t/home/a/.cache/v31np/0.tmp\tpingu-346656037\r\n"
            "D\tempty\r\n"
            "R\t/home/a/moved\tpingu-346656037\r\n"
            "H\t/home/a/hidden\tpingu-346656037\r\n";
        v31np::Shadow s;
        CHECK(v31np::ReadJournal(text, s));
        const auto& top = s.parts[{}];
        CHECK(top.size() == 7);
        if (top.size() == 7)
        {
            CHECK(top[0].plate.empty() && top[1].plate == "pingu-346656037" && top[1].file == L"/home/a/new.txt");
            CHECK(top[2].plate == "DESKTOP-1" && top[2].file == L"C:\\Users\\bob\\x.jpg");
            CHECK(top[5].kind == 'R' && top[5].plate == "pingu-346656037" && top[6].kind == 'H' && top[6].plate == "pingu-346656037");
            CHECK(v31np::Ours(top[0], "DESKTOP-1") && v31np::Ours(top[4], "DESKTOP-1"));
            CHECK(v31np::Ours(top[1], "pingu-346656037") && !v31np::Ours(top[2], "pingu-346656037"));
        }
        CHECK(v31np::WriteJournal(s) == text);
        CHECK(v31np::AnyForeign(s, "pingu-346656037") && v31np::AnyForeign(s, "DESKTOP-1"));
        s.parts[{}].erase(s.parts[{}].begin() + 2);
        CHECK(!v31np::AnyForeign(s, "pingu-346656037") && v31np::AnyForeign(s, "DESKTOP-1"));

        v31np::Shadow bad;
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\n@\r\nF\tk\t1\t/x\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\n@\r\nT\tk\t1\t/x\t\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\n@\r\nR\t/x\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\n@\r\nH\t/x\t\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\n@\r\nM\tk\t1\t/x\tp\textra\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 1\t1\t2\r\n@\r\nF\tk\t1\t/x\tp\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 1\t1\t2\r\n@\r\nH\t/x\tp\r\n", bad));

        v31np::Shadow old;
        CHECK(v31np::ReadJournal("v31np shadow 1\t1\t2\r\n@\r\nF\tk\t1\t/x\r\nH\t/y\r\n", old));
        CHECK(old.parts[{}].size() == 2 && old.parts[{}][0].plate.empty() && !v31np::AnyForeign(old, "anyone"));
        CHECK(v31np::WriteJournal(old) == "v31np shadow 1\t1\t2\r\n@\r\nF\tk\t1\t/x\r\nH\t/y\r\n");
        old.version = 2;
        const std::string here = v31np::Plate();
        CHECK(v31np::WriteJournal(old) == "v31np shadow 2\t1\t2\r\nG\t0\r\nN\t1\r\nS\t0\r\n@\r\nF\tk\t1\t/x\t" + here + "\r\nH\t/y\t" + here + "\r\n");
        v31np::Shadow again;
        CHECK(v31np::ReadJournal(v31np::WriteJournal(old), again) && !v31np::AnyForeign(again, here) && v31np::AnyForeign(again, here + "x"));
    }

    void Shadow2()
    {
        const std::string text =
            "v31np shadow 2\t204\t133000000000000000\r\n"
            "G\t7\r\n"
            "N\t4\r\n"
            "S\t1\r\n"
            "L\t2\tDESKTOP-1\tWindows 10\t20261101PM090101\t20261101PM090301\t7\r\n"
            "Q\t3\tpingu-346656037\tUbuntu 26.04\t20261101PM090201\t20261101PM090401\t0\t7\t20261101PM090201\t20261101PM090213\tdocs/note.txt\tx\\ty\r\n"
            "W\t2\tDESKTOP-1\tWindows 10\t20261101PM090101\twave\r\n"
            "@\r\n"
            "A\tdocs/note.txt\t5\t183\r\n";
        v31np::Shadow s, bad0;
        CHECK(v31np::ReadJournal(text, s));
        CHECK(s.version == 2 && s.officer.shift == 7 && s.officer.next == 4 && s.officer.shared && s.officer.held);
        CHECK(s.officer.lane.ticket == 2 && s.officer.lane.papers.plate == "DESKTOP-1" && s.officer.lane.papers.os == "Windows 10" && s.officer.lane.shift == 7);
        CHECK(s.officer.lane.until - s.officer.lane.granted == 120);
        CHECK(s.officer.queue.size() == 1 && s.officer.queue[0].keys.size() == 2 && s.officer.queue[0].keys[1] == "x\ty");
        CHECK(s.officer.queue[0].clock - s.officer.queue[0].checked == 12);
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\nQ\t1\tp\tos\t20261101PM090101\t20261101PM090101\t0\t0\r\n", bad0));
        CHECK(s.officer.log.size() == 1 && s.officer.log[0].kind == "wave");
        CHECK(v31np::WriteJournal(s) == text);

        v31np::Shadow fresh;
        fresh.size = 67;
        std::string written = v31np::WriteJournal(fresh);
        CHECK(written == "v31np shadow 2\t67\t0\r\nG\t0\r\nN\t1\r\nS\t0\r\n");
        v31np::Shadow back;
        CHECK(v31np::ReadJournal(written, back) && back.version == 2 && !back.officer.held && back.parts.empty());

        v31np::Shadow bad;
        CHECK(!v31np::ReadJournal("v31np shadow 1\t1\t2\r\nG\t1\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\n@\r\nG\t1\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\nG\t1\r\nG\t2\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\nN\t0\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\nS\t2\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\nL\t1\tp\tos\t20261101PM090101\tsoon\t1\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\nQ\t1\t\tos\t20261101PM090101\t20261101PM090101\t0\t0\t20261101PM090101\t20261101PM090101\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 2\t1\t2\r\nZ\t1\r\n", bad));
        CHECK(!v31np::ReadJournal("v31np shadow 3\t1\t2\r\n", bad));
        CHECK(v31np::ReadJournal("v31np shadow 2\t1\t2\r\nQ\t1\tp\tos\t20261101PM090101\t20261101PM090101\t1\t0\t20261101PM090101\t20261101PM090101\r\nQ\t2\tq\tos\t20261101PM090101\t20261101PM090101\t0\t0\t20261101PM090101\t20261101PM090101\r\n", bad));
        CHECK(bad.officer.queue.size() == 2 && bad.officer.queue[0].keys.empty() && bad.officer.queue[0].urgent);
    }

    void Traffic()
    {
        using namespace v31np;
        Officer o;
        const Papers win{ "DESKTOP-1", "Windows 10" };
        const Papers lin{ "pingu-346656037", "Ubuntu 26.04" };
        const Papers lin2{ "kumpel-42", "Debian 13" };
        const int64_t t = 1793530000;

        uint64_t a = TakeTicket(o, win, t, Lease, false, { "docs/note.txt" }, t);
        uint64_t b = TakeTicket(o, lin, t + 1, Lease, false, { "photo.png" }, t + 1);
        CHECK(a == 1 && b == 2 && o.next == 3);
        CHECK(TakeTicket(o, lin, t + 2, Lease, false, { "photo.png" }, t + 2) == 2 && o.queue.size() == 2);
        CHECK(NextCar(o)->ticket == a);
        CHECK(Ahead(o, b) == 1);
        CHECK(!WaveOn(o, b, t + 2, Lease));
        CHECK(WaveOn(o, a, t + 2, Lease));
        const uint64_t shiftA = o.lane.shift;
        CHECK(o.held && o.lane.ticket == a && shiftA == 1 && o.shift == 1);
        CHECK(Ahead(o, b) == 1);
        CHECK(SameShift(o, a, shiftA) && !SameShift(o, b, shiftA));
        CHECK(Renew(o, a, shiftA, t + 50, Lease) && o.lane.until == t + 50 + Lease);

        uint64_t c = TakeTicket(o, lin2, t + 60, Lease, true, {}, t + 60);
        CHECK(NextCar(o)->ticket == c);
        CHECK(Ahead(o, b) == 2 && Ahead(o, c) == 1);
        CHECK(Leave(o, a, shiftA, t + 61) && !o.held);
        CHECK(!Leave(o, a, shiftA, t + 62));
        CHECK(WaveOn(o, c, t + 62, Lease) && o.shift == 2);
        CHECK(!SameShift(o, a, shiftA));

        Sweep(o, t + 62 + Lease + 1);
        CHECK(!o.held && o.shift == 3);
        CHECK(o.queue.empty());
        CHECK(!Renew(o, c, 2, t + 62 + Lease + 2, Lease));
        CHECK(o.log.back().kind == "expired");

        uint64_t d = TakeTicket(o, win, t + 500, Lease, false, {}, t + 500);
        uint64_t e = TakeTicket(o, lin, t + 501, Lease, false, {}, t + 501);
        CHECK(WaveOn(o, d, t + 502, Lease));
        const uint64_t shiftD = o.shift;
        CHECK(Tag(o, e, t + 503) && o.log.back().kind == "tag" && o.log.back().papers.plate == lin.plate);
        CHECK(!Tag(o, 999, t + 503));
        CHECK(Override(o, e, t + 504, Lease));
        CHECK(o.held && o.lane.ticket == e && o.shift == shiftD + 1);
        CHECK(!SameShift(o, d, shiftD) && !Leave(o, d, shiftD, t + 505));
        CHECK(o.log[o.log.size() - 3].kind == "overridden" && o.log[o.log.size() - 2].kind == "override");

        for (int i = 0; i < 40; i++) Withdraw(o, TakeTicket(o, lin2, t + 600 + i, Lease, false, {}, t + 600 + i), t + 600 + i);
        CHECK(o.log.size() == LogKeep);

        Shadow before, now;
        std::vector<Entry> stored = { { "docs/note.txt", 183, 5 }, { "docs/other.txt", 188, 3 }, { "photo.png", 191, 9 } };
        CHECK(Changed(before, now, stored, { "docs", "photo.png" }).empty());
        now.parts[{}] = { { 'A', "docs/note.txt", 5, 183, {}, {}, 0 }, { 'A', "docs/other.txt", 3, 188, {}, {}, 0 }, { 'F', "photo.png", 12, 0, L"/home/a/photo.png", {}, 0 } };
        auto changed = Changed(before, now, stored, { "docs", "photo.png" });
        CHECK(changed.size() == 1 && changed[0] == "photo.png");
        now.parts[{ "docs/inner.v31np" }] = { { 'D', "x", 0, 0, {}, {}, 0 } };
        changed = Changed(before, now, stored, { "docs", "photo.png", "nothing" });
        CHECK(changed.size() == 2 && changed[0] == "docs");
    }

    void CustodyNames()
    {
        auto c = v31np::NextCustody({ "docs/note.txt" }, "pingu-346656037", "2026");
        CHECK(c.action == 1 && c.list == 1);
        CHECK(c.actionKey == "ALICEV31NP/pingu-346656037\\1.txt" && c.listKey == "ALICEV31NP/2026\\1-1.txt");
        c = v31np::NextCustody({ "ALICEV31NP/pingu-346656037\\1.txt", "ALICEV31NP/pingu-346656037\\9.txt", "ALICEV31NP/DESKTOP-1\\30.txt", "ALICEV31NP/2026\\1-1.txt", "ALICEV31NP/2026\\2-30.txt", "ALICEV31NP/2025\\77-1.txt", "ALICEV31NP/photos/2026\\90-1.txt" }, "pingu-346656037", "2026");
        CHECK(c.action == 10 && c.list == 3);
        CHECK(c.actionKey == "ALICEV31NP/pingu-346656037\\10.txt" && c.listKey == "ALICEV31NP/2026\\3-10.txt");
        CHECK(v31np::Stamp(v31np::Now()).substr(0, 4) >= "2026");
    }

    void FolderLeases()
    {
        using namespace v31np;
        const std::string text =
            "v31np shadow 2\t67\t0\r\n"
            "G\t0\r\n"
            "N\t1\r\n"
            "S\t1\r\n"
            "@\r\n"
            "D\tnew folder\tpingu-346656037\t20261101PM090301\r\n"
            "D\tother\tDESKTOP-1\t20261101PM091001\r\n"
            "D\tlocal style\r\n";
        Shadow s;
        CHECK(ReadJournal(text, s));
        auto& rows = s.parts[{}];
        CHECK(rows.size() == 3 && rows[0].plate == "pingu-346656037" && rows[2].plate.empty());
        CHECK(WriteJournal(s) == text);

        Shadow old;
        CHECK(!ReadJournal("v31np shadow 1\t1\t2\r\n@\r\nD\tk\tp\t20261101PM090301\r\n", old));
        CHECK(!ReadJournal("v31np shadow 2\t1\t2\r\n@\r\nD\tk\t\t20261101PM090301\r\n", old));
        CHECK(!ReadJournal("v31np shadow 2\t1\t2\r\n@\r\nD\tk\tp\r\n", old));

        int64_t at = 0;
        CHECK(Unstamp("20261101PM090301", at));
        Shadow renewed = s;
        RenewFolders(renewed, "pingu-346656037", at - 10, Lease);
        CHECK(renewed.parts[{}][0].until == at - 10 + Lease && renewed.parts[{}][1].until == s.parts[{}][1].until);
        Shadow late = s;
        RenewFolders(late, "pingu-346656037", at + 1, Lease);
        CHECK(late.parts[{}][0].until == at);

        Shadow swept = s;
        Sweep(swept, at + 1);
        CHECK(swept.parts[{}].size() == 2 && swept.parts[{}][0].key == "other" && swept.parts[{}][1].key == "local style");
        Sweep(swept, at + 3600);
        CHECK(swept.parts[{}].size() == 1 && swept.parts[{}][0].key == "local style");

        std::vector<Row> land = s.parts[{}];
        Landed(land, "new folder/file.txt");
        CHECK(land.size() == 2 && land[0].key == "other");
        Landed(land, "otherwise.txt");
        CHECK(land.size() == 2);
        Landed(land, "other");
        CHECK(land.size() == 1);

        std::vector<Entry> stored;
        CHECK(Changed(s, renewed, stored, { "new folder" }).empty());
    }

    void Judging()
    {
        using namespace v31np;
        const Papers win{ "DESKTOP-1", "Windows 10.0.19045" };
        const Papers lin{ "pingu-346656037", "Ubuntu 26.04.1 LTS" };
        const Papers lin2{ "kumpel-42", "Debian 13" };
        const int64_t t = 1793530000;

        Officer fine;
        uint64_t a = TakeTicket(fine, win, t, Lease, false, {}, t + 59);
        CHECK(Judge(fine, a, t) == Verdict::Fine);
        CHECK(fine.log.empty());

        Officer lone;
        uint64_t b = TakeTicket(lone, lin, t, Lease, false, {}, t - 75);
        CHECK(Judge(lone, b, t) == Verdict::Condemned);
        CHECK(lone.log.size() == 1 && lone.log.back().kind == "condemn");
        CHECK(Judge(lone, b, t + 1) == Verdict::Condemned && lone.log.size() == 1);

        Officer mixed;
        uint64_t c = TakeTicket(mixed, win, t, Lease, false, {}, t + 2);
        uint64_t d = TakeTicket(mixed, lin, t, Lease, false, {}, t + 500);
        CHECK(Judge(mixed, c, t) == Verdict::Fine);
        CHECK(Judge(mixed, d, t) == Verdict::Refused && mixed.log.back().kind == "skew");
        CHECK(Assess(mixed, d) == Verdict::Refused && mixed.log.size() == 1);
        CHECK(NextCar(mixed)->ticket == c);
        Officer blocked;
        uint64_t front = TakeTicket(blocked, lin, t, Lease, true, {}, t + 500);
        uint64_t behind = TakeTicket(blocked, win, t, Lease, false, {}, t);
        CHECK(Assess(blocked, front) == Verdict::Refused);
        CHECK(NextCar(blocked)->ticket == behind);
        CHECK(!WaveOn(blocked, front, t, Lease));
        CHECK(WaveOn(blocked, behind, t, Lease));
        CHECK(Ahead(blocked, front) == 1);

        Officer all;
        uint64_t e = TakeTicket(all, win, t, Lease, false, {}, t + 3600);
        uint64_t f = TakeTicket(all, lin, t, Lease, false, {}, t + 3630);
        uint64_t g = TakeTicket(all, lin2, t, Lease, false, {}, t + 3590);
        CHECK(Judge(all, e, t) == Verdict::Condemned && Judge(all, f, t) == Verdict::Condemned && Judge(all, g, t) == Verdict::Condemned);
        CHECK(all.log.size() == 3 && all.log.back().kind == "condemn");

        Officer split;
        uint64_t h = TakeTicket(split, win, t, Lease, false, {}, t + 3600);
        TakeTicket(split, lin, t, Lease, false, {}, t - 3600);
        CHECK(Judge(split, h, t) == Verdict::Refused);
        CHECK(Judge(split, 999, t) == Verdict::Refused);

        Officer renew;
        uint64_t k = TakeTicket(renew, lin, t, Lease, false, {}, t - 300);
        CHECK(TakeTicket(renew, lin, t + 10, Lease, false, {}, t + 10) == k);
        CHECK(Judge(renew, k, t + 10) == Verdict::Fine);
    }

    void CustodyRecords()
    {
        using namespace v31np;
        Record r;
        r.papers = { "pingu-346656037", "Ubuntu 26.04.1 LTS" };
        CHECK(Unstamp("20261101PM100101", r.time));
        r.shift = 12;
        r.ticket = 7;
        r.action = "rename";
        r.lines = { "docs/old name.txt", "docs/new\tname\r\n.txt", "back\\slash" };
        const std::string text = CustodyText(r);
        CHECK(text == "pingu-346656037\r\nUbuntu 26.04.1 LTS\r\n20261101PM100101\r\n12\r\n7\r\nrename\r\ndocs/old name.txt\r\ndocs/new\\tname\\r\\n.txt\r\nback\\\\slash\r\n");
        Record back;
        CHECK(ReadCustody(text, back));
        CHECK(back.papers.plate == r.papers.plate && back.papers.os == r.papers.os && back.time == r.time && back.shift == 12 && back.ticket == 7 && back.action == "rename" && back.lines == r.lines);

        CHECK(!ReadCustody("", back));
        CHECK(!ReadCustody(text.substr(0, text.size() - 2), back));
        CHECK(!ReadCustody("p\r\nos\r\n20261101PM100101\r\n1\r\n1\r\nexplode\r\n", back));
        CHECK(!ReadCustody("p\r\nos\r\nyesterday\r\n1\r\n1\r\nadd\r\n", back));
        CHECK(!ReadCustody("p\r\nos\r\n20261101PM100101\r\n1\r\n1\r\nadd\r\nbad\\q\r\n", back));
        CHECK(!ReadCustody("p\nos\n20261101PM100101\n1\n1\nadd\n", back));
        CHECK(ReadCustody("p\r\n\r\n20261101PM100101\r\n1\r\n1\r\ndelete\r\n", back) && back.lines.empty());

        Car car;
        car.ticket = 9;
        car.papers = { "pingu-346656037", "Ubuntu 26.04.1 LTS" };
        CHECK(Unstamp("20261101PM100101", car.checked));
        car.clock = car.checked - 75;
        Record c = Citation(car, 13, car.checked, false);
        CHECK(c.action == "citation" && c.ticket == 9 && c.shift == 13 && c.time == car.checked);
        CHECK(c.lines.size() == 1 && c.lines[0] == "WARNING - CLOCK BEHIND 75s");
        Record cr;
        CHECK(ReadCustody(CustodyText(c), cr) && cr.lines == c.lines);
        CHECK(CustodyText(c) == "pingu-346656037\r\nUbuntu 26.04.1 LTS\r\n20261101PM100101\r\n13\r\n9\r\ncitation\r\nWARNING - CLOCK BEHIND 75s\r\n");
        car.clock = car.checked + 3600;
        CHECK(Citation(car, 13, car.checked, true).lines[0] == "WARNING - CLOCK AHEAD 3600s ALL");
    }
    struct Server
    {
        std::string journal;
        bool exists = false;
        uint64_t size = 500;
        uint64_t stamp = 7;
        uint64_t ms = 0;
        bool signal = false;
        std::string post;
        int64_t Time() const { return 1800000000 + (int64_t)(ms / 1000); }
    };

    struct FakeRoad : v31np::Road
    {
        Server& sv;
        int64_t skew = 0;
        bool answer = false;
        int asks = 0;
        v31np::Papers asked;
        bool cancel = false;
        int writes = 0;
        std::function<void()> tick;

        explicit FakeRoad(Server& s) : sv(s) {}
        bool Green(uint32_t) override
        {
            if (sv.signal) return false;
            sv.signal = true;
            return true;
        }
        void Red() override { sv.signal = false; }
        bool ShowPapers(const std::string& p) override
        {
            sv.post = p;
            return true;
        }
        bool ServerNow(int64_t& now) override
        {
            now = sv.Time();
            return true;
        }
        int64_t Clock() override { return sv.Time() + skew; }
        uint64_t Ticks() override { return sv.ms; }
        void Sleep(uint32_t ms) override
        {
            sv.ms += ms;
            if (sv.ms > 3600000) cancel = true;
            if (tick) tick();
        }
        bool Archive(uint64_t& size, uint64_t& stamp) override
        {
            size = sv.size;
            stamp = sv.stamp;
            return true;
        }
        bool ReadJournal(std::string& text, bool& exists) override
        {
            text = sv.journal;
            exists = sv.exists;
            return true;
        }
        bool WriteJournal(const std::string& text) override
        {
            sv.journal = text;
            sv.exists = true;
            writes++;
            return true;
        }
        bool Ask(const v31np::Papers& holder) override
        {
            asks++;
            asked = holder;
            return answer;
        }
        bool Cancelled() override { return cancel; }
    };

    struct FakeTrip : v31np::Trip
    {
        Server& sv;
        bool look = true;
        int steps = 1;
        std::function<void(int)> during;
        std::vector<v31np::Record> records;
        int looks = 0;
        int builds = 0;
        int lands = 0;
        int abandons = 0;

        explicit FakeTrip(Server& s) : sv(s) {}
        bool Look(const v31np::Shadow&) override
        {
            looks++;
            return look;
        }
        bool Build(const std::vector<v31np::Record>& r, const std::function<bool()>& renew) override
        {
            builds++;
            records = r;
            for (int i = 0; i < steps; i++)
            {
                sv.ms += 10000;
                if (during) during(i);
                if (!renew()) return false;
            }
            return true;
        }
        std::vector<v31np::Row> top;
        int64_t landed = 0;
        bool Land(v31np::Shadow& next, int64_t now) override
        {
            lands++;
            landed = now;
            sv.size += 100;
            sv.stamp++;
            next.size = sv.size;
            next.stamp = sv.stamp;
            if (!top.empty()) next.parts[{}] = top;
            return true;
        }
        void Abandon() override { abandons++; }
    };

    v31np::Shadow Seen(const Server& sv)
    {
        v31np::Shadow s;
        v31np::ReadJournal(sv.journal, s);
        return s;
    }

    size_t Events(const v31np::Shadow& s, const std::string& kind)
    {
        size_t n = 0;
        for (const auto& e : s.officer.log)
            if (e.kind == kind) n++;
        return n;
    }

    void Hold(Server& sv, const v31np::Papers& p)
    {
        FakeRoad other(sv);
        v31np::Visit(other, p, [&](v31np::Shadow& s, int64_t now)
        {
            const uint64_t t = v31np::TakeTicket(s.officer, p, now, v31np::Lease, false, {}, now);
            v31np::WaveOn(s.officer, t, now, v31np::Lease);
        });
    }

    void Roads()
    {
        const v31np::Papers me{ "pingu-1", "Ubuntu 26.04.1 LTS" };
        const v31np::Papers b{ "fenster-2", "Windows 10 Pro 10.0.19045" };
        const v31np::Papers c{ "third-3", "Fedora Linux 44" };
        using Lines = std::vector<std::string>;
        v31np::Request rq;
        rq.me = me;
        rq.steps = { { "rename", { "a.txt", "b.txt" } } };
        rq.keys = { "a.txt", "b.txt" };

        CHECK(v31np::PapersLine(me) == "plate=pingu-1 os=Ubuntu 26.04.1 LTS");
        CHECK(v31np::Year(1800000000) == "2027");

        {
            Server sv;
            FakeRoad road(sv);
            FakeTrip trip(sv);
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Done);
            const auto s = Seen(sv);
            CHECK(!s.officer.held && s.officer.queue.empty() && s.officer.shift == 1 && s.officer.shared);
            CHECK(Events(s, "wave") == 1 && Events(s, "leave") == 1);
            CHECK(s.size == 600 && s.stamp == 8);
            CHECK(sv.post == "plate=pingu-1 os=Ubuntu 26.04.1 LTS" && !sv.signal);
            CHECK(trip.records.size() == 1 && trip.records[0].action == "rename" && trip.records[0].lines == rq.steps[0].lines && trip.records[0].ticket == 1 && trip.records[0].shift == 1);
            CHECK(trip.records[0].papers.plate == me.plate && trip.records[0].time == 1800000000);
            CHECK(road.asks == 0 && trip.abandons == 0);
        }

        {
            Server sv;
            Hold(sv, b);
            FakeRoad road(sv);
            FakeTrip trip(sv);
            int64_t renewedB = sv.Time();
            road.tick = [&]()
            {
                FakeRoad other(sv);
                if (sv.Time() >= 1800000000 + 100)
                    v31np::Visit(other, b, [&](v31np::Shadow& s, int64_t now) { v31np::Leave(s.officer, s.officer.lane.ticket, s.officer.lane.shift, now); });
                else if (sv.Time() - renewedB >= 30)
                {
                    v31np::Visit(other, b, [&](v31np::Shadow& s, int64_t now) { v31np::Renew(s.officer, s.officer.lane.ticket, s.officer.lane.shift, now, v31np::Lease); });
                    renewedB = sv.Time();
                }
            };
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Done);
            const auto s = Seen(sv);
            CHECK(road.asks == 0 && Events(s, "override") == 0 && Events(s, "expired") == 0);
            CHECK(trip.records.size() == 1 && trip.records[0].time >= 1800000000 + 100);
        }

        {
            Server sv;
            Hold(sv, b);
            FakeRoad road(sv);
            road.answer = true;
            FakeTrip trip(sv);
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Done);
            const auto s = Seen(sv);
            CHECK(road.asks == 1 && road.asked.plate == b.plate && road.asked.os == b.os);
            CHECK(Events(s, "overridden") == 1 && Events(s, "override") == 1 && Events(s, "expired") == 0);
            CHECK(trip.records.size() == 2 && trip.records[0].action == "override" && (trip.records[0].lines == std::vector<std::string>{ b.plate, b.os }));
            CHECK(trip.records.size() == 2 && trip.records[0].time >= 1800000000 + v31np::Missed && trip.records[0].time < 1800000000 + v31np::Lease);
            CHECK(trip.records.size() == 2 && trip.records[1].action == "rename");
        }

        {
            Server sv;
            Hold(sv, b);
            FakeRoad road(sv);
            road.answer = false;
            FakeTrip trip(sv);
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Done);
            const auto s = Seen(sv);
            CHECK(road.asks == 1 && Events(s, "override") == 0 && Events(s, "expired") == 1);
            CHECK(trip.records.size() == 1 && trip.records[0].time > 1800000000 + v31np::Lease);
        }

        {
            Server sv;
            Hold(sv, b);
            FakeRoad road(sv);
            road.answer = true;
            FakeTrip trip(sv);
            road.tick = [&]()
            {
                FakeRoad other(sv);
                v31np::Visit(other, c, [&](v31np::Shadow& s, int64_t now)
                {
                    const uint64_t t = v31np::TakeTicket(s.officer, c, now, v31np::Lease, false, {}, now);
                    if (v31np::WaveOn(s.officer, t, now, v31np::Lease)) v31np::Leave(s.officer, t, s.officer.shift, now);
                });
            };
            road.tick();
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Done);
            const auto s = Seen(sv);
            CHECK(road.asks == 0 && Events(s, "override") == 0);
            CHECK(Events(s, "tag") == 1 && Events(s, "expired") == 1);
        }

        {
            Server sv;
            FakeRoad road(sv);
            road.skew = 100;
            FakeTrip trip(sv);
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Done);
            const auto s = Seen(sv);
            CHECK(Events(s, "condemn") == 1);
            CHECK(trip.records.size() == 2 && trip.records[0].action == "citation" && trip.records[0].lines == Lines{ "WARNING - CLOCK AHEAD 100s" });
        }

        {
            Server sv;
            FakeRoad road(sv);
            road.skew = -75;
            FakeTrip trip(sv);
            const v31np::Papers d{ "slow-4", "Debian GNU/Linux 13" };
            auto join = [&](int64_t off)
            {
                FakeRoad other(sv);
                v31np::Visit(other, d, [&](v31np::Shadow& s, int64_t now) { v31np::TakeTicket(s.officer, d, now, v31np::Lease, false, {}, now + off); });
            };
            Hold(sv, b);
            road.tick = [&]()
            {
                join(-80);
                if (sv.Time() < 1800000000 + 30) return;
                FakeRoad other(sv);
                v31np::Visit(other, b, [&](v31np::Shadow& s, int64_t now)
                {
                    if (s.officer.held && s.officer.lane.papers.plate == b.plate) v31np::Leave(s.officer, s.officer.lane.ticket, s.officer.lane.shift, now);
                });
            };
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Done);
            CHECK(road.asks == 0 && Seen(sv).officer.queue.size() == 1 && Seen(sv).officer.queue[0].papers.plate == d.plate);
            CHECK(trip.records.size() == 2 && trip.records[0].lines == Lines{ "WARNING - CLOCK BEHIND 75s ALL" });
        }

        {
            Server sv;
            FakeRoad road(sv);
            road.skew = 100;
            FakeTrip trip(sv);
            auto fine = [&]()
            {
                FakeRoad other(sv);
                v31np::Visit(other, c, [&](v31np::Shadow& s, int64_t now) { v31np::TakeTicket(s.officer, c, now, v31np::Lease, false, {}, now); });
            };
            fine();
            road.tick = fine;
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Skewed);
            const auto s = Seen(sv);
            CHECK(Events(s, "skew") >= 1 && Events(s, "withdraw") == 1 && trip.builds == 0);
            bool mine = false;
            for (const auto& car : s.officer.queue) mine = mine || car.papers.plate == me.plate;
            CHECK(!mine && sv.Time() >= 1800000000 + v31np::Lease && sv.Time() < 1800000000 + v31np::Lease + 2);
        }

        {
            Server sv;
            FakeRoad road(sv);
            FakeTrip trip(sv);
            trip.steps = 20;
            trip.during = [&](int)
            {
                if (sv.Time() < 1800000000 + 50) return;
                FakeRoad other(sv);
                v31np::Visit(other, b, [&](v31np::Shadow& s, int64_t now)
                {
                    if (!s.officer.held || s.officer.lane.papers.plate != me.plate) return;
                    const uint64_t t = v31np::TakeTicket(s.officer, b, now, v31np::Lease, false, {}, now);
                    v31np::Override(s.officer, t, now, v31np::Lease);
                });
            };
            const uint64_t size = sv.size;
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Lost);
            CHECK(trip.abandons == 1 && trip.lands == 0 && sv.size == size);
            const auto s = Seen(sv);
            CHECK(s.officer.held && s.officer.lane.papers.plate == b.plate);
        }

        {
            Server sv;
            FakeRoad road(sv);
            FakeTrip trip(sv);
            trip.steps = 2;
            trip.during = [&](int i)
            {
                if (i != 0) return;
                FakeRoad other(sv);
                v31np::Visit(other, b, [&](v31np::Shadow& s, int64_t now)
                {
                    const uint64_t t = v31np::TakeTicket(s.officer, b, now, v31np::Lease, false, {}, now);
                    v31np::Override(s.officer, t, now, v31np::Lease);
                });
            };
            const uint64_t size = sv.size;
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Lost);
            CHECK(trip.builds == 1 && trip.lands == 0 && trip.abandons == 1 && sv.size == size);
            const auto s = Seen(sv);
            CHECK(s.officer.held && s.officer.lane.papers.plate == b.plate && Events(s, "leave") == 0);
        }

        {
            Server sv;
            FakeRoad road(sv);
            FakeTrip trip(sv);
            trip.look = false;
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Refused);
            const auto s = Seen(sv);
            CHECK(trip.builds == 0 && trip.lands == 0 && !s.officer.held && Events(s, "leave") == 1 && sv.size == 500);
        }

        {
            Server sv;
            FakeRoad road(sv);
            FakeTrip trip(sv);
            trip.steps = 20;
            const int before = road.writes;
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Done);
            const auto s = Seen(sv);
            CHECK(road.writes - before == 1 + 5 + 1 && Events(s, "expired") == 0 && Events(s, "leave") == 1);
        }

        {
            Server sv;
            FakeRoad road(sv);
            FakeTrip trip(sv);
            trip.top = { { 'A', "x.txt", 1, 0, {}, {}, 0 }, { 'D', "mine", 0, 0, {}, {}, 0 }, { 'D', "theirs", 0, 0, {}, b.plate, 1800000000 + 30 } };
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Done);
            const auto s = Seen(sv);
            const auto it = s.parts.find({});
            CHECK(it != s.parts.end() && it->second.size() == 3);
            if (it != s.parts.end() && it->second.size() == 3)
            {
                CHECK(it->second[1].plate == me.plate && it->second[1].until == 1800000000 + 10 + v31np::Lease);
                CHECK(it->second[2].plate == b.plate && it->second[2].until == 1800000000 + 30);
            }
        }

        {
            Server sv;
            FakeRoad road(sv);
            FakeTrip trip(sv);
            v31np::Request two = rq;
            two.steps = { { "delete", { "b.txt" } }, { "rename", { "a.txt", "b.txt" } } };
            CHECK(v31np::Drive(road, trip, two) == v31np::Drove::Done);
            CHECK(trip.records.size() == 2);
            if (trip.records.size() == 2)
            {
                CHECK(trip.records[0].action == "delete" && trip.records[0].lines == Lines{ "b.txt" });
                CHECK(trip.records[1].action == "rename" && trip.records[1].lines == (Lines{ "a.txt", "b.txt" }));
                CHECK(trip.records[0].ticket == trip.records[1].ticket && trip.records[0].shift == trip.records[1].shift && trip.records[0].time == trip.records[1].time);
            }
            CHECK(trip.lands == 1 && trip.landed == 1800000000 + 10);
        }

        {
            Server sv;
            Hold(sv, b);
            FakeRoad road(sv);
            road.answer = true;
            FakeTrip trip(sv);
            v31np::Request two = rq;
            two.steps = { { "add", { "n.v31np" } }, { "delete", { "a.txt", "b.txt" } } };
            CHECK(v31np::Drive(road, trip, two) == v31np::Drove::Done);
            CHECK(trip.records.size() == 3);
            if (trip.records.size() == 3)
                CHECK(trip.records[0].action == "override" && trip.records[1].action == "add" && trip.records[2].action == "delete" && trip.records[2].lines == (Lines{ "a.txt", "b.txt" }));
            CHECK(trip.landed >= 1800000000 + v31np::Missed);
        }

        {
            Server sv;
            FakeRoad road(sv);
            FakeTrip trip(sv);
            road.cancel = true;
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Cancelled);
            const auto s = Seen(sv);
            CHECK(s.officer.queue.empty() && !s.officer.held && trip.looks == 0);
        }

        {
            Server sv;
            Hold(sv, b);
            v31np::Shadow j = Seen(sv);
            j.size = 499;
            j.parts[{}] = { { 'A', "x.txt", 1, 0, {}, {}, 0 }, { 'D', "empty", 0, 0, {}, b.plate, 1800000000 + 100 } };
            sv.journal = v31np::WriteJournal(j);
            FakeRoad road(sv);
            v31np::Shadow got;
            CHECK(v31np::Visit(road, me, [&](v31np::Shadow& s, int64_t) { got = s; }));
            CHECK(got.parts.empty() && got.officer.held && got.officer.lane.papers.plate == b.plate && got.size == 500 && got.stamp == 7);
            j.size = 500;
            j.parts[{}].push_back({ 'T', "t.txt", 1, 0, L"/tmp/x.tmp", me.plate, 0 });
            sv.journal = v31np::WriteJournal(j);
            CHECK(v31np::Visit(road, me, [&](v31np::Shadow& s, int64_t) { got = s; }));
            CHECK(got.parts.empty() && !got.officer.held && got.officer.next == 1);
            j.parts[{}].pop_back();
            sv.journal = v31np::WriteJournal(j);
            CHECK(v31np::Visit(road, me, [&](v31np::Shadow& s, int64_t) { got = s; }));
            CHECK(got.parts.size() == 1 && got.parts[{}].size() == 2 && got.officer.held);
        }

        {
            Server sv;
            sv.signal = true;
            FakeRoad road(sv);
            FakeTrip trip(sv);
            CHECK(v31np::Drive(road, trip, rq) == v31np::Drove::Failed);
            CHECK(trip.looks == 0 && !sv.exists);
        }

        {
            v31np::Record r1{ me, 1800000000, 3, 9, "citation", { "WARNING - CLOCK AHEAD 100s" } };
            v31np::Record r2{ me, 1800000000, 3, 9, "rename", { "a.txt", "b.txt" } };
            const auto files = v31np::CustodyFiles({ "ALICEV31NP/pingu-1\\4.txt", "ALICEV31NP/2027\\2-4.txt", "a.txt" }, { r1, r2 });
            CHECK(files.size() == 4);
            CHECK(files[0].first == "ALICEV31NP/pingu-1\\5.txt" && files[0].second == v31np::CustodyText(r1));
            CHECK(files[1].first == "ALICEV31NP/2027\\3-5.txt" && files[1].second == "ALICEV31NP/pingu-1\\\\5.txt\r\n" + v31np::CustodyText(r1));
            CHECK(files[2].first == "ALICEV31NP/pingu-1\\6.txt" && files[3].first == "ALICEV31NP/2027\\4-6.txt");
            v31np::Record back;
            CHECK(v31np::ReadCustody(files[2].second, back) && back.action == "rename" && back.lines == r2.lines);
        }
    }

}

int main()
{
    EmptyArchive();
    OneFile();
    HeaderLength();
    Refusals();
    Suffixes();
    Names();
    Trees();
    Order();
    Journal();
    SharedListing();
    Propertime();
    Signals();
    Shadow2();
    Plates();
    Traffic();
    CustodyNames();
    FolderLeases();
    Judging();
    CustodyRecords();
    Roads();
    std::printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
