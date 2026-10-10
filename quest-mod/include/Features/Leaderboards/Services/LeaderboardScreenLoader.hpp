#pragma once

#include "Features/Leaderboards/Domain/LeaderboardScreenState.hpp"
#include "Features/Leaderboards/Services/LeaderboardQueryService.hpp"
#include "Features/Leaderboards/Services/MaxScoreCache.hpp"
#include "Features/Players/Services/GameSessionService.hpp"

#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <GlobalNamespace/PlatformLeaderboardsModel.hpp>

#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <functional>

namespace SnoreSaber::Features::Leaderboards::Services
{
    struct LoadResult
    {
        SnoreSaber::Data::LeaderboardScreenState state;
        int maxScore = 0;
    };
}

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Leaderboards::Services, LeaderboardScreenLoader, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Utils::MaxScoreCache*, _maxScoreCache);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Leaderboards::Services::LeaderboardQueryService*, _leaderboardQueryService);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Players::Services::GameSessionService*, _gameSessionService);
    DECLARE_CTOR(ctor,
                 SnoreSaber::Utils::MaxScoreCache* maxScoreCache,
                 SnoreSaber::Features::Leaderboards::Services::LeaderboardQueryService* leaderboardQueryService,
                 SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService);

  public:
    bool CanPageScope(GlobalNamespace::PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry) const;
    void Load(GlobalNamespace::BeatmapLevel* beatmapLevel,
              GlobalNamespace::BeatmapKey beatmapKey,
              GlobalNamespace::PlatformLeaderboardsModel::ScoresScope scope,
              int page,
              bool filterAroundCountry,
              std::function<void(LoadResult)> finished);
};
