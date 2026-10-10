
#include "Features/Replays/Recorders/MainRecorder.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include "Features/Replays/Format/ReplayWriter.hpp"
#include "Features/Replays/Recorders/EnergyEventRecorder.hpp"
#include "Features/Replays/Recorders/HeightEventRecorder.hpp"
#include "Features/Replays/Recorders/MetadataRecorder.hpp"
#include "Features/Replays/Recorders/NoteEventRecorder.hpp"
#include "Features/Replays/Recorders/PauseEventRecorder.hpp"
#include "Features/Replays/Recorders/PoseRecorder.hpp"
#include "Features/Replays/Recorders/ScoreEventRecorder.hpp"
#include "Features/Replays/Recorders/WallEventRecorder.hpp"
#include "Features/Replays/Recorders/HsvConfigRecorder.hpp"
#include "Data/Private/Settings.hpp"
#include "Services/ReplayService.hpp"
#include <System/Action.hpp>
#include <beatsaber-hook/shared/utils/hooking.hpp>
#include <custom-types/shared/delegate.hpp>
#include "logging.hpp"

using namespace SnoreSaber::ReplaySystem;
using namespace SnoreSaber::Services;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Recorders, MainRecorder);

namespace SnoreSaber::ReplaySystem::Recorders
{
    void MainRecorder::ctor(PoseRecorder* poseRecorder, MetadataRecorder* metadataRecorder, NoteEventRecorder* noteEventRecorder, ScoreEventRecorder* scoreEventRecorder, HeightEventRecorder* heightEventRecorder, EnergyEventRecorder* energyEventRecorder, PauseEventRecorder* pauseEventRecorder, WallEventRecorder* wallEventRecorder, HsvConfigRecorder* hsvConfigRecorder, SnoreSaber::Features::Live::Replay::LiveReplayStreamingService* liveReplayStreamingService, Zenject::DiContainer* container)
    {
        INVOKE_CTOR();
        _poseRecorder = poseRecorder;
        _metadataRecorder = metadataRecorder;
        _noteEventRecorder = noteEventRecorder;
        _scoreEventRecorder = scoreEventRecorder;
        _heightEventRecorder = heightEventRecorder;
        _energyEventRecorder = energyEventRecorder;
        _pauseEventRecorder = pauseEventRecorder;
        _wallEventRecorder = wallEventRecorder;
        _hsvConfigRecorder = hsvConfigRecorder;
        _liveReplayStreamingService = liveReplayStreamingService;
        auto gamePause = container->TryResolve<GlobalNamespace::IGamePause*>();
        _gamePause = gamePause ? il2cpp_utils::try_cast<GlobalNamespace::GamePause>(gamePause).value_or(nullptr) : nullptr;
        _audioTimeSyncController = container->TryResolve<GlobalNamespace::AudioTimeSyncController*>();
    }

    void MainRecorder::Initialize() {
        ReplayService::NewPlayStarted(this);
        _hsvConfig = Settings::shareHsvProfiles ? _hsvConfigRecorder->Export() : std::vector<char>();
        _liveReplayStreamingService->Begin(_metadataRecorder->Export(), _hsvConfig);
        if (_gamePause) {
            didPauseDelegate = { &MainRecorder::GamePause_didPauseEvent, this };
            didResumeDelegate = { &MainRecorder::GamePause_didResumeEvent, this };
            _gamePause->___didPauseEvent += didPauseDelegate;
            _gamePause->___didResumeEvent += didResumeDelegate;
        }
    }

    void MainRecorder::Dispose() {
        if (_gamePause) {
            if (didPauseDelegate) {
                _gamePause->___didPauseEvent -= didPauseDelegate;
            }
            if (didResumeDelegate) {
                _gamePause->___didResumeEvent -= didResumeDelegate;
            }
        }
    }

    void MainRecorder::GamePause_didPauseEvent() {
        _liveReplayStreamingService->SetPaused(true, CurrentSongTime());
    }

    void MainRecorder::GamePause_didResumeEvent() {
        _liveReplayStreamingService->SetPaused(false, CurrentSongTime());
    }

    float MainRecorder::CurrentSongTime() {
        return _audioTimeSyncController ? _audioTimeSyncController->songTime : 0.0f;
    }

    std::shared_ptr<ReplayFile> MainRecorder::ExportCurrentReplay()
    {
        std::shared_ptr<Metadata> metadata = _metadataRecorder->Export();
        vector<VRPoseGroup> poseKeyFrames = _poseRecorder->Export();
        vector<HeightEvent> heightKeyframes = _heightEventRecorder->Export();
        vector<NoteEvent> noteKeyframes = _noteEventRecorder->Export();
        vector<ScoreEvent> scoreKeyframes = _scoreEventRecorder->ExportScoreKeyframes();
        vector<ComboEvent> comboKeyframes = _scoreEventRecorder->ExportComboKeyframes();
        vector<MultiplierEvent> multiplierKeyframes = _scoreEventRecorder->ExportMultiplierKeyframes();
        vector<EnergyEvent> energyKeyframes = _energyEventRecorder->Export();
        auto file = std::make_shared<ReplayFile>(metadata, poseKeyFrames, heightKeyframes, noteKeyframes, scoreKeyframes, comboKeyframes, multiplierKeyframes, energyKeyframes);
        file->pauseKeyframes = _pauseEventRecorder->Export();
        file->wallKeyframes = _wallEventRecorder->Export();
        file->hsvConfig = _hsvConfig;
        return file;
    }

    void MainRecorder::StopRecording() {
        _poseRecorder->StopRecording();
    }
} // namespace SnoreSaber::ReplaySystem::Recorders