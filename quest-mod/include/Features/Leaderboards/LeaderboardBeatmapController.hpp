#pragma once

#include "Features/Leaderboards/Services/LeaderboardScreenSession.hpp"

#include <GlobalNamespace/BeatmapKey.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Leaderboards, LeaderboardBeatmapController, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Leaderboards::Services::LeaderboardScreenSession*, _leaderboardSession);
    DECLARE_CTOR(ctor, SnoreSaber::Features::Leaderboards::Services::LeaderboardScreenSession* leaderboardSession);

  public:
    void OnLeaderboardSet(GlobalNamespace::BeatmapKey beatmapKey);
};
