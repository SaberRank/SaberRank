#pragma once
#include <GlobalNamespace/PlatformLeaderboardsModel.hpp>
#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include "Features/Leaderboards/Domain/InternalLeaderboard.hpp"
#include "Features/Leaderboards/Domain/LeaderboardSnapshot.hpp"
#include "Features/Replays/ReplayStorageService.hpp"

namespace SnoreSaber::Services::LeaderboardService
{
    SnoreSaber::Data::InternalLeaderboard GetLeaderboardError(std::string error, bool leaderboardNotFound = false);
    SnoreSaber::Data::InternalLeaderboard ParseLeaderboardData(SnoreSaber::Data::LeaderboardSnapshot currentLeaderboard, GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, PlatformLeaderboardsModel::ScoresScope scope,
                                                   int page, bool filterAroundCountry, int maxScore, SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService = nullptr);
} // namespace SnoreSaber::Services::LeaderboardService
