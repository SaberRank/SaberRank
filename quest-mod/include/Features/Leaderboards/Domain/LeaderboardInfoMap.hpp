#pragma once

#include "Features/Leaderboards/Domain/LeaderboardDetails.hpp"

#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>

#include <string>

namespace SnoreSaber::Data
{
    struct LeaderboardInfoMap
    {
        LeaderboardInfoMap() = default;
        LeaderboardInfoMap(LeaderboardDetails leaderboard, GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey);

        LeaderboardDetails leaderboard;
        GlobalNamespace::BeatmapLevel* beatmapLevel = nullptr;
        GlobalNamespace::BeatmapKey beatmapKey;
        std::string songHash;
    };
}
