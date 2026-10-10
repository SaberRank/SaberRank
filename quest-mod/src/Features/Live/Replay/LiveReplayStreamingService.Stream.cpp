#include "Features/Live/Replay/LiveReplayStreamingService.hpp"

#include "Features/Live/Ludus/Services/LudusSessionService.hpp"
#include "logging.hpp"

#include <UnityEngine/Time.hpp>

#include <algorithm>
#include <exception>
#include <random>

namespace SnoreSaber::Features::Live::Replay
{
    void LiveReplayStreamingService::Complete(GlobalNamespace::LevelCompletionResults* results, float playOutcomeTime, std::optional<Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride)
    {
        if (!_recording)
        {
            return;
        }

        _recording = false;
        if (!_streaming || !_ludus)
        {
            TrySendIdlePresence();
            ResetStream();
            return;
        }

        try
        {
            if (!Flush())
            {
                return;
            }

            V1::ReplayStreamEnd end{};
            end.Cursor = Cursor(_nextSequence++, playOutcomeTime);
            end.Completion = CompletionFromResults(results, playOutcomeOverride);
            end.Score = ScoreSummary(results);
            end.ChunkCount = _chunkCount;
            end.CumulativeEventCounts = CloneCounts(_counts);

            V1::ReplayStreamPacket packet{};
            packet.StreamId = _streamId;
            packet.Body = std::move(end);
            _ludus->SendReplayPacket(std::move(packet));

            TrySendIdlePresence();
            INFO("Live replay: Stream finished ({} chunks).", _chunkCount);
        }
        catch (const std::exception& ex)
        {
            WARN("Failed to finish live replay stream: {}", ex.what());
        }

        ResetStream();
    }

    void LiveReplayStreamingService::StopPublicPresenceStream()
    {
        _playerFollowRequested = false;
        _followViewerCount = 0;
        if (!_ludus || _ludus->IsInTournamentRoom())
        {
            return;
        }

        if (!_streaming)
        {
            _playingPresenceSent = false;
            return;
        }

        try
        {
            Flush();

            V1::ReplayStreamEnd end{};
            end.Cursor = Cursor(_nextSequence++, _lastStreamSongTime);
            end.Completion = V1::ReplayCompletion::Aborted;
            end.ChunkCount = _chunkCount;
            end.CumulativeEventCounts = CloneCounts(_counts);

            V1::ReplayStreamPacket packet{};
            packet.StreamId = _streamId;
            packet.Body = std::move(end);
            _ludus->SendReplayPacket(std::move(packet));

            INFO("Live replay: Public presence stream stopped.");
        }
        catch (const std::exception& ex)
        {
            WARN("Failed to stop public presence stream: {}", ex.what());
        }

        ResetPublicStreamState();
    }

    void LiveReplayStreamingService::PlayerFollowWasRequested(int viewerCount)
    {
        viewerCount = std::max(0, viewerCount);
        if (_playerFollowRequested && _followViewerCount == viewerCount)
        {
            TryStartStreaming();
            return;
        }

        _playerFollowRequested = true;
        _followViewerCount = viewerCount;
        INFO("Live replay: Follow requested{}.", FormatViewerSource(viewerCount));
        TryStartStreaming();
    }

    void LiveReplayStreamingService::TryStartStreaming()
    {
        if (_streaming || !_recording || !_ludus || !_ludus->IsConnectedToLudus())
        {
            return;
        }

        if (IsWaitingForTournamentMapStart())
        {
            return;
        }

        if (_ludus->IsInPublicPresence() && !_canUsePublicPresence)
        {
            return;
        }

        TrySendPlayingPresence();

        bool shouldStream = _ludus->IsInTournamentRoom() || (_canUsePublicPresence && _playerFollowRequested && _ludus->IsInPublicPresence());
        if (!shouldStream)
        {
            return;
        }

        _streamId = NewStreamId();
        _streaming = true;
        INFO("Live replay: Streaming started{}.", FormatViewerSuffix(_followViewerCount));

        V1::ReplayStreamStart start{};
        start.ProtocolVersion = 1;
        start.Player = PlayerIdentity();
        start.Beatmap = BeatmapIdentity();
        start.PayloadFormat = V1::ReplayPayloadFormat::ScoresaberStreamV1;
        start.PayloadFormatVersion = 1;
        start.PayloadCompression = V1::ReplayCompression::None;
        start.RecommendedChunkSizeBytes = RecommendedChunkSizeBytes;
        start.MaxChunkSizeBytes = MaxChunkSizeBytes;
        start.ClientStartTimeUnixMs = UnixNowMs();
        start.GameSessionId = _ludus->GameSessionId();
        start.Features = {V1::ReplayFeature::SpectatorCatchup};
        start.ReplayMetadata = StreamMetadata();
        start.ReplayExtensions = ToReplayExtensions(Data::Private::ReplayExtensionPayloads::CreateStartExtensions(*_metadata, _hsvConfig));

        V1::ReplayStreamPacket packet{};
        packet.StreamId = _streamId;
        packet.Body = std::move(start);
        _ludus->SendReplayPacket(std::move(packet));

        if (_paused)
        {
            PublishPauseState(_lastPauseSongTime);
        }
    }

    void LiveReplayStreamingService::TrySendPlayingPresence()
    {
        if (_playingPresenceSent || !_recording || !_ludus || !_ludus->IsConnectedToLudus())
        {
            return;
        }

        if (IsWaitingForTournamentMapStart())
        {
            return;
        }

        if (!_ludus->IsInTournamentRoom() && !_ludus->IsInPublicPresence())
        {
            return;
        }

        if (_ludus->IsInPublicPresence() && !_canUsePublicPresence)
        {
            return;
        }

        _playingPresenceSent = true;
        _ludus->SendPresence(V1::LudusPlayState::InGame, V1::LudusDownloadState::None, ExtractMapHash(_metadata->LevelID));
        INFO("Live replay: Playing presence sent.");
    }

    void LiveReplayStreamingService::TrySendIdlePresence()
    {
        if (!_playingPresenceSent || !_ludus)
        {
            return;
        }

        _ludus->SendPresence(V1::LudusPlayState::InMenus, V1::LudusDownloadState::None, "");
        INFO("Live replay: Idle presence sent.");
    }

    void LiveReplayStreamingService::FlushIfFull()
    {
        if (_pendingEventCount >= MaxEventsPerChunk)
        {
            Flush();
        }
    }

    void LiveReplayStreamingService::FlushIfStale()
    {
        if (_pendingEventCount == 0 || _pendingBatchStartedAt <= 0.0f)
        {
            return;
        }

        if (UnityEngine::Time::get_realtimeSinceStartup() - _pendingBatchStartedAt >= MaxChunkAgeSeconds)
        {
            Flush();
        }
    }

    void LiveReplayStreamingService::PublishPauseState(float songTime)
    {
        if (!_recording || !_streaming || !_ludus)
        {
            return;
        }

        if (_pauseStatePublished && _publishedPausedState == _paused)
        {
            return;
        }

        if (!Flush())
        {
            return;
        }

        _pendingBatch.PauseEvents.push_back(V1::ReplayPauseEvent{
                .Paused = _paused,
                .TimeSeconds = songTime,
                .ClientTimeUnixMs = UnixNowMs(),
        });
        _counts.PauseEvents++;
        MarkEventTime(songTime);

        if (!Flush())
        {
            return;
        }

        _pauseStatePublished = true;
        _publishedPausedState = _paused;
        if (_paused)
        {
            INFO("Live replay: Pause event sent.");
        }
        else
        {
            INFO("Live replay: Resume event sent.");
        }
    }

    bool LiveReplayStreamingService::Flush()
    {
        if (_pendingEventCount == 0)
        {
            return true;
        }

        if (!_ludus || !_ludus->IsConnectedToLudus() || _streamId.empty())
        {
            return true;
        }

        V1::ReplayChunk chunk{};
        chunk.Cursor = Cursor(_nextSequence, _pendingBatch.MaxTimeSeconds);
        // copy, not move: a failed send keeps the batch pending for the next flush
        chunk.Events = _pendingBatch;
        chunk.CumulativeEventCounts = CloneCounts(_counts);
        chunk.ReplayExtensions = _pendingExtensions;

        V1::ReplayStreamPacket packet{};
        packet.StreamId = _streamId;
        packet.Body = std::move(chunk);
        if (!_ludus->SendReplayPacket(std::move(packet)))
        {
            return false;
        }

        _nextSequence++;
        _chunkCount++;
        ResetBatch();
        return true;
    }

    void LiveReplayStreamingService::RestartStreamAfterConnectionLoss()
    {
        _streaming = false;
        _playingPresenceSent = false;
        _pauseStatePublished = false;
        _publishedPausedState = false;
        _streamId.clear();
        _nextSequence = 1;
        _chunkCount = 0;
        ResetBatch();
        ResetCounts();
        INFO("Live replay: Stream interrupted; waiting for Ludus reconnect.");
    }

    void LiveReplayStreamingService::ResetPublicStreamState()
    {
        _streaming = false;
        _playingPresenceSent = false;
        _pauseStatePublished = false;
        _publishedPausedState = false;
        _streamId.clear();
        _nextSequence = 1;
        _chunkCount = 0;
        _followViewerCount = 0;
        _lastStreamSongTime = 0.0f;
        _lastMaxScore = 0;
        ResetBatch();
        ResetCounts();
    }

    std::string LiveReplayStreamingService::FormatViewerSuffix(int viewerCount)
    {
        if (viewerCount <= 0)
        {
            return "";
        }

        return viewerCount == 1 ? " for 1 viewer" : fmt::format(" for {} viewers", viewerCount);
    }

    std::string LiveReplayStreamingService::FormatViewerSource(int viewerCount)
    {
        if (viewerCount <= 0)
        {
            return "";
        }

        return viewerCount == 1 ? " by 1 viewer" : fmt::format(" by {} viewers", viewerCount);
    }

    void LiveReplayStreamingService::MarkEventTime(float time)
    {
        if (_pendingEventCount == 0)
        {
            _pendingBatch.MinTimeSeconds = time;
            _pendingBatch.MaxTimeSeconds = time;
            _pendingBatchStartedAt = UnityEngine::Time::get_realtimeSinceStartup();
        }
        else
        {
            _pendingBatch.MinTimeSeconds = std::min(_pendingBatch.MinTimeSeconds, time);
            _pendingBatch.MaxTimeSeconds = std::max(_pendingBatch.MaxTimeSeconds, time);
        }

        _pendingEventCount++;
        _lastStreamSongTime = std::max(_lastStreamSongTime, _pendingBatch.MaxTimeSeconds);
    }

    std::string LiveReplayStreamingService::NewStreamId()
    {
        static constexpr char hex[] = "0123456789abcdef";
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<int> dist(0, 15);

        std::string id = "quest-";
        id.reserve(id.size() + 32);
        for (int i = 0; i < 32; i++)
        {
            id += hex[dist(gen)];
        }
        return id;
    }
}
