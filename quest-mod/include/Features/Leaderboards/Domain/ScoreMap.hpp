#pragma once

#include "Features/Leaderboards/Domain/LeaderboardInfoMap.hpp"
#include "Features/Leaderboards/Domain/Score.hpp"
#include "Features/Replays/ReplayStorageService.hpp"

#include <string>

namespace SnoreSaber::Data
{
    struct ScoreMap
    {
        ScoreMap() = default;
        ScoreMap(Score score,
                 LeaderboardInfoMap parent,
                 int maxScore,
                 SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService);

        Score score;
        LeaderboardInfoMap parent;
        std::string replayFileName;
        bool hasLocalReplay = false;
        double accuracy = 0.0;
        std::string modifierText;

        bool HasReplay() const;
    };
}
