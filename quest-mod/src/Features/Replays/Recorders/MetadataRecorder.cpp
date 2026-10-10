#include "Features/Replays/Recorders/MetadataRecorder.hpp"
#include "Core/Gameplay/SnoreSaberGameplayModifiers.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include <BeatSaber/GameSettings/Controller.hpp>
#include <BeatSaber/GameSettings/ControllerProfile.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/BeatmapDifficulty.hpp>
#include <GlobalNamespace/BeatmapDifficultyMethods.hpp>
#include <GlobalNamespace/BeatmapObjectSpawnMovementData.hpp>
#include <GlobalNamespace/ColorScheme.hpp>
#include <GlobalNamespace/EnvironmentInfoSO.hpp>
#include <GlobalNamespace/GameEnergyCounter.hpp>
#include <GlobalNamespace/GameplayCoreSceneSetupData.hpp>
#include <GlobalNamespace/PlayerSpecificSettings.hpp>
#include <GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp>
#include <System/Action.hpp>
#include <UnityEngine/Application.hpp>
#include <custom-types/shared/delegate.hpp>
#include <functional>
#include "Utils/Versions.hpp"
#include "logging.hpp"

using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Recorders, MetadataRecorder);

namespace SnoreSaber::ReplaySystem::Recorders
{
    void MetadataRecorder::ctor(AudioTimeSyncController* audioTimeSyncController, GameplayCoreSceneSetupData* gameplayCoreSceneSetupData, BeatmapObjectSpawnController::InitData* beatmapObjectSpawnControllerInitData, GameEnergyCounter* gameEnergyCounter, GlobalNamespace::SettingsManager* settingsManager, Zenject::DiContainer* container)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        _beatmapObjectSpawnControllerInitData = beatmapObjectSpawnControllerInitData;
        _gameEnergyCounter = gameEnergyCounter;
        _gameplayCoreSceneSetupData = gameplayCoreSceneSetupData;
        _settingsManager = settingsManager;
        _audioTimeSyncInitData = container->TryResolve<AudioTimeSyncController::InitData*>();
        _movementDataProvider = container->TryResolve<VariableMovementDataProvider*>();
        _controllerProfilesModel = container->TryResolve<BeatSaber::GameSettings::ControllerProfilesModel*>();
    }

    void MetadataRecorder::Initialize()
    {
        gameEnergyDidReach0Delegate = { &MetadataRecorder::GameEnergyCounter_gameEnergyDidReach0Event, this };
        _gameEnergyCounter->___gameEnergyDidReach0Event += gameEnergyDidReach0Delegate;
    }

    void MetadataRecorder::Dispose()
    {
        if(_gameEnergyCounter)
            _gameEnergyCounter->___gameEnergyDidReach0Event -= gameEnergyDidReach0Delegate;
    }

    void MetadataRecorder::GameEnergyCounter_gameEnergyDidReach0Event()
    {
        _failTime = _audioTimeSyncController->songTime;
    }
    
    std::shared_ptr<Metadata> MetadataRecorder::Export()
    {
        StringW csc;
        auto metadata = make_shared<Metadata>();
        metadata->Version = version("3.1.0");
        metadata->LevelID = (string)_gameplayCoreSceneSetupData->beatmapLevel->levelID;
        metadata->Difficulty = BeatmapDifficultyMethods::DefaultRating(_gameplayCoreSceneSetupData->beatmapKey.difficulty);
        metadata->Characteristic = (string)_gameplayCoreSceneSetupData->beatmapKey.beatmapCharacteristic->serializedName;
        metadata->Environment = (string)_gameplayCoreSceneSetupData->targetEnvironmentInfo->serializedName;
        metadata->Modifiers = SnoreSaber::Core::Gameplay::SnoreSaberGameplayModifiers::ToCodeList(_gameplayCoreSceneSetupData->gameplayModifiers, -1);
        metadata->NoteSpawnOffset = _beatmapObjectSpawnControllerInitData->noteJumpValue;
        metadata->LeftHanded = _gameplayCoreSceneSetupData->playerSpecificSettings->leftHanded;
        metadata->InitialHeight = _gameplayCoreSceneSetupData->playerSpecificSettings->playerHeight;
        metadata->RoomRotation = _settingsManager->settings.room.rotation;
        metadata->RoomCenter = VRPosition(_settingsManager->settings.room.center.x, _settingsManager->settings.room.center.y, _settingsManager->settings.room.center.z);
        metadata->FailTime = _failTime;
        metadata->GameVersion = version((string)Application::get_version());
        metadata->PluginVersion = version(VERSION);
        metadata->Platform = "Quest";
        metadata->HasPlaySettingsExtension = true;
        metadata->SongSpeed = SongSpeed();
        metadata->JumpDistance = JumpDistance();
        if (auto colorScheme = _gameplayCoreSceneSetupData->colorScheme) {
            metadata->LeftSaberColor = colorScheme->saberAColor;
            metadata->RightSaberColor = colorScheme->saberBColor;
            metadata->ObstacleColor = colorScheme->obstaclesColor;
            metadata->EnvironmentColor0 = colorScheme->environmentColor0;
            metadata->EnvironmentColor1 = colorScheme->environmentColor1;
            metadata->EnvironmentColorW = colorScheme->environmentColorW;
            metadata->EnvironmentColor0Boost = colorScheme->environmentColor0Boost;
            metadata->EnvironmentColor1Boost = colorScheme->environmentColor1Boost;
            metadata->EnvironmentColorWBoost = colorScheme->environmentColorWBoost;
            metadata->SupportsEnvironmentColorBoost = colorScheme->_supportsEnvironmentColorBoost;
        }
        auto playerSpecificSettings = _gameplayCoreSceneSetupData->playerSpecificSettings;
        metadata->EnvironmentEffectsFilterDefaultPreset = (int)playerSpecificSettings->environmentEffectsFilterDefaultPreset;
        metadata->EnvironmentEffectsFilterExpertPlusPreset = (int)playerSpecificSettings->environmentEffectsFilterExpertPlusPreset;
        metadata->EnvironmentEffectsFilterPreset = CurrentEnvironmentEffectsFilterPreset();
        metadata->NoTextsAndHuds = playerSpecificSettings->noTextsAndHuds;
        metadata->SaberTrailIntensity = playerSpecificSettings->saberTrailIntensity;
        metadata->HideNoteSpawnEffect = playerSpecificSettings->hideNoteSpawnEffect;
        metadata->ArcsHapticFeedback = playerSpecificSettings->arcsHapticFeedback;
        metadata->ArcVisibility = (int)playerSpecificSettings->arcVisibility;
        metadata->ControllerOffsets = ControllerOffsets();
        return metadata;
    }

    float MetadataRecorder::SongSpeed()
    {
        if (_audioTimeSyncInitData && _audioTimeSyncInitData->timeScale > 0.0f) {
            return _audioTimeSyncInitData->timeScale;
        }

        return _audioTimeSyncController ? _audioTimeSyncController->timeScale : 1.0f;
    }

    float MetadataRecorder::JumpDistance()
    {
        if (_movementDataProvider && _movementDataProvider->jumpDistance > 0.0f) {
            return _movementDataProvider->jumpDistance;
        }

        if (_beatmapObjectSpawnControllerInitData->noteJumpValueType == BeatmapObjectSpawnMovementData::NoteJumpValueType::JumpDuration) {
            return _beatmapObjectSpawnControllerInitData->noteJumpMovementSpeed * _beatmapObjectSpawnControllerInitData->noteJumpValue * 2.0f;
        }

        if (_beatmapObjectSpawnControllerInitData->beatsPerMinute <= 0.0f) {
            return 0.0f;
        }

        float halfJumpDuration = 4.0f;
        float beatDuration = 60.0f / _beatmapObjectSpawnControllerInitData->beatsPerMinute;
        while (_beatmapObjectSpawnControllerInitData->noteJumpMovementSpeed * beatDuration * halfJumpDuration > 17.999f) {
            halfJumpDuration /= 2.0f;
        }

        halfJumpDuration += _beatmapObjectSpawnControllerInitData->noteJumpValue;
        if (halfJumpDuration < 0.25f) {
            halfJumpDuration = 0.25f;
        }

        return _beatmapObjectSpawnControllerInitData->noteJumpMovementSpeed * beatDuration * halfJumpDuration * 2.0f;
    }

    int MetadataRecorder::CurrentEnvironmentEffectsFilterPreset()
    {
        if (_gameplayCoreSceneSetupData->beatmapKey.difficulty == BeatmapDifficulty::ExpertPlus) {
            return (int)_gameplayCoreSceneSetupData->playerSpecificSettings->environmentEffectsFilterExpertPlusPreset;
        }

        return (int)_gameplayCoreSceneSetupData->playerSpecificSettings->environmentEffectsFilterDefaultPreset;
    }

    namespace
    {
        ReplayControllerOffset ControllerOffset(UnityEngine::Vector3 position, UnityEngine::Vector3 rotation)
        {
            return ReplayControllerOffset{VRPosition(position.x, position.y, position.z), VRPosition(rotation.x, rotation.y, rotation.z)};
        }
    }

    std::optional<ReplayControllerOffsets> MetadataRecorder::ControllerOffsets()
    {
        if (!_controllerProfilesModel) {
            return std::nullopt;
        }

        auto profile = _controllerProfilesModel->selectedProfile;
        ReplayControllerOffsets offsets;
        offsets.Left = ControllerOffset(profile->leftController.position, profile->leftController.rotation);
        offsets.Right = ControllerOffset(profile->rightController.position, profile->rightController.rotation);
        return offsets;
    }
} // namespace SnoreSaber::ReplaySystem::Recorders
