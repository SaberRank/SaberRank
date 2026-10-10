#include "Features/Replays/Playback/HeightPlayer.hpp"
#include "Features/Replays/ReplayTime.hpp"
#include <GlobalNamespace/PlayerSpecificSettings.hpp>
#include <System/Action_1.hpp>
#include <UnityEngine/Mathf.hpp>
#include <UnityEngine/Transform.hpp>
#include "logging.hpp"
#include <algorithm>

using namespace UnityEngine;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Playback, HeightPlayer);

namespace SnoreSaber::ReplaySystem::Playback
{
    void HeightPlayer::ctor(ReplayPlaybackContext* replayContext, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::PlayerHeightDetector* playerHeightDetector, GlobalNamespace::GameplayCoreSceneSetupData* gameplayCoreSceneSetupData)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        _playerHeightDetector = playerHeightDetector;
        _sortedHeightEvents = replayContext->GetReplayFile()->heightKeyframes;
        std::stable_sort(_sortedHeightEvents.begin(), _sortedHeightEvents.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.Time < rhs.Time;
        });
        _gameplayCoreSceneSetupData = gameplayCoreSceneSetupData;
    }

    void HeightPlayer::Initialize()
    {
        if (_gameplayCoreSceneSetupData->playerSpecificSettings->automaticPlayerHeight)
        {
            _playerHeightDetector->OnDestroy();
        }
    }

    void HeightPlayer::Tick()
    {
        optional<float> newHeight;
        while (_nextIndex < _sortedHeightEvents.size() && _audioTimeSyncController->songTime >= _sortedHeightEvents[_nextIndex].Time) {
            newHeight = _sortedHeightEvents[_nextIndex].Height;
            _nextIndex++;
        }
        if (newHeight.has_value()) {
            if (_playerHeightDetector->playerHeightDidChangeEvent) {
                _playerHeightDetector->playerHeightDidChangeEvent->Invoke(newHeight.value());
            }
        }
    }

    void HeightPlayer::TimeUpdate(float songTime)
    {
        _nextIndex = ReplayTimeSearch::CountAtOrBefore(_sortedHeightEvents, songTime, [](const auto& heightEvent) {
            return heightEvent.Time;
        });
        if (_nextIndex > 0) {
            if( _playerHeightDetector->playerHeightDidChangeEvent) {
                _playerHeightDetector->playerHeightDidChangeEvent->Invoke(_sortedHeightEvents[_nextIndex - 1].Height);
            }
        }
    }
} // namespace SnoreSaber::ReplaySystem::Playback
