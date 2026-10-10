#include "Features/Replays/Recorders/WallEventRecorder.hpp"

#include <GlobalNamespace/ObstacleData.hpp>
#include <System/Action_1.hpp>
#include <algorithm>
#include <custom-types/shared/delegate.hpp>

using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Recorders, WallEventRecorder);

namespace SnoreSaber::ReplaySystem::Recorders
{
    void WallEventRecorder::ctor(AudioTimeSyncController* audioTimeSyncController, Zenject::DiContainer* container, SnoreSaber::Features::Live::Replay::LiveReplayStreamingService* liveReplayStreamingService)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        _playerHeadAndObstacleInteraction = container->TryResolve<PlayerHeadAndObstacleInteraction*>();
        _gameEnergyCounter = container->TryResolve<GameEnergyCounter*>();
        _liveReplayStreamingService = liveReplayStreamingService;
    }

    void WallEventRecorder::Initialize()
    {
        if (_playerHeadAndObstacleInteraction) {
            _headInObstacle = _playerHeadAndObstacleInteraction->playerHeadIsInObstacle;
            headDidEnterObstacleDelegate = { &WallEventRecorder::HeadDidEnterObstacleEvent, this };
            _playerHeadAndObstacleInteraction->___headDidEnterObstacleEvent += headDidEnterObstacleDelegate;
        }
    }

    void WallEventRecorder::Tick()
    {
        if (!_playerHeadAndObstacleInteraction) {
            return;
        }

        bool headInObstacle = _playerHeadAndObstacleInteraction->playerHeadIsInObstacle;
        if (_headInObstacle && !headInObstacle) {
            CloseOpenContacts();
        }
        _headInObstacle = headInObstacle;
    }

    void WallEventRecorder::Dispose()
    {
        if (_playerHeadAndObstacleInteraction && headDidEnterObstacleDelegate) {
            _playerHeadAndObstacleInteraction->___headDidEnterObstacleEvent -= headDidEnterObstacleDelegate;
        }
    }

    void WallEventRecorder::HeadDidEnterObstacleEvent(UnityW<ObstacleController> obstacleController)
    {
        if (!obstacleController) {
            return;
        }

        ObstacleData* obstacle = obstacleController->obstacleData;
        WallEvent wallEvent;
        wallEvent.Time = _audioTimeSyncController->songTime;
        wallEvent.ExitTime = wallEvent.Time;
        wallEvent.Energy = CurrentEnergy();
        wallEvent.ObstacleTime = obstacle->time;
        wallEvent.ObstacleDuration = obstacle->duration;
        wallEvent.LineIndex = obstacle->lineIndex;
        wallEvent.LineLayer = (int)obstacle->lineLayer;
        wallEvent.Width = obstacle->width;
        wallEvent.Height = obstacle->height;
        _wallEvents.push_back(wallEvent);
        _openContactIndexes.push_back((int)_wallEvents.size() - 1);
        _headInObstacle = true;
    }

    void WallEventRecorder::CloseOpenContacts()
    {
        if (_openContactIndexes.empty()) {
            return;
        }

        float exitTime = _audioTimeSyncController->songTime;
        float energy = CurrentEnergy();
        for (int index : _openContactIndexes) {
            WallEvent& wallEvent = _wallEvents[index];
            wallEvent.ExitTime = std::max(wallEvent.Time, exitTime);
            wallEvent.Energy = energy;
            _liveReplayStreamingService->RecordWall(wallEvent);
        }
        _openContactIndexes.clear();
    }

    float WallEventRecorder::CurrentEnergy()
    {
        return _gameEnergyCounter ? _gameEnergyCounter->energy : 0.0f;
    }

    vector<WallEvent> WallEventRecorder::Export()
    {
        CloseOpenContacts();
        return _wallEvents;
    }
} // namespace SnoreSaber::ReplaySystem::Recorders
