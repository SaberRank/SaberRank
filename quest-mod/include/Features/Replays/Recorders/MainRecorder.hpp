#pragma once

#include "Features/Live/Replay/LiveReplayStreamingService.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include "Features/Replays/Recorders/PoseRecorder.hpp"
#include "Features/Replays/Recorders/MetadataRecorder.hpp"
#include "Features/Replays/Recorders/NoteEventRecorder.hpp"
#include "Features/Replays/Recorders/ScoreEventRecorder.hpp"
#include "Features/Replays/Recorders/HeightEventRecorder.hpp"
#include "Features/Replays/Recorders/EnergyEventRecorder.hpp"
#include "Features/Replays/Recorders/PauseEventRecorder.hpp"
#include "Features/Replays/Recorders/WallEventRecorder.hpp"
#include "Features/Replays/Recorders/HsvConfigRecorder.hpp"
#include "Utils/DelegateUtils.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/GamePause.hpp>
#include <GlobalNamespace/IGamePause.hpp>
#include <System/IDisposable.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/IInitializable.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::ReplaySystem::Recorders,
        MainRecorder,
        System::Object,
        System::IDisposable*,
        Zenject::IInitializable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(PoseRecorder*, _poseRecorder);
    DECLARE_INSTANCE_FIELD_PRIVATE(MetadataRecorder*, _metadataRecorder);
    DECLARE_INSTANCE_FIELD_PRIVATE(NoteEventRecorder*, _noteEventRecorder);
    DECLARE_INSTANCE_FIELD_PRIVATE(ScoreEventRecorder*, _scoreEventRecorder);
    DECLARE_INSTANCE_FIELD_PRIVATE(HeightEventRecorder*, _heightEventRecorder);
    DECLARE_INSTANCE_FIELD_PRIVATE(EnergyEventRecorder*, _energyEventRecorder);
    DECLARE_INSTANCE_FIELD_PRIVATE(PauseEventRecorder*, _pauseEventRecorder);
    DECLARE_INSTANCE_FIELD_PRIVATE(WallEventRecorder*, _wallEventRecorder);
    DECLARE_INSTANCE_FIELD_PRIVATE(HsvConfigRecorder*, _hsvConfigRecorder);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Live::Replay::LiveReplayStreamingService*, _liveReplayStreamingService);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::GamePause*, _gamePause);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::AudioTimeSyncController>, _audioTimeSyncController);
    DECLARE_CTOR(ctor, PoseRecorder* poseRecorder, MetadataRecorder* metadataRecorder, NoteEventRecorder* noteEventRecorder, ScoreEventRecorder* scoreEventRecorder, HeightEventRecorder* heightEventRecorder, EnergyEventRecorder* energyEventRecorder, PauseEventRecorder* pauseEventRecorder, WallEventRecorder* wallEventRecorder, HsvConfigRecorder* hsvConfigRecorder, SnoreSaber::Features::Live::Replay::LiveReplayStreamingService* liveReplayStreamingService, Zenject::DiContainer* container);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);
    DECLARE_INSTANCE_METHOD(void, GamePause_didPauseEvent);
    DECLARE_INSTANCE_METHOD(void, GamePause_didResumeEvent);
    std::vector<char> _hsvConfig;
    DelegateUtils::DelegateW<System::Action> didPauseDelegate;
    DelegateUtils::DelegateW<System::Action> didResumeDelegate;
    float CurrentSongTime();
public:
    std::shared_ptr<SnoreSaber::Data::Private::ReplayFile> ExportCurrentReplay();
    void StopRecording();
};