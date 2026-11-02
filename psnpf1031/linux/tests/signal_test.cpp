#include "officer.h"
#include "propertime.h"
#include "tree.h"

#include <cstdio>
#include <string>
#include <unistd.h>

int main(int argc, char** argv)
{
    if (argc != 3) return 2;
    const std::string name = "NIEURFASMIT UUTI road.v31np.lock";
    const std::wstring a = v31np::Wide(std::string(argv[1]) + "/" + name);
    const std::wstring b = v31np::Wide(std::string(argv[2]) + "/" + name);
    int bad = 0;
    auto check = [&](bool ok, const char* what)
    {
        std::printf("%s %s\n", ok ? "ok  " : "FAIL", what);
        if (!ok) bad++;
    };
    check(v31np::SharedRoad(v31np::Wide(std::string(argv[1]) + "/road.v31np")), "SharedRoad on the share");
    v31np::Signal first(a);
    v31np::Signal second(b);
    std::string papers;
    second.PapersPlease(papers);
    check(first.Green(0), "A green");
    check(!second.Green(200), "B stays red while A is green, across sessions");
    for (int i = 0; i < 10; i++)
    {
        const std::string p = "plate=" + v31np::Plate() + " os=Ubuntu round=" + std::to_string(i);
        first.ShowPapers(p);
        std::string seen;
        if (!second.PapersPlease(seen) || seen != p)
        {
            check(false, ("B reads A's fresh papers, round " + std::to_string(i) + " got '" + seen + "'").c_str());
            break;
        }
        if (i == 9) check(true, "B reads A's fresh papers 10 of 10 times, right after the write");
    }
    int64_t server = 0;
    check(first.ShowPapers("plate=" + v31np::Plate()) && first.ServerNow(server), "A reads the server clock after writing papers");
    const int64_t here = v31np::Now();
    check(server > 0 && server - here <= 2 && here - server <= 2, ("server clock within 2 s of this machine: " + std::to_string(server - here) + " s").c_str());
    check(!second.ServerNow(server), "B cannot read the server clock while red");
    first.Red();
    check(second.Green(500), "B green after A red");
    check(!first.Green(100), "A red while B green");
    second.Red();
    return bad == 0 ? 0 : 1;
}
