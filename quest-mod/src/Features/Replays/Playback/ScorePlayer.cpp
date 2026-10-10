
#include "Features/Replays/Playback/ScorePlayer.hpp"
#include "Features/Replays/ReplayTime.hpp"
#include <GlobalNamespace/GameplayModifiersModelSO.hpp>
#include <GlobalNamespace/ScoreModel.hpp>
#include <System/Action_2.hpp>
#include "Core/Gameplay/SnoreSaberScoreModel.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"
#include "logging.hpp"
#include <algorithm>
#include <metacore/shared/internals.hpp>

using namespace UnityEngine;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Playback, ScorePlayer);

namespace SnoreSaber::ReplaySystem::Playback
{
    void ScorePlayer::ctor(ReplayPlaybackContext* replayContext, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::ScoreController* scoreController, GlobalNamespace::GameEnergyCounter* gameEnergyCounter)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        _scoreController = scoreController;
        _gameEnergyCounter = gameEnergyCounter;
        auto replayFile = replayContext->GetReplayFile();
        _sortedScoreEvents = replayFile->scoreKeyframes;
        _sortedNoteEvents = replayFile->noteKeyframes;
        std::stable_sort(_sortedScoreEvents.begin(), _sortedScoreEvents.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.Time < rhs.Time;
        });
        std::stable_sort(_sortedNoteEvents.begin(), _sortedNoteEvents.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.Time < rhs.Time;
        });
        for (const auto& noteEvent : _sortedNoteEvents) {
            if (ReplayTimeSearch::IsScoringNoteEvent(noteEvent)) {
                _scoringNoteEventTimes.push_back(noteEvent.Time);
            }
        }
    }

    void ScorePlayer::Tick()
    {
        optional<int> recentMultipliedScore;
        optional<int> recentImmediateMaxPossibleScore;
        while (_nextIndex < _sortedScoreEvents.size() && _audioTimeSyncController->songTime >= _sortedScoreEvents[_nextIndex].Time) {
            ScoreEvent activeEvent = _sortedScoreEvents[_nextIndex++];
            recentMultipliedScore = activeEvent.Score;
            recentImmediateMaxPossibleScore = activeEvent.ImmediateMaxPossibleScore;
        }

        if (recentMultipliedScore.has_value()) {
            UpdateScore(recentMultipliedScore.value(), recentImmediateMaxPossibleScore, _audioTimeSyncController->songTime);
        }
    }

    void ScorePlayer::TimeUpdate(float newTime)
    {
        ScorePlayer::UpdateMultiplier();

        _nextIndex = ReplayTimeSearch::CountAtOrBefore(_sortedScoreEvents, newTime, [](const auto& scoreEvent) {
            return scoreEvent.Time;
        });

        if (_nextIndex > 0) {
            auto scoreEvent = _sortedScoreEvents[_nextIndex - 1];
            UpdateScore(scoreEvent.Score, scoreEvent.ImmediateMaxPossibleScore, newTime);
        } else {
            UpdateScore(0, 0, newTime);
        }
    }

    void ScorePlayer::UpdateMultiplier()
    {
        float totalMultiplier = _scoreController->_gameplayModifiersModel->GetTotalMultiplier(_scoreController->_gameplayModifierParams, _gameEnergyCounter->energy);
        _scoreController->_prevMultiplierFromModifiers = totalMultiplier;
        MetaCore::Internals::multiplier = totalMultiplier;
    }

    void ScorePlayer::UpdateScore(int newScore, optional<int> immediateMaxPossibleScore, float time)
    {
        int immediate;
        if (immediateMaxPossibleScore.has_value()) {
            immediate = immediateMaxPossibleScore.value();
        } else {
            immediate = SnoreSaber::Core::Gameplay::SnoreSaberScoreModel::OldMaxRawScoreForNumberOfNotes(CalculatePostNoteCountForTime(time));
        }
        float multiplier = _scoreController->_prevMultiplierFromModifiers;

        int newModifiedScore = GlobalNamespace::ScoreModel::GetModifiedScoreForGameplayModifiersScoreMultiplier(newScore, multiplier);

        _scoreController->_multipliedScore = newScore;
        _scoreController->_immediateMaxPossibleMultipliedScore = immediate;
        _scoreController->_modifiedScore = newModifiedScore;
        _scoreController->_immediateMaxPossibleModifiedScore = GlobalNamespace::ScoreModel::GetModifiedScoreForGameplayModifiersScoreMultiplier(immediate, multiplier);

        if (_scoreController->scoreDidChangeEvent) {
            _scoreController->scoreDidChangeEvent->Invoke(newScore, newModifiedScore);
        }

        _scoreController->____playerHeadAndObstacleInteraction->____intersectingObstacles->Clear();

        // this sucks but metacore expects left/right scores separately
        // will figure out proper calculations, and probably make a full util for updating metacore values later
        MetaCore::Internals::songMaxScore = immediate;
        
        auto splitScore = [](int total) -> std::pair<int, int> {
            return {total / 2, total - total / 2};
        };
        
        auto [leftScore, rightScore] = splitScore(newModifiedScore);
        MetaCore::Internals::leftScore = leftScore;
        MetaCore::Internals::rightScore = rightScore;

        auto [leftMaxScore, rightMaxScore] = splitScore(immediate);
        MetaCore::Internals::leftMaxScore = leftMaxScore;
        MetaCore::Internals::rightMaxScore = rightMaxScore;
    }
    
    int ScorePlayer::CalculatePostNoteCountForTime(float time)
    {
        return ReplayTimeSearch::CountAtOrBefore(_scoringNoteEventTimes, time);
    }
} // namespace SnoreSaber::ReplaySystem::Playback
