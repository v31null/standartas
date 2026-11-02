#include "propertime.h"

#include <chrono>
#include <cstdio>

namespace v31np
{
    namespace
    {
        constexpr int64_t Japan = 9 * 3600;
        constexpr int64_t Day = 86400;

        int64_t DaysFromCivil(int64_t y, unsigned m, unsigned d)
        {
            y -= m <= 2;
            const int64_t era = (y >= 0 ? y : y - 399) / 400;
            const unsigned yoe = (unsigned)(y - era * 400);
            const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
            const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
            return era * 146097 + (int64_t)doe - 719468;
        }

        void CivilFromDays(int64_t z, int64_t& y, unsigned& m, unsigned& d)
        {
            z += 719468;
            const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
            const unsigned doe = (unsigned)(z - era * 146097);
            const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
            const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
            const unsigned mp = (5 * doy + 2) / 153;
            d = doy - (153 * mp + 2) / 5 + 1;
            m = mp < 10 ? mp + 3 : mp - 9;
            y = (int64_t)yoe + era * 400 + (m <= 2);
        }

        int64_t FloorDiv(int64_t a, int64_t b)
        {
            return a / b - ((a % b != 0) && ((a < 0) != (b < 0)));
        }

        bool Digits(const std::string& s, size_t at, size_t n, unsigned& v)
        {
            if (at + n > s.size()) return false;
            v = 0;
            for (size_t i = at; i < at + n; i++)
            {
                if (s[i] < '0' || s[i] > '9') return false;
                v = v * 10 + (unsigned)(s[i] - '0');
            }
            return true;
        }

        unsigned MonthDays(int64_t y, unsigned m)
        {
            static const unsigned days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            const bool leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
            return m == 2 && leap ? 29 : days[m - 1];
        }
    }

    std::string Stamp(int64_t unixSeconds)
    {
        const int64_t local = unixSeconds + Japan;
        const int64_t days = FloorDiv(local, Day);
        const int64_t secs = local - days * Day;
        int64_t y;
        unsigned m, d;
        CivilFromDays(days, y, m, d);
        const unsigned h = (unsigned)(secs / 3600);
        const unsigned mi = (unsigned)(secs / 60 % 60);
        const unsigned s = (unsigned)(secs % 60);
        unsigned label = mi + 1;
        unsigned sec = s + 1;
        if (h == 23 && mi >= 56)
        {
            label = 57;
            sec = (mi - 56) * 60 + s + 1;
        }
        char buf[64];
        if (sec >= 100)
            std::snprintf(buf, sizeof buf, "%04lld%02u%02u%c%02u%02u%03u", (long long)y, m, d, h >= 12 ? 'P' : 'A', h % 12, label, sec);
        else
            std::snprintf(buf, sizeof buf, "%04lld%02u%02u%s%02u%02u%02u", (long long)y, m, d, h >= 12 ? "PM" : "AM", h % 12, label, sec);
        return buf;
    }

    bool Unstamp(const std::string& stamp, int64_t& unixSeconds)
    {
        size_t mer = stamp.find_first_of("AP");
        if (mer == std::string::npos || mer < 8) return false;
        const bool pm = stamp[mer] == 'P';
        const bool full = mer + 1 < stamp.size() && stamp[mer + 1] == 'M';
        std::string digits = stamp.substr(0, mer);
        std::string after = stamp.substr(mer + (full ? 2 : 1));
        std::string date, time;
        if (after.size() >= 6)
        {
            date = digits;
            time = after;
        }
        else if (after.empty())
        {
            const size_t len = pm && digits.size() >= 15 && digits.compare(digits.size() - 7, 4, "1157") == 0 && !full ? 7 : 6;
            if (digits.size() < 8 + len) return false;
            date = digits.substr(0, digits.size() - len);
            time = digits.substr(digits.size() - len);
        }
        else return false;
        if (date.size() < 5 || (time.size() != 6 && time.size() != 7)) return false;
        if (time.size() == 7 && full) return false;
        if (time.size() == 6 && !full) return false;

        unsigned y4, m, d, h, mi, s;
        if (!Digits(date, 0, date.size() - 4, y4) || !Digits(date, date.size() - 4, 2, m) || !Digits(date, date.size() - 2, 2, d)) return false;
        if (!Digits(time, 0, 2, h) || !Digits(time, 2, 2, mi) || !Digits(time, 4, time.size() - 4, s)) return false;
        const int64_t y = y4;
        if (m < 1 || m > 12 || d < 1 || d > MonthDays(y, m) || h > 11 || mi < 1 || s < 1) return false;

        const bool leapMinute = pm && h == 11 && mi == 57;
        if (leapMinute ? s > 240 : (mi > 60 || s > 60)) return false;
        if (time.size() == 7 && !leapMinute) return false;

        int64_t secs = (int64_t)(h + (pm ? 12 : 0)) * 3600;
        if (leapMinute) secs += 56 * 60 + (int64_t)s - 1;
        else
        {
            if (pm && h == 11 && mi > 57) return false;
            secs += (int64_t)(mi - 1) * 60 + (int64_t)s - 1;
        }
        unixSeconds = DaysFromCivil(y, m, d) * Day + secs - Japan;
        return true;
    }

    int64_t Now()
    {
        using namespace std::chrono;
        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }
}
