#include "Features/Replays/ReplayLoader.hpp"

#include "Core/Gameplay/SnoreSaberGameplayModifiers.hpp"
#include "Data/Private/Settings.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include "Features/Replays/ReplayFileCodec.hpp"
#include "Features/Replays/ReplayStateRegistry.hpp"
#include "Features/ScoreSubmission/Services/ScoreSubmissionService.hpp"
#include "Utils/AsyncUtils.hpp"
#include "Utils/GCUtil.hpp"
#include "Utils/SafePtr.hpp"
#include "logging.hpp"

#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/ColorScheme.hpp>
#include <GlobalNamespace/ColorSchemesSettings.hpp>
#include <GlobalNamespace/EnvironmentInfoSO.hpp>
#include <GlobalNamespace/EnvironmentsListModel.hpp>
#include <GlobalNamespace/GameplayModifiers.hpp>
#include <GlobalNamespace/OverrideEnvironmentSettings.hpp>
#include <GlobalNamespace/PlayerData.hpp>
#include <GlobalNamespace/PlayerSpecificSettings.hpp>
#include <GlobalNamespace/RecordingToolManager.hpp>
#include <System/Action_2.hpp>
#include <System/Nullable_1.hpp>
#include <bsml/shared/Helpers/getters.hpp>
#include <custom-types/shared/delegate.hpp>
#include <metacore/shared/game.hpp>

using namespace GlobalNamespace;
using namespace BSML;
using namespace BSML::Helpers;

DEFINE_TYPE(SnoreSaber::ReplaySystem, ReplayLoader);

namespace SnoreSaber::ReplaySystem
{
    namespace
    {
        std::shared_ptr<SnoreSaber::Data::Private::ReplayFile> CurrentReplay()
        {
            return ReplayStateRegistry::Current.loadedReplayFile;
        }

        void ReplayEndCallback(UnityW<StandardLevelScenesTransitionSetupDataSO> standardLevelSceneSetupData, LevelCompletionResults* levelCompletionResults)
        {
            (void)standardLevelSceneSetupData;
            (void)levelCompletionResults;

            ReplayStateRegistry::Current.EndPlayback();
            if (auto scoreSubmissionService = Helpers::GetDiContainer()->TryResolve<Features::ScoreSubmission::Services::ScoreSubmissionService*>())
            {
                scoreSubmissionService->ResumeAfterReplay();
            }
        }

        bool IsLegacyReplayData(const std::vector<char>& replayData)
        {
            return replayData.size() >= 4 &&
                   replayData[0] == static_cast<char>(93) &&
                   replayData[1] == 0 &&
                   replayData[2] == 0 &&
                   replayData[3] == static_cast<char>(128);
        }

        ArcVisibilityType ArcVisibilityOrDefault(int value, ArcVisibilityType arcVisibilityFallback)
        {
            return value >= (int)ArcVisibilityType::None && value <= (int)ArcVisibilityType::High
                       ? ArcVisibilityType(value)
                       : arcVisibilityFallback;
        }

        EnvironmentEffectsFilterPreset EnvironmentEffectsFilterPresetOrDefault(int value, EnvironmentEffectsFilterPreset presetFallback)
        {
            switch (value)
            {
                case (int)EnvironmentEffectsFilterPreset::AllEffects:
                case (int)EnvironmentEffectsFilterPreset::StrobeFilter:
                case (int)EnvironmentEffectsFilterPreset::NoEffects:
                    return EnvironmentEffectsFilterPreset(value);
                default:
                    return presetFallback;
            }
        }

        ColorScheme* ReplayColorScheme(PlayerData* playerData, const SnoreSaber::Data::Private::Metadata& metadata)
        {
            if (!metadata.LeftSaberColor.has_value() || !metadata.RightSaberColor.has_value())
            {
                return nullptr;
            }

            auto colorSchemesSettings = playerData->colorSchemesSettings;
            auto baseScheme = colorSchemesSettings->GetOverrideColorScheme();
            if (!baseScheme)
            {
                baseScheme = colorSchemesSettings->GetSelectedColorScheme();
            }
            if (!baseScheme)
            {
                return nullptr;
            }

            bool hasLightColors = metadata.ObstacleColor.has_value() &&
                                  metadata.EnvironmentColor0.has_value() &&
                                  metadata.EnvironmentColor1.has_value() &&
                                  metadata.EnvironmentColor0Boost.has_value() &&
                                  metadata.EnvironmentColor1Boost.has_value();
            auto obstacleColor = hasLightColors ? metadata.ObstacleColor.value() : baseScheme->obstaclesColor;
            auto environmentColor0 = hasLightColors ? metadata.EnvironmentColor0.value() : baseScheme->environmentColor0;
            auto environmentColor1 = hasLightColors ? metadata.EnvironmentColor1.value() : baseScheme->environmentColor1;
            auto environmentColor0Boost = hasLightColors ? metadata.EnvironmentColor0Boost.value() : baseScheme->environmentColor0Boost;
            auto environmentColor1Boost = hasLightColors ? metadata.EnvironmentColor1Boost.value() : baseScheme->environmentColor1Boost;
            bool supportsEnvironmentColorBoost = hasLightColors ? metadata.SupportsEnvironmentColorBoost : baseScheme->_supportsEnvironmentColorBoost;
            auto environmentColorW = hasLightColors && metadata.EnvironmentColorW.has_value() ? metadata.EnvironmentColorW.value() : baseScheme->environmentColorW;
            auto environmentColorWBoost = hasLightColors && metadata.EnvironmentColorWBoost.has_value() ? metadata.EnvironmentColorWBoost.value() : baseScheme->environmentColorWBoost;
            return ColorScheme::New_ctor(baseScheme, true, metadata.LeftSaberColor.value(), metadata.RightSaberColor.value(),
                                         hasLightColors, environmentColor0, environmentColor1, environmentColorW,
                                         supportsEnvironmentColorBoost, environmentColor0Boost, environmentColor1Boost,
                                         environmentColorWBoost, obstacleColor);
        }
    }

    void ReplayLoader::ctor(PlayerDataModel* playerDataModel,
                            MenuTransitionsHelper* menuTransitionsHelper,
                            EnvironmentsListModel* environmentsListModel,
                            Services::ReplayQueryService* replayQueryService)
    {
        INVOKE_CTOR();
        _playerDataModel = playerDataModel;
        _menuTransitionsHelper = menuTransitionsHelper;
        _environmentsListModel = environmentsListModel;
        _replayQueryService = replayQueryService;
    }

    OverrideEnvironmentSettings* ReplayLoader::ReplayEnvironmentSettings(PlayerData* playerData, const SnoreSaber::Data::Private::Metadata& metadata, bool useRecordedPlayerSettings)
    {
        if (!useRecordedPlayerSettings || metadata.Environment.empty())
        {
            return playerData->overrideEnvironmentSettings;
        }

        auto environmentInfo = _environmentsListModel->GetEnvironmentInfoBySerializedName(StringW(metadata.Environment));
        if (!environmentInfo)
        {
            return playerData->overrideEnvironmentSettings;
        }

        auto overrideEnvironmentSettings = OverrideEnvironmentSettings::New_ctor();
        overrideEnvironmentSettings->overrideEnvironments = true;
        overrideEnvironmentSettings->SetEnvironmentInfoForType(environmentInfo->environmentType, environmentInfo.ptr());
        return overrideEnvironmentSettings;
    }

    void ReplayLoader::StartReplay(BeatmapLevel* beatmapLevel, BeatmapKey beatmapKey)
    {
        auto loadedReplay = CurrentReplay();
        if (!loadedReplay)
        {
            ERROR("Cannot start replay because no replay is loaded");
            return;
        }

        auto playerData = _playerDataModel->playerData;
        auto localPlayerSettings = playerData->playerSpecificSettings;
        if (SnoreSaber::Data::Private::Settings::replayOverrideHandedness && loadedReplay->metadata->LeftHanded != localPlayerSettings->leftHanded)
        {
            loadedReplay->Mirror();
        }

        auto metadata = loadedReplay->metadata;
        bool useRecordedPlayerSettings = SnoreSaber::Data::Private::Settings::useRecordedPlayerSettings && metadata->HasPlaySettingsExtension;
        bool replayNoTextsAndHuds = useRecordedPlayerSettings ? metadata->NoTextsAndHuds : (bool)localPlayerSettings->noTextsAndHuds;
        float replaySaberTrailIntensity = useRecordedPlayerSettings ? metadata->SaberTrailIntensity : (float)localPlayerSettings->saberTrailIntensity;
        bool replayHideNoteSpawnEffect = useRecordedPlayerSettings ? metadata->HideNoteSpawnEffect : (bool)localPlayerSettings->hideNoteSpawnEffect;
        bool replayArcsHapticFeedback = useRecordedPlayerSettings ? metadata->ArcsHapticFeedback : (bool)localPlayerSettings->arcsHapticFeedback;
        ArcVisibilityType replayArcVisibility = useRecordedPlayerSettings
                                                    ? ArcVisibilityOrDefault(metadata->ArcVisibility, localPlayerSettings->arcVisibility)
                                                    : (ArcVisibilityType)localPlayerSettings->arcVisibility;
        EnvironmentEffectsFilterPreset replayEnvironmentEffectsFilterDefaultPreset = useRecordedPlayerSettings
                                                                                         ? EnvironmentEffectsFilterPresetOrDefault(metadata->EnvironmentEffectsFilterDefaultPreset, localPlayerSettings->environmentEffectsFilterDefaultPreset)
                                                                                         : (EnvironmentEffectsFilterPreset)localPlayerSettings->environmentEffectsFilterDefaultPreset;
        EnvironmentEffectsFilterPreset replayEnvironmentEffectsFilterExpertPlusPreset = useRecordedPlayerSettings
                                                                                            ? EnvironmentEffectsFilterPresetOrDefault(metadata->EnvironmentEffectsFilterExpertPlusPreset, localPlayerSettings->environmentEffectsFilterExpertPlusPreset)
                                                                                            : (EnvironmentEffectsFilterPreset)localPlayerSettings->environmentEffectsFilterExpertPlusPreset;

        auto playerSettings = PlayerSpecificSettings::New_ctor(loadedReplay->metadata->LeftHanded, loadedReplay->metadata->InitialHeight,
                                                               loadedReplay->heightKeyframes.size() > 0, localPlayerSettings->sfxVolume,
                                                               localPlayerSettings->reduceDebris, replayNoTextsAndHuds, localPlayerSettings->noFailEffects,
                                                               localPlayerSettings->advancedHud, localPlayerSettings->autoRestart, replaySaberTrailIntensity,
                                                               localPlayerSettings->noteJumpDurationTypeSettings, localPlayerSettings->noteJumpFixedDuration,
                                                               localPlayerSettings->noteJumpStartBeatOffset, replayHideNoteSpawnEffect, localPlayerSettings->adaptiveSfx,
                                                               replayArcsHapticFeedback, replayArcVisibility,
                                                               replayEnvironmentEffectsFilterDefaultPreset, replayEnvironmentEffectsFilterExpertPlusPreset,
                                                               localPlayerSettings->headsetHapticIntensity);
        ColorScheme* replayColorScheme = useRecordedPlayerSettings ? ReplayColorScheme(playerData, *metadata) : nullptr;
        OverrideEnvironmentSettings* replayEnvironmentSettings = ReplayEnvironmentSettings(playerData, *metadata, useRecordedPlayerSettings);

        auto replayEndDelegate = custom_types::MakeDelegate<System::Action_2<UnityW<StandardLevelScenesTransitionSetupDataSO>, LevelCompletionResults*>*>(
            classof(System::Action_2<UnityW<StandardLevelScenesTransitionSetupDataSO>, LevelCompletionResults*>*),
            std::function<void(UnityW<StandardLevelScenesTransitionSetupDataSO>, LevelCompletionResults*)>(ReplayEndCallback));

        if (auto scoreSubmissionService = Helpers::GetDiContainer()->TryResolve<Features::ScoreSubmission::Services::ScoreSubmissionService*>())
        {
            scoreSubmissionService->SuspendForReplay();
        }
        MetaCore::Game::DisableScoreSubmissionOnce(MOD_ID);
        MetaCore::Game::DisableScoreSubmissionOnce("Replay"); // keep BL from submitting during replay

        System::Nullable_1<RecordingToolManager_SetupData> recordingToolData;
        _menuTransitionsHelper->StartStandardLevel("Replay",
                                                   byref(beatmapKey),
                                                   beatmapLevel,
                                                   replayEnvironmentSettings,
                                                   replayColorScheme ? replayColorScheme : playerData->colorSchemesSettings->GetOverrideColorScheme(),
                                                   replayColorScheme ? (bool)replayColorScheme->overrideLights : (bool)playerData->colorSchemesSettings->ShouldOverrideLightshowColors(),
                                                   beatmapLevel->GetColorScheme(beatmapKey.beatmapCharacteristic, beatmapKey.difficulty),
                                                   SnoreSaber::Core::Gameplay::SnoreSaberGameplayModifiers::FromCodes(loadedReplay->metadata->Modifiers, true).gameplayModifiers,
                                                   playerSettings,
                                                   nullptr,
                                                   _environmentsListModel,
                                                   "Exit Replay",
                                                   false,
                                                   false,
                                                   nullptr,
                                                   nullptr,
                                                   replayEndDelegate,
                                                   nullptr,
                                                   recordingToolData);
    }

    void ReplayLoader::Load(const std::vector<char>& replayData, BeatmapLevel* beatmapLevel, BeatmapKey beatmapKey, std::string modifiers, std::u16string playerName)
    {
        ReplayStateRegistry::Current.BeginReplay(beatmapLevel, beatmapKey, std::move(modifiers), std::move(playerName));

        auto loadedReplay = ReplayFileCodec::Read(replayData);
        if (!loadedReplay)
        {
            ERROR("Replay could not be read");
            return;
        }

        ReplayStateRegistry::Current.LoadReplay(std::move(loadedReplay));

        SafePtr<ReplayLoader> self(this);
        SafePtr<BeatmapLevel> beatmapLevelSafe(beatmapLevel);
        SnoreSaber::Utils::Async::Main([self, beatmapLevelSafe, beatmapKey]() {
            self->StartReplay(beatmapLevelSafe.ptr(), beatmapKey);
        });
    }

    void ReplayLoader::GetReplayData(BeatmapLevel* beatmapLevel,
                                     BeatmapKey beatmapKey,
                                     int leaderboardId,
                                     std::string replayFileName,
                                     SnoreSaber::Data::Score& score,
                                     const std::function<void(ReplayLoadResult)>& finished)
    {
        (void)leaderboardId;
        ReplayStateRegistry::Current.BeginReplay(beatmapLevel, beatmapKey, score.modifiers, score.leaderboardPlayerInfo.name.value_or(u"unknown"));

        SafePtr<ReplayLoader> self(this);
        SafePtr<BeatmapLevel> beatmapLevelSafe(beatmapLevel);
        SnoreSaber::Utils::Async::Run([self, beatmapLevelSafe, beatmapKey, replayFileName = std::move(replayFileName), score, finished]() {
            INFO("Starting replay download");
            auto replayData = self->_replayQueryService->GetReplayData(beatmapLevelSafe.ptr(), beatmapKey, score, replayFileName);
            if (!replayData.has_value())
            {
                finished(ReplayLoadResult::Missing);
                return;
            }

            if (IsLegacyReplayData(replayData.value()))
            {
                ERROR("Legacy replays use the old BinaryFormatter Z.SavedData format and cannot be loaded on Quest");
                finished(ReplayLoadResult::UnsupportedLegacy);
                return;
            }

            auto loadedReplay = ReplayFileCodec::Read(replayData.value());
            if (!loadedReplay)
            {
                ERROR("Replay data could not be read");
                finished(ReplayLoadResult::InvalidData);
                return;
            }

            ReplayStateRegistry::Current.LoadReplay(std::move(loadedReplay));
            finished(ReplayLoadResult::Loaded);
        });
    }
}
