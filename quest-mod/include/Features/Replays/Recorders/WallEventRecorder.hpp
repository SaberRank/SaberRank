#pragma once

#include "Features/Live/Replay/LiveReplayStreamingService.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include "Utils/DelegateUtils.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/GameEnergyCounter.hpp>
#include <GlobalNamespace/ObstacleController.hpp>
#include <GlobalNamespace/PlayerHeadAndObstacleInteraction.hpp>
#include <System/IDisposable.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/IInitializable.hpp>
#include <Zenject/ITickable.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <vector>

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::ReplaySystem::Recorders,
        WallEventRecorder,
        System::Object,
        System::IDisposable*,
        Zenject::IInitializable*,
        Zenject::ITickable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::AudioTimeSyncController>, _audioTimeSyncController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::PlayerHeadAndObstacleInteraction>, _playerHeadAndObstacleInteraction);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::GameEnergyCounter>, _gameEnergyCounter);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Live::Replay::LiveReplayStreamingService*, _liveReplayStreamingService);
    DECLARE_CTOR(ctor, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, Zenject::DiContainer* container, SnoreSaber::Features::Live::Replay::LiveReplayStreamingService* liveReplayStreamingService);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Tick, &::Zenject::ITickable::Tick);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);
    DECLARE_INSTANCE_METHOD(void, HeadDidEnterObstacleEvent, UnityW<GlobalNamespace::ObstacleController> obstacleController);
    std::vector<Data::Private::WallEvent> _wallEvents;
    std::vector<int> _openContactIndexes;
    bool _headInObstacle;
    DelegateUtils::DelegateW<System::Action_1<UnityW<GlobalNamespace::ObstacleController>>> headDidEnterObstacleDelegate;
    float CurrentEnergy();
    void CloseOpenContacts();
public:
    std::vector<Data::Private::WallEvent> Export();
};
