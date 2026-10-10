#pragma once

#include "Features/Leaderboards/Services/LeaderboardScreenLoader.hpp"

#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/PlatformLeaderboardsModel.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Leaderboards::Services, LeaderboardScreenSession, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Leaderboards::Services::LeaderboardScreenLoader*, _leaderboardLoader);
    DECLARE_INSTANCE_FIELD_PRIVATE(int, _page);
    DECLARE_INSTANCE_FIELD_PRIVATE(bool, _filterAroundCountry);
    DECLARE_INSTANCE_FIELD_PRIVATE(int, _currentRefreshId);
    DECLARE_CTOR(ctor, SnoreSaber::Features::Leaderboards::Services::LeaderboardScreenLoader* leaderboardLoader);

  public:
    void Reset();
    void SetBeatmap(GlobalNamespace::BeatmapKey beatmapKey);
    void ClearBeatmap();
    void SelectScope(GlobalNamespace::PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry);
    void RefreshFromFirstPage();
    void PageUp();
    void PageDown();
    int Page();
    bool FilterAroundCountry();
    int BeginRefresh(GlobalNamespace::PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry);
    bool IsCurrentRefresh(int refreshId);
    bool CanPageScope(GlobalNamespace::PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry) const;
};
