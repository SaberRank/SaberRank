#pragma once

#include "Features/Leaderboards/Domain/LeaderboardInfoMap.hpp"
#include "Features/Leaderboards/Domain/LeaderboardSnapshot.hpp"
#include "Features/Leaderboards/Domain/ScoreMap.hpp"
#include "Features/Replays/ReplayStorageService.hpp"

#include <optional>
#include <vector>

namespace SnoreSaber::Data
{
    struct LeaderboardMap
    {
        LeaderboardMap() = default;
        LeaderboardMap(const LeaderboardSnapshot& leaderboard,
                       GlobalNamespace::BeatmapLevel* beatmapLevel,
                       GlobalNamespace::BeatmapKey beatmapKey,
                       int maxScore,
                       SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService);

        LeaderboardInfoMap leaderboardInfo;
        std::vector<ScoreMap> scores;
        std::optional<Score> playerScore;
    };
}
