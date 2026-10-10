#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include "Utils/DelegateUtils.hpp"
#include <BeatSaber/GameSettings/ControllerProfilesModel.hpp>
#include <GlobalNamespace/SettingsManager.hpp>
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/BeatmapObjectSpawnController.hpp>
#include <GlobalNamespace/GameplayCoreSceneSetupData.hpp>
#include <GlobalNamespace/GameEnergyCounter.hpp>
#include <GlobalNamespace/VariableMovementDataProvider.hpp>
#include <System/IDisposable.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/IInitializable.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include "Utils/DelegateUtils.hpp"

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::ReplaySystem::Recorders,
        MetadataRecorder,
        System::Object,
        System::IDisposable*,
        Zenject::IInitializable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::AudioTimeSyncController>, _audioTimeSyncController);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::BeatmapObjectSpawnController::InitData*, _beatmapObjectSpawnControllerInitData);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::GameplayCoreSceneSetupData*, _gameplayCoreSceneSetupData);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::SettingsManager*, _settingsManager);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::GameEnergyCounter>, _gameEnergyCounter);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::AudioTimeSyncController::InitData*, _audioTimeSyncInitData);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::VariableMovementDataProvider*, _movementDataProvider);
    DECLARE_INSTANCE_FIELD_PRIVATE(BeatSaber::GameSettings::ControllerProfilesModel*, _controllerProfilesModel);
    DECLARE_CTOR(ctor, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::GameplayCoreSceneSetupData* gameplayCoreSceneSetupData, GlobalNamespace::BeatmapObjectSpawnController::InitData* beatmapObjectSpawnControllerInitData, GlobalNamespace::GameEnergyCounter* gameEnergyCounter, GlobalNamespace::SettingsManager* settingsManager, Zenject::DiContainer* container);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);
    DECLARE_INSTANCE_METHOD(void, GameEnergyCounter_gameEnergyDidReach0Event);
    float _failTime;
    DelegateUtils::DelegateW<System::Action> gameEnergyDidReach0Delegate;
    float SongSpeed();
    float JumpDistance();
    int CurrentEnvironmentEffectsFilterPreset();
    std::optional<Data::Private::ReplayControllerOffsets> ControllerOffsets();
public:
    std::shared_ptr<Data::Private::Metadata> Export();
};