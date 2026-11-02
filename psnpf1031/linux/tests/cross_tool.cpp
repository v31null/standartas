#include "officer.h"
#include "propertime.h"
#include "tree.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>

namespace
{
    void Say(const std::string& what)
    {
        std::printf("%lld %s\n", (long long)v31np::Now(), what.c_str());
        std::fflush(stdout);
    }

    std::string Papers()
    {
        return "plate=" + v31np::Plate() + " os=Ubuntu";
    }

    int Hold(const std::wstring& post, int seconds)
    {
        v31np::Signal signal(post);
        if (!signal.Green(10000))
        {
            Say("RED could not take the signal in 10 s");
            return 1;
        }
        Say("GREEN");
        if (!signal.ShowPapers(Papers())) Say("papers not written");
        int64_t server = 0;
        if (signal.ServerNow(server)) Say("server " + std::to_string(server) + " offset " + std::to_string(server - v31np::Now()));
        else Say("server clock unreadable");
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        signal.Red();
        Say("RED released");
        return 0;
    }

    int Probe(const std::wstring& post, int waitMs)
    {
        v31np::Signal signal(post);
        std::string papers;
        signal.PapersPlease(papers);
        if (signal.Green((uint32_t)waitMs))
        {
            Say("GREEN after waiting up to " + std::to_string(waitMs) + " ms; papers were '" + papers + "'");
            signal.Red();
            return 0;
        }
        Say("RED; holder papers '" + papers + "'");
        return 3;
    }
}

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        std::fprintf(stderr, "usage: cross_tool hold <post> <seconds> | probe <post> <wait ms>\n");
        return 2;
    }
    const std::string mode = argv[1];
    const std::wstring post = v31np::Wide(argv[2]);
    const int n = std::atoi(argv[3]);
    if (mode == "hold") return Hold(post, n);
    if (mode == "probe") return Probe(post, n);
    return 2;
}
