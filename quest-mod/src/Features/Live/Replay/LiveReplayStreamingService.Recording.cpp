#include "Features/Live/Replay/LiveReplayStreamingService.hpp"

#include "Features/Live/Ludus/Services/LudusSessionService.hpp"
#include "logging.hpp"

#include <utility>

namespace SnoreSaber::Features::Live::Replay
{
    void LiveReplayStreamingService::Begin(std::shared_ptr<Data::Private::Metadata> metadata, std::vector<char> hsvConfig)
    {
        _metadata = std::move(metadata);
        _hsvConfig = std::move(hsvConfig);
        _canUsePublicPresence = CanUsePublicPresenceForCurrentLevel();
        _recording = true;
        _streaming = false;
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

        INFO("Live replay: Recording started.");
        TrySendPlayingPresence();
        TryStartStreaming();
    }

    void LiveReplayStreamingService::SetPaused(bool paused, float songTime)
    {
        if (!_recording || _paused == paused)
        {
            return;
        }

        _paused = paused;
        _pauseStatePublished = false;
        _lastPauseSongTime = songTime;
        TrySendPlayingPresence();
        TryStartStreaming();
        PublishPauseState(songTime);
    }

    void LiveReplayStreamingService::RecordPose(const Data::Private::VRPoseGroup& frame)
    {
        if (_paused)
        {
            TryStartStreaming();
            return;
        }

        if (!CanRecordEvent())
        {
            return;
        }

        _pendingBatch.PoseFrames.push_back(ToReplayPoseFrame(frame));
        _counts.PoseFrames++;
        MarkEventTime(frame.Time);
        FlushIfFull();
    }

    void LiveReplayStreamingService::RecordHeight(const Data::Private::HeightEvent& height)
    {
        if (!CanRecordEvent())
        {
            return;
        }

        _pendingBatch.HeightEvents.push_back(V1::ReplayHeightEvent{.Height = height.Height, .TimeSeconds = height.Time});
        _counts.HeightEvents++;
        MarkEventTime(height.Time);
        FlushIfFull();
    }

    void LiveReplayStreamingService::RecordNote(const Data::Private::NoteEvent& note)
    {
        if (!CanRecordEvent())
        {
            return;
        }

        _pendingBatch.NoteEvents.push_back(ToReplayNoteEvent(note));
        _counts.NoteEvents++;
        MarkEventTime(note.Time);
        FlushIfFull();
    }

    void LiveReplayStreamingService::RecordScore(const Data::Private::ScoreEvent& score)
    {
        if (!CanRecordEvent())
        {
            return;
        }

        if (score.ImmediateMaxPossibleScore.has_value() && *score.ImmediateMaxPossibleScore > 0)
        {
            _lastMaxScore = static_cast<uint32_t>(*score.ImmediateMaxPossibleScore);
        }

        V1::ReplayScoreEvent scoreEvent{.Score = score.Score, .TimeSeconds = score.Time};
        if (score.ImmediateMaxPossibleScore.has_value())
        {
            scoreEvent.ImmediateMaxPossibleScore = *score.ImmediateMaxPossibleScore;
        }

        _pendingBatch.ScoreEvents.push_back(scoreEvent);
        _counts.ScoreEvents++;
        MarkEventTime(score.Time);
        FlushIfFull();
    }

    void LiveReplayStreamingService::RecordCombo(const Data::Private::ComboEvent& combo)
    {
        if (!CanRecordEvent())
        {
            return;
        }

        _pendingBatch.ComboEvents.push_back(V1::ReplayComboEvent{.Combo = combo.Combo, .TimeSeconds = combo.Time});
        _counts.ComboEvents++;
        MarkEventTime(combo.Time);
        FlushIfFull();
    }

    void LiveReplayStreamingService::RecordMultiplier(const Data::Private::MultiplierEvent& multiplier)
    {
        if (!CanRecordEvent())
        {
            return;
        }

        _pendingBatch.MultiplierEvents.push_back(V1::ReplayMultiplierEvent{
                .Multiplier = multiplier.Multiplier,
                .NextMultiplierProgress = multiplier.NextMultiplierProgress,
                .TimeSeconds = multiplier.Time,
        });
        _counts.MultiplierEvents++;
        MarkEventTime(multiplier.Time);
        FlushIfFull();
    }

    void LiveReplayStreamingService::RecordEnergy(const Data::Private::EnergyEvent& energy)
    {
        if (!CanRecordEvent())
        {
            return;
        }

        _pendingBatch.EnergyEvents.push_back(V1::ReplayEnergyEvent{.Energy = energy.Energy, .TimeSeconds = energy.Time});
        _counts.EnergyEvents++;
        MarkEventTime(energy.Time);
        FlushIfFull();
    }

    void LiveReplayStreamingService::RecordWall(const Data::Private::WallEvent& wall)
    {
        if (!CanRecordEvent())
        {
            return;
        }

        auto entry = Data::Private::ReplayExtensionPayloads::CreateWallEvents({wall});
        _pendingExtensions.push_back(ToReplayExtension(entry));
        MarkEventTime(wall.ExitTime);
        FlushIfFull();
    }

    bool LiveReplayStreamingService::CanRecordEvent()
    {
        if (!_recording)
        {
            return false;
        }

        if (_streaming && (!_ludus || !_ludus->IsConnectedToLudus()))
        {
            RestartStreamAfterConnectionLoss();
            return false;
        }

        TrySendPlayingPresence();
        TryStartStreaming();
        return _streaming;
    }
}
