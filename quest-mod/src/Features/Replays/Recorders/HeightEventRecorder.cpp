#include "Features/Replays/Recorders/HeightEventRecorder.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/PlayerHeightDetector.hpp>
#include <System/Action_1.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/Time.hpp>
#include "Utils/StringUtils.hpp"
#include "logging.hpp"
#include <custom-types/shared/delegate.hpp>
#include <functional>
#include "Utils/DelegateUtils.hpp"

using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Recorders, HeightEventRecorder);

namespace SnoreSaber::ReplaySystem::Recorders
{
    void HeightEventRecorder::ctor(AudioTimeSyncController* audioTimeSyncController, Zenject::DiContainer* container, SnoreSaber::Features::Live::Replay::LiveReplayStreamingService* liveReplayStreamingService)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        _playerHeightDetector = container->TryResolve<PlayerHeightDetector*>();
        _liveReplayStreamingService = liveReplayStreamingService;
    }
    
    void HeightEventRecorder::Initialize()
    {
        if(_playerHeightDetector) {
            playerHeightDidChangeDelegate = {&HeightEventRecorder::PlayerHeightDetector_playerHeightDidChangeEvent, this};
            _playerHeightDetector->___playerHeightDidChangeEvent += playerHeightDidChangeDelegate;
        }
    }

    void HeightEventRecorder::Dispose()
    {
        if(_playerHeightDetector && playerHeightDidChangeDelegate) {
            _playerHeightDetector->___playerHeightDidChangeEvent -= playerHeightDidChangeDelegate;
        }
    }

    void HeightEventRecorder::PlayerHeightDetector_playerHeightDidChangeEvent(float newHeight)
    {
        _heightKeyframes.push_back(HeightEvent(newHeight, _audioTimeSyncController->songTime));
        _liveReplayStreamingService->RecordHeight(_heightKeyframes.back());
    }

    vector<HeightEvent> HeightEventRecorder::Export()
    {
        return _heightKeyframes;
    }
} // namespace SnoreSaber::ReplaySystem::Recorders