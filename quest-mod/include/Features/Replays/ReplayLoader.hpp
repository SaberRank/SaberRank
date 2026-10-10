#pragma once

#include "Features/Leaderboards/Domain/Score.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include "Features/Replays/Services/ReplayQueryService.hpp"

#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <GlobalNamespace/EnvironmentsListModel.hpp>
#include <GlobalNamespace/LevelCompletionResults.hpp>
#include <GlobalNamespace/MenuTransitionsHelper.hpp>
#include <GlobalNamespace/OverrideEnvironmentSettings.hpp>
#include <GlobalNamespace/PlayerData.hpp>
#include <GlobalNamespace/PlayerDataModel.hpp>
#include <GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp>
#include <System/zzzz__Object_def.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <functional>
#include <string>
#include <vector>

namespace SnoreSaber::ReplaySystem
{
    enum class ReplayLoadResult
    {
        Loaded,
        Missing,
        UnsupportedLegacy,
        InvalidData,
    };
}

DECLARE_CLASS_CODEGEN(SnoreSaber::ReplaySystem, ReplayLoader, System::Object) {
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::PlayerDataModel*, _playerDataModel);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::MenuTransitionsHelper*, _menuTransitionsHelper);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::EnvironmentsListModel*, _environmentsListModel);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::ReplaySystem::Services::ReplayQueryService*, _replayQueryService);

    DECLARE_CTOR(ctor,
                 GlobalNamespace::PlayerDataModel* playerDataModel,
                 GlobalNamespace::MenuTransitionsHelper* menuTransitionsHelper,
                 GlobalNamespace::EnvironmentsListModel* environmentsListModel,
                 SnoreSaber::ReplaySystem::Services::ReplayQueryService* replayQueryService);

    GlobalNamespace::OverrideEnvironmentSettings* ReplayEnvironmentSettings(GlobalNamespace::PlayerData* playerData,
                                                                            const SnoreSaber::Data::Private::Metadata& metadata,
                                                                            bool useRecordedPlayerSettings);

  public:
    void StartReplay(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey);
    void Load(const std::vector<char>& replayData,
              GlobalNamespace::BeatmapLevel* beatmapLevel,
              GlobalNamespace::BeatmapKey beatmapKey,
              std::string modifiers,
              std::u16string playerName);
    void GetReplayData(GlobalNamespace::BeatmapLevel* beatmapLevel,
                       GlobalNamespace::BeatmapKey beatmapKey,
                       int leaderboardId,
                       std::string replayFileName,
                       SnoreSaber::Data::Score& score,
                       const std::function<void(SnoreSaber::ReplaySystem::ReplayLoadResult)>& finished);
};
