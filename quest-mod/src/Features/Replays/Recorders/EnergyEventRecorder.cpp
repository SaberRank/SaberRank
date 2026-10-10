#include "Features/Replays/Recorders/EnergyEventRecorder.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/GameEnergyCounter.hpp>
#include <System/Action_1.hpp>
#include "logging.hpp"
#include <custom-types/shared/delegate.hpp>
#include <functional>

using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Recorders, EnergyEventRecorder);

namespace SnoreSaber::ReplaySystem::Recorders
{
    void EnergyEventRecorder::ctor(AudioTimeSyncController* audioTimeSyncController, GameEnergyCounter* gameEnergyCounter, SnoreSaber::Features::Live::Replay::LiveReplayStreamingService* liveReplayStreamingService)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        _gameEnergyCounter = gameEnergyCounter;
        _liveReplayStreamingService = liveReplayStreamingService;
    }

    void EnergyEventRecorder::Initialize()
    {
        if(_gameEnergyCounter) {
            gameEnergyDidChangeDelegate = { &EnergyEventRecorder::GameEnergyCounter_gameEnergyDidChangeEvent, this };
            _gameEnergyCounter->___gameEnergyDidChangeEvent += gameEnergyDidChangeDelegate;
        }
    }

    void EnergyEventRecorder::Dispose()
    {
        if(_gameEnergyCounter && gameEnergyDidChangeDelegate) {
            _gameEnergyCounter->___gameEnergyDidChangeEvent -= gameEnergyDidChangeDelegate;
        }
    }

    void EnergyEventRecorder::GameEnergyCounter_gameEnergyDidChangeEvent(float energy)
    {
        _energyKeyframes.push_back(EnergyEvent(energy, _audioTimeSyncController->songTime));
        _liveReplayStreamingService->RecordEnergy(_energyKeyframes.back());
    }

    vector<EnergyEvent> EnergyEventRecorder::Export()
    {
        return _energyKeyframes;
    }
} // namespace SnoreSaber::ReplaySystem::Recorders