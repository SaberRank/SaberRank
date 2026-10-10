#pragma once

#include <optional>
#include <string>

namespace SnoreSaber::Data
{
    enum class LeaderboardQueryScope
    {
        Global,
        AroundPlayer,
        Friends,
        Country,
        Region,
        Countries,
    };

    struct LeaderboardQuery
    {
        std::string songHash;
        std::string gameMode;
        int difficulty = 0;
        int page = 1;
        int limit = 10;
        LeaderboardQueryScope scope = LeaderboardQueryScope::Global;
        std::string countries;
        bool hideNoArrows = false;
        std::optional<int> realmId;
    };
}
