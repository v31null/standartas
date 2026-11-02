#include "../mount/change.h"

#include <cstdio>
#include <string>
#include <vector>

namespace
{
    int checks = 0;
    int failures = 0;

#define CHECK(x) Check((x), #x, __LINE__)

    void Check(bool ok, const char* what, int line)
    {
        checks++;
        if (ok) return;
        failures++;
        std::printf("FAIL line %d: %s\n", line, what);
    }

    using Rows = std::vector<v31np::Row>;

    v31np::Row A(const std::string& key, uint64_t at = 0)
    {
        return { 'A', key, 1, at, {}, {}, 0 };
    }

    v31np::Row T(const std::string& key, const std::wstring& file)
    {
        return { 'T', key, 0, 0, file, {}, 0 };
    }

    std::string Show(const Rows& rows)
    {
        std::string out;
        for (const auto& r : rows)
        {
            if (!out.empty()) out += ' ';
            out += r.kind;
            out += ':';
            out += r.key;
        }
        return out;
    }

    void Paths()
    {
        CHECK(change::Split("a/b") == change::Path({ "a", "b" }));
        CHECK(change::Split("/a//b/") == change::Path({ "a", "b" }));
        CHECK(change::Split("").empty());
        CHECK(change::Join({ "a", "b" }) == "a/b");
        CHECK(change::Join({}).empty());
        CHECK(change::Within("a/b", { "a" }) && change::Within("a", { "a" }) && change::Within("a", {}));
        CHECK(!change::Within("ab/x", { "a" }));
        CHECK(change::Within("/a//b", { "a", "b" }));
        CHECK(change::Below("a/b", { "a" }) && !change::Below("a", { "a" }));
    }

    void Puts()
    {
        Rows r = { A("a.txt"), A("docs/x"), { 'D', "docs/empty", 0, 0, {}, {}, 0 } };
        change::Put(r, T("new.txt", L"/c/1.tmp"));
        CHECK(Show(r) == "A:a.txt A:docs/x D:docs/empty T:new.txt");

        change::Put(r, T("a.txt", L"/c/2.tmp"));
        CHECK(Show(r) == "A:docs/x D:docs/empty T:new.txt T:a.txt");

        change::Put(r, T("docs/empty/f", L"/c/3.tmp"));
        CHECK(Show(r) == "A:docs/x T:new.txt T:a.txt T:docs/empty/f");

        change::Put(r, T("docs", L"/c/4.tmp"));
        CHECK(Show(r) == "T:new.txt T:a.txt T:docs");

        change::Put(r, T("docs/inner", L"/c/5.tmp"));
        CHECK(Show(r) == "T:new.txt T:a.txt T:docs/inner");

        Rows keep = { A("x"), { 'R', "", 0, 0, L"/home/a/moved", {}, 0 }, { 'H', "", 0, 0, L"/home/a/h", {}, 0 } };
        change::Put(keep, T("x", L"/c/6.tmp"));
        CHECK(keep.size() == 3 && keep[0].kind == 'R' && keep[1].kind == 'H' && keep[2].kind == 'T');
    }

    void Removes()
    {
        Rows r = { A("docs/x"), A("docs/y"), A("top") };
        change::Remove(r, { "docs", "x" });
        CHECK(Show(r) == "A:docs/y A:top");
        change::Remove(r, { "docs", "y" });
        CHECK(Show(r) == "A:top D:docs");
        change::Remove(r, { "top" });
        CHECK(Show(r) == "D:docs");
        change::Remove(r, { "docs" });
        CHECK(r.empty());

        Rows deep = { A("a/b/c/f"), A("a/g") };
        change::Remove(deep, { "a", "b" });
        CHECK(Show(deep) == "A:a/g");
        change::Remove(deep, { "a", "g" });
        CHECK(Show(deep) == "D:a");

        Rows odd = { A("/a//f"), A("a/g") };
        change::Remove(odd, { "a", "f" });
        CHECK(Show(odd) == "A:a/g");

        Rows prefix = { A("ab/x"), A("a/y") };
        change::Remove(prefix, { "a" });
        CHECK(Show(prefix) == "A:ab/x");
    }

    void Dirs()
    {
        Rows r = { { 'D', "a", 0, 0, {}, {}, 0 } };
        change::MakeDir(r, { "a", "b" });
        CHECK(Show(r) == "D:a/b");
        change::MakeDir(r, { "a", "b" });
        CHECK(Show(r) == "D:a/b");
        change::MakeDir(r, { "c" });
        CHECK(Show(r) == "D:a/b D:c");
    }

    void Moves()
    {
        Rows r = { A("docs/x"), A("docs/sub/y"), { 'D', "docs/e", 0, 0, {}, {}, 0 }, A("docsx"), A("other/z") };
        change::Move(r, { "docs" }, { "papers" });
        CHECK(Show(r) == "A:papers/x A:papers/sub/y D:papers/e A:docsx A:other/z");

        change::Move(r, { "other", "z" }, { "papers", "z" });
        CHECK(Show(r) == "A:papers/x A:papers/sub/y D:papers/e A:docsx A:papers/z D:other");

        Rows into = { A("f"), { 'D', "box", 0, 0, {}, {}, 0 } };
        change::Move(into, { "f" }, { "box", "f" });
        CHECK(Show(into) == "A:box/f");

        Rows same = { A("d/a"), A("d/b") };
        change::Move(same, { "d", "a" }, { "d", "c" });
        CHECK(Show(same) == "A:d/c A:d/b");

        Rows kept = { A("k", 77) };
        change::Move(kept, { "k" }, { "m" });
        CHECK(kept.size() == 1 && kept[0].at == 77 && kept[0].size == 1);
    }

    void Refs()
    {
        Rows r = { T("a", L"/c/1.tmp"), T("b", L"/c/1.tmp"), A("x") };
        CHECK(change::Refers(r, L"/c/1.tmp"));
        CHECK(!change::Refers(r, L"/c/2.tmp"));
        change::Remove(r, { "a" });
        CHECK(change::Refers(r, L"/c/1.tmp"));
        change::Remove(r, { "b" });
        CHECK(!change::Refers(r, L"/c/1.tmp"));
    }

    void Sections()
    {
        change::Parts parts;
        parts[{}] = { A("box/mid.v31np"), A("top.txt") };
        parts[{ "box/mid.v31np" }] = { A("deep.v31np"), T("new.txt", L"/c/1.tmp") };
        parts[{ "box/mid.v31np", "deep.v31np" }] = { A("z/bottom.txt") };
        parts[{ "boxer.v31np" }] = { A("q") };

        CHECK(change::Refers(parts, L"/c/1.tmp"));
        CHECK(!change::Refers(parts, L"/c/2.tmp"));

        change::Prune(parts);
        CHECK(parts.size() == 3 && parts.count({ "boxer.v31np" }) == 0);

        change::Rekey(parts, {}, { "box" }, { "crate" });
        CHECK(parts.count({ "crate/mid.v31np" }) == 1 && parts.count({ "crate/mid.v31np", "deep.v31np" }) == 1);
        CHECK(parts.count({ "box/mid.v31np" }) == 0);
        CHECK(Show(parts[{ "crate/mid.v31np", "deep.v31np" }]) == "A:z/bottom.txt");

        change::Rekey(parts, { "crate/mid.v31np" }, { "deep.v31np" }, { "sub", "deeper.v31np" });
        CHECK(parts.count({ "crate/mid.v31np", "sub/deeper.v31np" }) == 1);

        change::Rekey(parts, {}, { "cr" }, { "xx" });
        CHECK(parts.count({ "crate/mid.v31np" }) == 1);

        change::Prune(parts);
        CHECK(parts.size() == 1);

        change::Parts kept;
        kept[{}] = { A("a.v31np") };
        kept[{ "a.v31np" }] = { A("in") };
        kept[{ "a.v31np", "in" }] = { A("x") };
        kept[{ "a.v31np", "gone", "x" }] = { A("y") };
        change::Prune(kept);
        CHECK(kept.size() == 3 && kept.count({ "a.v31np", "in" }) == 1 && kept.count({ "a.v31np", "gone", "x" }) == 0);

        change::Parts held;
        held[{}] = { T("n.v31np", L"/c/3.tmp"), { 'D', "e.v31np", 0, 0, {}, {}, 0 } };
        held[{ "n.v31np" }] = { A("x") };
        held[{ "e.v31np" }] = { A("y") };
        change::Prune(held);
        CHECK(held.size() == 2 && held.count({ "n.v31np" }) == 1 && held.count({ "e.v31np" }) == 0);
    }
}

int main()
{
    Paths();
    Puts();
    Removes();
    Dirs();
    Moves();
    Refs();
    Sections();
    std::printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
