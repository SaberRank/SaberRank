#include "Features/Replays/Playback/MultiplierPlayer.hpp"
#include "Features/Replays/ReplayTime.hpp"
#include <System/Action_2.hpp>
#include "logging.hpp"
#include <algorithm>
#include <metacore/shared/internals.hpp>

using namespace UnityEngine;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Playback, MultiplierPlayer);

namespace SnoreSaber::ReplaySystem::Playback
{
    void MultiplierPlayer::ctor(ReplayPlaybackContext* replayContext, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::ScoreController* scoreController)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        _scoreController = scoreController;
        _sortedMultiplierEvents = replayContext->GetReplayFile()->multiplierKeyframes;
        std::stable_sort(_sortedMultiplierEvents.begin(), _sortedMultiplierEvents.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.Time < rhs.Time;
        });
    }

    void MultiplierPlayer::TimeUpdate(float newTime)
    {
        if (_sortedMultiplierEvents.empty())
        {
            return;
        }

        int nextIndex = ReplayTimeSearch::CountAtOrBefore(_sortedMultiplierEvents, newTime, [](const auto& multiplierEvent) {
            return multiplierEvent.Time;
        });
        if (nextIndex == 0) {
            UpdateMultiplier(1, 0.0f);
            return;
        }

        auto multiplierEvent = _sortedMultiplierEvents[nextIndex - 1];
        UpdateMultiplier(multiplierEvent.Multiplier, multiplierEvent.NextMultiplierProgress);
    }

    void MultiplierPlayer::UpdateMultiplier(int multiplier, float progress)
    {
        auto counter = _scoreController->_scoreMultiplierCounter;
        counter->____multiplier = multiplier;
        counter->____multiplierIncreaseMaxProgress = multiplier * 2;
        counter->____multiplierIncreaseProgress = (int)(progress * (multiplier * 2));
        if (_scoreController->multiplierDidChangeEvent)
        {
            _scoreController->multiplierDidChangeEvent->Invoke(multiplier, progress);
        }
        MetaCore::Internals::multiplier = multiplier;
        MetaCore::Internals::multiplierProgress = progress;
    }
} // namespace SnoreSaber::ReplaySystem::Playback
