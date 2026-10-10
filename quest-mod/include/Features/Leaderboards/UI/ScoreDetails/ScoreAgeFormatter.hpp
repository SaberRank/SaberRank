#pragma once

#include <string>
#include <string_view>

namespace SnoreSaber::UI::Other::ScoreAgeFormatter
{
    std::string FormatAgo(std::string_view createdAt);
}
