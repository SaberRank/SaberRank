#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include "Utils/DelegateUtils.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/GamePause.hpp>
#include <GlobalNamespace/IGamePause.hpp>
#include <System/IDisposable.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/IInitializable.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <vector>

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::ReplaySystem::Recorders,
        PauseEventRecorder,
        System::Object,
        System::IDisposable*,
        Zenject::IInitializable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::AudioTimeSyncController>, _audioTimeSyncController);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::GamePause*, _gamePause);
    DECLARE_CTOR(ctor, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, Zenject::DiContainer* container);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);
    DECLARE_INSTANCE_METHOD(void, GamePause_didPauseEvent);
    DECLARE_INSTANCE_METHOD(void, GamePause_didResumeEvent);
    std::vector<Data::Private::PauseEvent> _pauseEvents;
    bool _paused;
    float _pauseSongTime;
    float _pauseRealtime;
    int64_t _pauseUnixStartTime;
    DelegateUtils::DelegateW<System::Action> didPauseDelegate;
    DelegateUtils::DelegateW<System::Action> didResumeDelegate;
    void FinishOpenPause();
public:
    std::vector<Data::Private::PauseEvent> Export();
};
