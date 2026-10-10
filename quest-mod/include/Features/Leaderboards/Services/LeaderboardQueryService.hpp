#pragma once

#include "Features/Leaderboards/Domain/InternalLeaderboard.hpp"
#include "Features/Leaderboards/Services/LeaderboardPlayerScoreCache.hpp"
#include "Features/Players/Services/GameSessionService.hpp"
#include "Features/Replays/ReplayStorageService.hpp"

#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <GlobalNamespace/PlatformLeaderboardsModel.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <functional>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Leaderboards::Services, LeaderboardQueryService, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Leaderboards::Services::LeaderboardPlayerScoreCache*, _playerScoreCache);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::ReplaySystem::ReplayStorageService*, _replayStorageService);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Players::Services::GameSessionService*, _gameSessionService);
    DECLARE_CTOR(ctor,
                 SnoreSaber::Features::Leaderboards::Services::LeaderboardPlayerScoreCache* playerScoreCache,
                 SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService,
                 SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService);

  public:
    void GetLeaderboardData(int maxMultipliedScore,
                            GlobalNamespace::BeatmapLevel* beatmapLevel,
                            GlobalNamespace::BeatmapKey beatmapKey,
                            GlobalNamespace::PlatformLeaderboardsModel::ScoresScope scope,
                            int page,
                            std::function<void(SnoreSaber::Data::InternalLeaderboard)> finished,
                            bool filterAroundCountry);
};
