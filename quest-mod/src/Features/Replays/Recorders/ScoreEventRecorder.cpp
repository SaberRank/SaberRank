#include "Features/Replays/Recorders/ScoreEventRecorder.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/PlayerHeightDetector.hpp>
#include <GlobalNamespace/PrepareLevelCompletionResults.hpp>
#include <System/Action_1.hpp>
#include <System/Action_2.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/Time.hpp>
#include "Utils/StringUtils.hpp"
#include "logging.hpp"
#include <custom-types/shared/delegate.hpp>
#include <functional>

using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Recorders, ScoreEventRecorder);

namespace SnoreSaber::ReplaySystem::Recorders
{
    void ScoreEventRecorder::ctor(AudioTimeSyncController* audioTimeSyncController, ScoreController* scoreController, ComboController* comboController, SnoreSaber::Features::Live::Replay::LiveReplayStreamingService* liveReplayStreamingService)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        _scoreController = scoreController;
        _comboController = comboController;
        _liveReplayStreamingService = liveReplayStreamingService;
    }

    void ScoreEventRecorder::Initialize()
    {
        comboDidChangeDelegate = { &ScoreEventRecorder::ComboController_comboDidChangeEvent, this };
        scoreDidChangeDelegate = { &ScoreEventRecorder::ScoreController_scoreDidChangeEvent, this };
        multiplierDidChangeDelegate = { &ScoreEventRecorder::ScoreController_multiplierDidChangeEvent, this };
        
        _comboController->___comboDidChangeEvent += comboDidChangeDelegate;
        _scoreController->___scoreDidChangeEvent += scoreDidChangeDelegate;
        _scoreController->___multiplierDidChangeEvent += multiplierDidChangeDelegate;
    }

    void ScoreEventRecorder::Dispose()
    {
        if(_comboController)
            _comboController->___comboDidChangeEvent -= comboDidChangeDelegate;
        if(_scoreController) {
            _scoreController->___scoreDidChangeEvent -= scoreDidChangeDelegate;
            _scoreController->___multiplierDidChangeEvent -= multiplierDidChangeDelegate;
        }
    }

    void ScoreEventRecorder::ComboController_comboDidChangeEvent(int combo)
    {
        _comboKeyFrames.push_back(ComboEvent(combo, _audioTimeSyncController->songTime));
        _liveReplayStreamingService->RecordCombo(_comboKeyFrames.back());
    }

    void ScoreEventRecorder::ScoreController_scoreDidChangeEvent(int rawScore, int score)
    {
        _scoreKeyFrames.push_back(ScoreEvent(rawScore, _audioTimeSyncController->songTime, _scoreController->immediateMaxPossibleMultipliedScore));
        _liveReplayStreamingService->RecordScore(_scoreKeyFrames.back());
    }

    void ScoreEventRecorder::ScoreController_multiplierDidChangeEvent(int multiplier, float nextMultiplierProgress)
    {
        _multiplierKeyFrames.push_back(MultiplierEvent(multiplier, nextMultiplierProgress, _audioTimeSyncController->songTime));
        _liveReplayStreamingService->RecordMultiplier(_multiplierKeyFrames.back());
    }

    vector<ScoreEvent> ScoreEventRecorder::ExportScoreKeyframes()
    {
        return _scoreKeyFrames;
    }

    vector<ComboEvent> ScoreEventRecorder::ExportComboKeyframes()
    {
        return _comboKeyFrames;
    }

    vector<MultiplierEvent> ScoreEventRecorder::ExportMultiplierKeyframes()
    {
        return _multiplierKeyFrames;
    }

} // namespace SnoreSaber::ReplaySystem::Recorders