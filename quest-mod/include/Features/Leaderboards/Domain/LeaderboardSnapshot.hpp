#pragma once

#include "Core/Api/Paging/PagedResult.hpp"
#include "Features/Leaderboards/Domain/LeaderboardDetails.hpp"
#include "Features/Leaderboards/Domain/Score.hpp"

#include <optional>

namespace SnoreSaber::Data
{
    struct LeaderboardSnapshot
    {
        LeaderboardDetails leaderboard;
        Core::Api::Paging::PagedResult<Score> scores;
        std::optional<Score> playerScore;
    };
}
