#include "Features/Leaderboards/UI/ScoreDetails/ScoreAgeFormatter.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fmt/format.h>
#include <optional>
#include <vector>

namespace SnoreSaber::UI::Other::ScoreAgeFormatter
{
    namespace
    {
        int DaysFromCivil(int year, unsigned month, unsigned day)
        {
            year -= month <= 2;
            int era = (year >= 0 ? year : year - 399) / 400;
            unsigned yearOfEra = static_cast<unsigned>(year - era * 400);
            unsigned dayOfYear = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
            unsigned dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
            return era * 146097 + static_cast<int>(dayOfEra) - 719468;
        }

        std::optional<double> TryParseScoreTimestamp(std::string_view createdAt)
        {
            int year = 0;
            int month = 0;
            int day = 0;
            int hour = 0;
            int minute = 0;
            int second = 0;
            int millisecond = 0;

            std::string text(createdAt);
            int parsed = std::sscanf(text.c_str(), "%d-%d-%dT%d:%d:%d.%dZ", &year, &month, &day, &hour, &minute, &second, &millisecond);
            if (parsed < 6)
            {
                return std::nullopt;
            }

            double days = DaysFromCivil(year, static_cast<unsigned>(month), static_cast<unsigned>(day));
            return days * 86400.0 + hour * 3600.0 + minute * 60.0 + second + millisecond / 1000.0;
        }

        std::string FormatValue(double value)
        {
            if (std::floor(value) == value)
            {
                return fmt::format("{:.0f}", value);
            }

            auto formatted = fmt::format("{:.3f}", value);
            while (!formatted.empty() && formatted.back() == '0')
            {
                formatted.pop_back();
            }
            if (!formatted.empty() && formatted.back() == '.')
            {
                formatted.pop_back();
            }
            return formatted;
        }

        void AddPart(std::vector<std::string>& parts, double value, std::string_view unit)
        {
            parts.push_back(fmt::format("{} {}{}", FormatValue(value), unit, value > 1 ? "s" : ""));
        }

        bool TryReduceSeconds(double& seconds, double unitSeconds, std::vector<std::string>& parts, std::string_view unit)
        {
            if (seconds < unitSeconds)
            {
                return false;
            }

            double value = std::floor(seconds / unitSeconds);
            AddPart(parts, value, unit);
            seconds -= value * unitSeconds;
            return true;
        }

        std::string JoinParts(const std::vector<std::string>& parts, int precisionParts)
        {
            int count = std::min(static_cast<int>(parts.size()), precisionParts);
            if (count == 0)
            {
                return "";
            }
            if (count == 1)
            {
                return parts[0];
            }

            std::string result;
            for (int i = 0; i < count; i++)
            {
                if (i > 0)
                {
                    result += i == count - 1 ? " and " : ", ";
                }
                result += parts[i];
            }
            return result;
        }
    }

    std::string FormatAgo(std::string_view createdAt)
    {
        auto scoreTimestamp = TryParseScoreTimestamp(createdAt);
        if (!scoreTimestamp.has_value())
        {
            return "unknown";
        }

        auto now = std::chrono::system_clock::now().time_since_epoch();
        double nowSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(now).count() / 1000.0;
        double seconds = std::max(0.0, nowSeconds - scoreTimestamp.value());
        std::vector<std::string> parts;

        constexpr double secondsInDay = 86400.0;
        TryReduceSeconds(seconds, 365.0 * secondsInDay, parts, "year");
        TryReduceSeconds(seconds, 30.0 * secondsInDay, parts, "month");
        TryReduceSeconds(seconds, 7.0 * secondsInDay, parts, "week");
        TryReduceSeconds(seconds, secondsInDay, parts, "day");
        TryReduceSeconds(seconds, 3600.0, parts, "hour");
        TryReduceSeconds(seconds, 60.0, parts, "minute");
        if (seconds >= 1.0)
        {
            AddPart(parts, std::floor(seconds), "second");
        }
        else if (seconds > 0)
        {
            AddPart(parts, std::round(seconds * 1000.0) / 1000.0, "second");
        }

        auto naturalTime = JoinParts(parts, 2);
        return naturalTime.empty() ? "ago" : naturalTime + " ago";
    }
}
