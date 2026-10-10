#pragma once

#include "Features/Leaderboards/Domain/Score.hpp"
#include "Features/Replays/ReplayStorageService.hpp"
#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <System/zzzz__Object_def.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <optional>
#include <string>
#include <vector>

DECLARE_CLASS_CODEGEN(SnoreSaber::ReplaySystem::Services, ReplayQueryService, System::Object) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::ReplaySystem::ReplayStorageService*, _replayStorageService);
    DECLARE_CTOR(ctor, SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService);

  public:
    std::optional<std::vector<char>> GetReplayData(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, const SnoreSaber::Data::Score& score, const std::string& legacyReplayFileName = "");
};
