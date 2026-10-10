#pragma once

#include <GlobalNamespace/AudioManagerSO.hpp>
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/BasicBeatmapObjectManager.hpp>
#include <GlobalNamespace/BeatmapCallbacksController.hpp>
#include <GlobalNamespace/NoteCutSoundEffectManager.hpp>
#include "Features/Replays/Playback/ComboPlayer.hpp"
#include "Features/Replays/Playback/EnergyPlayer.hpp"
#include "Features/Replays/Playback/HeightPlayer.hpp"
#include "Features/Replays/Playback/MultiplierPlayer.hpp"
#include "Features/Replays/Playback/NotePlayer.hpp"
#include "Features/Replays/Playback/PosePlayer.hpp"
#include "Features/Replays/Playback/ScorePlayer.hpp"
#include <System/IDisposable.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/IInitializable.hpp>
#include <Zenject/ITickable.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::ReplaySystem::Playback,
        ReplayTimeSyncController,
        System::Object,
        Zenject::ITickable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::AudioTimeSyncController>, _audioTimeSyncController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::AudioManagerSO>, _audioManagerSO);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::AudioTimeSyncController::InitData*, _audioInitData);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::BasicBeatmapObjectManager*, _basicBeatmapObjectManager);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::NoteCutSoundEffectManager>, _noteCutSoundEffectManager);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::BeatmapCallbacksController::InitData*, _callbackInitData);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::BeatmapCallbacksController*, _beatmapObjectCallbackController);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::ReplaySystem::Playback::ComboPlayer*, _comboPlayer);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::ReplaySystem::Playback::EnergyPlayer*, _energyPlayer);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::ReplaySystem::Playback::HeightPlayer*, _heightPlayer);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::ReplaySystem::Playback::MultiplierPlayer*, _multiplierPlayer);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::ReplaySystem::Playback::NotePlayer*, _notePlayer);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::ReplaySystem::Playback::PosePlayer*, _posePlayer);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::ReplaySystem::Playback::ScorePlayer*, _scorePlayer);
    DECLARE_INSTANCE_FIELD_PRIVATE(bool, _paused);

    DECLARE_CTOR(ctor,
        GlobalNamespace::AudioTimeSyncController* audioTimeSyncController,
        GlobalNamespace::AudioTimeSyncController::InitData* audioInitData,
        GlobalNamespace::BasicBeatmapObjectManager* basicBeatmapObjectManager,
        GlobalNamespace::NoteCutSoundEffectManager* noteCutSoundEffectManager,
        GlobalNamespace::BeatmapCallbacksController::InitData* callbackInitData,
        GlobalNamespace::BeatmapCallbacksController* beatmapObjectCallbackController,
        Zenject::DiContainer* container);

    DECLARE_OVERRIDE_METHOD_MATCH(void, Tick, &::Zenject::ITickable::Tick);
    DECLARE_INSTANCE_METHOD(void, UpdateTimes);
    DECLARE_INSTANCE_METHOD(void, OverrideTime, float time);
    DECLARE_INSTANCE_METHOD(void, OverrideTimeScale, float timeScale);
    DECLARE_INSTANCE_METHOD(void, CancelAllHitSounds);
};