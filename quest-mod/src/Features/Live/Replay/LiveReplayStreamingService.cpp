#include "Features/Live/Replay/LiveReplayStreamingService.hpp"

#include "Features/Live/Ludus/Services/LudusSessionService.hpp"

DEFINE_TYPE(SnoreSaber::Features::Live::Replay, LiveReplayStreamingService);

namespace SnoreSaber::Features::Live::Replay
{
    void LiveReplayStreamingService::ctor(
            Core::SnoreSaberRuntimeInfo* runtimeInfo,
            Compete::Services::CompeteGameplayState* competeGameplayState,
            Core::Timing::SnoreSaberClock* clock)
    {
        INVOKE_CTOR();
        _runtimeInfo = runtimeInfo;
        _competeGameplayState = competeGameplayState;
        _clock = clock;
        ResetBatch();
        ResetCounts();
    }

    void LiveReplayStreamingService::AttachLudus(Ludus::Services::LudusSessionService* ludus)
    {
        if (_ludus)
        {
            _ludus->PlayerFollowRequested.Remove(_ludusFollowToken);
            _ludus->ViewerListUpdated.Remove(_ludusViewersToken);
            _ludusFollowToken = 0;
            _ludusViewersToken = 0;
        }

        _ludus = ludus;
        if (_ludus)
        {
            _ludusFollowToken = _ludus->PlayerFollowRequested.Add([this](int viewerCount) { PlayerFollowWasRequested(viewerCount); });
            _ludusViewersToken = _ludus->ViewerListUpdated.Add([this](const std::vector<V1::LiveRoomViewerState>& viewers) { ViewerListWasUpdated(viewers); });
        }
    }

    void LiveReplayStreamingService::Tick()
    {
        // the session service is menu-scoped on pc; the app-scoped streamer keeps
        // ticking it during gameplay
        if (_ludus)
        {
            _ludus->Tick();
        }

        if (!_recording)
        {
            return;
        }

        if (_streaming && (!_ludus || !_ludus->IsConnectedToLudus()))
        {
            RestartStreamAfterConnectionLoss();
        }

        if (!_streaming)
        {
            TryStartStreaming();
            return;
        }

        FlushIfStale();
    }

    void LiveReplayStreamingService::ResetStream()
    {
        _streaming = false;
        _playerFollowRequested = false;
        _followViewerCount = 0;
        _playingPresenceSent = false;
        _paused = false;
        _pauseStatePublished = false;
        _publishedPausedState = false;
        _lastPauseSongTime = 0.0f;
        _lastStreamSongTime = 0.0f;
        _streamId.clear();
        _nextSequence = 1;
        _chunkCount = 0;
        _lastMaxScore = 0;
        ResetBatch();
        ResetCounts();
    }

    void LiveReplayStreamingService::ViewerListWasUpdated(const std::vector<V1::LiveRoomViewerState>& viewers)
    {
        _followViewerCount = static_cast<int>(viewers.size());
    }

    void LiveReplayStreamingService::ResetBatch()
    {
        _pendingBatch = V1::StreamReplayEventBatch{};
        _pendingExtensions.clear();
        _pendingEventCount = 0;
        _pendingBatchStartedAt = 0.0f;
    }

    void LiveReplayStreamingService::ResetCounts()
    {
        _counts = V1::ReplayEventCounts{};
    }

    bool LiveReplayStreamingService::IsWaitingForTournamentMapStart() const
    {
        return _ludus && _ludus->IsInTournamentRoom() && _competeGameplayState->IsWaitingForMapStartReady();
    }
}
