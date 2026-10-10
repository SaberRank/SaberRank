#pragma once

#include "Features/Leaderboards/Domain/Score.hpp"
#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <System/zzzz__Object_def.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <optional>
#include <string>
#include <vector>

DECLARE_CLASS_CODEGEN(SnoreSaber::ReplaySystem, ReplayStorageService, System::Object) {
    DECLARE_CTOR(ctor);

  public:
    bool LocalReplayExists(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, const SnoreSaber::Data::Score& score, const std::string& legacyReplayFileName = "");
    std::optional<std::vector<char>> ReadLocalReplay(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, const SnoreSaber::Data::Score& score, const std::string& legacyReplayFileName = "");
    void SaveLocalReplay(const std::string& playerId, GlobalNamespace::BeatmapKey beatmapKey, const std::vector<char>& replay);
    std::string GetReplayPath(const std::string& playerId, const std::string& songHash, GlobalNamespace::BeatmapKey beatmapKey);
    std::string GetExistingReplayPath(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, const SnoreSaber::Data::Score& score, const std::string& legacyReplayFileName = "");
};
