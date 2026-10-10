#pragma once

#include "Core/Gameplay/SnoreSaberPlayOutcome.hpp"
#include "Core/SnoreSaberRuntimeInfo.hpp"
#include "Core/Timing/SnoreSaberClock.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayState.hpp"
#include "Features/Live/Protocol/Generated/ReplayStream.hpp"
#include "Features/Live/Protocol/Generated/RoomState.hpp"
#include "Features/Replays/Format/ReplayExtensionPayloads.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"

#include <GlobalNamespace/LevelCompletionResults.hpp>
#include <Zenject/ITickable.hpp>
#include <custom-types/shared/macros.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace SnoreSaber::Features::Live::Ludus::Services
{
    class LudusSessionService;
}

namespace SnoreSaber::Features::Live::Replay
{
    namespace V1 = ::SnoreSaber::Live::V1;
}

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::Features::Live::Replay,
        LiveReplayStreamingService,
        System::Object,
        Zenject::ITickable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(Core::SnoreSaberRuntimeInfo*, _runtimeInfo);
    DECLARE_INSTANCE_FIELD_PRIVATE(Compete::Services::CompeteGameplayState*, _competeGameplayState);
    DECLARE_INSTANCE_FIELD_PRIVATE(Core::Timing::SnoreSaberClock*, _clock);
    DECLARE_CTOR(ctor,
                 Core::SnoreSaberRuntimeInfo* runtimeInfo,
                 Compete::Services::CompeteGameplayState* competeGameplayState,
                 Core::Timing::SnoreSaberClock* clock);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Tick, &::Zenject::ITickable::Tick);

  public:
    void AttachLudus(Ludus::Services::LudusSessionService* ludus);

    // recording surface, driven by the replay recorders on the gameplay main thread
    void Begin(std::shared_ptr<Data::Private::Metadata> metadata, std::vector<char> hsvConfig);
    void SetPaused(bool paused, float songTime);
    void RecordPose(const Data::Private::VRPoseGroup& frame);
    void RecordHeight(const Data::Private::HeightEvent& height);
    void RecordNote(const Data::Private::NoteEvent& note);
    void RecordScore(const Data::Private::ScoreEvent& score);
    void RecordCombo(const Data::Private::ComboEvent& combo);
    void RecordMultiplier(const Data::Private::MultiplierEvent& multiplier);
    void RecordEnergy(const Data::Private::EnergyEvent& energy);
    void RecordWall(const Data::Private::WallEvent& wall);
    void Complete(GlobalNamespace::LevelCompletionResults* results, float playOutcomeTime, std::optional<Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride = std::nullopt);
    void StopPublicPresenceStream();

  private:
    static constexpr int MaxEventsPerChunk = 64;
    static constexpr float MaxChunkAgeSeconds = 0.25f;
    static constexpr uint32_t RecommendedChunkSizeBytes = 64 * 1024;
    static constexpr uint32_t MaxChunkSizeBytes = 256 * 1024;

    void ResetStream();
    void ViewerListWasUpdated(const std::vector<V1::LiveRoomViewerState>& viewers);
    void ResetBatch();
    void ResetCounts();
    bool IsWaitingForTournamentMapStart() const;

    bool CanRecordEvent();

    void PlayerFollowWasRequested(int viewerCount);
    void TryStartStreaming();
    void TrySendPlayingPresence();
    void TrySendIdlePresence();
    void FlushIfFull();
    void FlushIfStale();
    void PublishPauseState(float songTime);
    bool Flush();
    void RestartStreamAfterConnectionLoss();
    void ResetPublicStreamState();
    static std::string FormatViewerSuffix(int viewerCount);
    static std::string FormatViewerSource(int viewerCount);
    void MarkEventTime(float time);

    V1::ReplayCursor Cursor(uint64_t sequence, float songTime);
    V1::PlayerIdentity PlayerIdentity();
    V1::BeatmapIdentity BeatmapIdentity();
    V1::StreamReplayMetadata StreamMetadata();
    V1::ReplayScoreSummary ScoreSummary(GlobalNamespace::LevelCompletionResults* results);
    bool CanUsePublicPresenceForCurrentLevel() const;
    int64_t UnixNowMs();

    static std::vector<V1::ReplayExtension> ToReplayExtensions(const std::vector<Data::Private::ReplayExtensionPayloads::ReplayExtensionEntry>& entries);
    static V1::ReplayExtension ToReplayExtension(const Data::Private::ReplayExtensionPayloads::ReplayExtensionEntry& entry);
    static V1::ReplayPoseFrame ToReplayPoseFrame(const Data::Private::VRPoseGroup& frame);
    static V1::ReplayPose ToReplayPose(const Data::Private::VRPose& pose);
    static V1::ReplayNoteEvent ToReplayNoteEvent(const Data::Private::NoteEvent& note);
    static V1::ReplayVector3 ToReplayVector(const Data::Private::VRPosition& position);
    static V1::ReplayVector3 ToReplayVector(const std::optional<Data::Private::VRPosition>& position);
    static V1::ReplayQuaternion ToReplayQuaternion(const Data::Private::VRRotation& rotation);
    static V1::ReplayQuaternion ToReplayQuaternion(const std::optional<Data::Private::VRRotation>& rotation);
    static std::optional<V1::ReplayColor> ToReplayColor(const std::optional<UnityEngine::Color>& color);
    static V1::ReplayNoteEventType ToReplayNoteEventType(Data::Private::NoteEventType type);
    static V1::ReplayCompletion CompletionFromResults(GlobalNamespace::LevelCompletionResults* results, std::optional<Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride);
    static V1::ReplayCompletion CompletionFromPlayOutcome(Core::Gameplay::SnoreSaberPlayOutcome outcome);
    static V1::ReplayPlatform PlatformFromAuthType(int authType);
    static std::string ExtractMapHash(const std::string& levelId);
    static std::string ExtractLeaderboardId(const std::string& levelId);
    static std::string DifficultyName(int difficulty);
    static V1::ReplayEventCounts CloneCounts(const V1::ReplayEventCounts& counts);
    static uint32_t ToUint(int value);
    static std::string NewStreamId();

    // app-scoped singleton rooted by the installer; attached by the session service
    Ludus::Services::LudusSessionService* _ludus = nullptr;
    uint64_t _ludusFollowToken = 0;
    uint64_t _ludusViewersToken = 0;

    std::shared_ptr<Data::Private::Metadata> _metadata;
    bool _canUsePublicPresence = false;
    std::vector<char> _hsvConfig;
    V1::StreamReplayEventBatch _pendingBatch;
    std::vector<V1::ReplayExtension> _pendingExtensions;
    V1::ReplayEventCounts _counts;
    std::string _streamId;
    bool _recording = false;
    bool _streaming = false;
    bool _playerFollowRequested = false;
    int _followViewerCount = 0;
    bool _playingPresenceSent = false;
    bool _paused = false;
    bool _pauseStatePublished = false;
    bool _publishedPausedState = false;
    float _lastPauseSongTime = 0.0f;
    float _lastStreamSongTime = 0.0f;
    uint64_t _nextSequence = 1;
    uint64_t _chunkCount = 0;
    int _pendingEventCount = 0;
    float _pendingBatchStartedAt = 0.0f;
    uint32_t _lastMaxScore = 0;
};
