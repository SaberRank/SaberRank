#pragma once

#include "Core/SnoreSaberRuntimeInfo.hpp"
#include "Core/Timing/SnoreSaberClock.hpp"
#include "Features/Live/Cancellation.hpp"
#include "Features/Live/Compete/Domain/CompeteMapStartCountdown.hpp"
#include "Features/Live/Compete/Domain/CompeteOrganizerPrompt.hpp"
#include "Features/Live/Compete/Domain/CompeteRoom.hpp"
#include "Features/Live/Compete/Packets/ILudusServerCommandSession.hpp"
#include "Features/Live/Compete/Services/CompeteDirectoryService.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayControl.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayLauncher.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayState.hpp"
#include "Features/Live/Compete/Services/CompeteSongService.hpp"
#include "Features/Live/Ludus/Domain/LiveChatEntry.hpp"
#include "Features/Live/Ludus/Packets/LudusChatMessageBuffer.hpp"
#include "Features/Live/Ludus/Packets/LudusPacketDispatcher.hpp"
#include "Features/Live/Ludus/Packets/LudusPacketSender.hpp"
#include "Features/Live/Ludus/Services/ILudusSessionPacketContext.hpp"
#include "Features/Live/Ludus/Services/LudusMainThreadQueue.hpp"
#include "Features/Live/Ludus/Services/LudusMapStartCountdown.hpp"
#include "Features/Live/Ludus/Services/LudusSessionTransport.hpp"
#include "Features/Live/Protocol/Generated/Common.hpp"
#include "Features/Live/Protocol/Generated/ReplayStream.hpp"
#include "Features/Live/Protocol/Generated/RoomState.hpp"
#include "Features/Live/Protocol/LudusProto.hpp"
#include "Features/Live/Replay/LiveReplayStreamingService.hpp"
#include "Features/Players/Services/GameSessionService.hpp"
#include "Utils/Event.hpp"

#include <Zenject/IInitializable.hpp>
#include <Zenject/ITickable.hpp>
#include <System/IDisposable.hpp>
#include <custom-types/shared/macros.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace SnoreSaber::Features::Live::Ludus::Services
{
    namespace V1 = ::SnoreSaber::Live::V1;
}

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::Features::Live::Ludus::Services,
        LudusSessionService,
        System::Object,
        Zenject::IInitializable*,
        Zenject::ITickable*,
        System::IDisposable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(Players::Services::GameSessionService*, _gameSessionService);
    DECLARE_INSTANCE_FIELD_PRIVATE(Core::SnoreSaberRuntimeInfo*, _runtimeInfo);
    DECLARE_INSTANCE_FIELD_PRIVATE(Core::Timing::SnoreSaberClock*, _clock);
    DECLARE_INSTANCE_FIELD_PRIVATE(Compete::Services::CompeteSongService*, _songService);
    DECLARE_INSTANCE_FIELD_PRIVATE(Compete::Services::CompeteDirectoryService*, _directoryService);
    DECLARE_INSTANCE_FIELD_PRIVATE(Compete::Services::CompeteGameplayLauncher*, _gameplayLauncher);
    DECLARE_INSTANCE_FIELD_PRIVATE(Compete::Services::CompeteGameplayControl*, _gameplayControl);
    DECLARE_INSTANCE_FIELD_PRIVATE(Compete::Services::CompeteGameplayState*, _competeGameplayState);
    DECLARE_INSTANCE_FIELD_PRIVATE(Replay::LiveReplayStreamingService*, _replayStreamingService);
    DECLARE_CTOR(ctor,
                 Players::Services::GameSessionService* gameSessionService,
                 Core::SnoreSaberRuntimeInfo* runtimeInfo,
                 Core::Timing::SnoreSaberClock* clock,
                 Compete::Services::CompeteSongService* songService,
                 Compete::Services::CompeteDirectoryService* directoryService,
                 Compete::Services::CompeteGameplayLauncher* gameplayLauncher,
                 Compete::Services::CompeteGameplayControl* gameplayControl,
                 Compete::Services::CompeteGameplayState* competeGameplayState,
                 Replay::LiveReplayStreamingService* replayStreamingService);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Tick, &::Zenject::ITickable::Tick);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);

  public:
    // pc multicast events; room/status callbacks can fire from command handler
    // worker threads, subscribers marshal to the main thread themselves
    Utils::Event<int> PlayerFollowRequested;
    Utils::Event<const std::vector<V1::LiveRoomViewerState>&> ViewerListUpdated;
    Utils::Event<const std::shared_ptr<Compete::Domain::CompeteRoom>&> RoomUpdated;
    Utils::Event<> RoomClosed;
    Utils::Event<const Compete::Domain::CompeteOrganizerPrompt&> PromptReceived;
    Utils::Event<const std::optional<Compete::Domain::CompeteMapStartCountdown>&> MapStartCountdownChanged;
    Utils::Event<const std::vector<Domain::LiveChatEntry>&> ChatMessagesChanged;
    Utils::Event<const std::string&> StatusChanged;

    bool IsConnectedToLudus();
    bool IsInTournamentRoom();
    bool IsInPublicPresence();
    std::string CurrentLudusMatchId() const;
    std::string LocalPlayerId() const;
    std::string SnoreSaberPlayerId() const;
    std::string GameSessionId() const;
    // pc exposes the raw auth type string; quest auth types are ints (2=quest, 3=dev), -1 when unknown
    int LocalAuthType() const;
    std::vector<V1::LiveRoomViewerState> CurrentViewers() const;
    int CurrentViewerCount() const;
    std::vector<Domain::LiveChatEntry> CurrentChatMessages() const;

    // blocking (pc awaits the connect + join ack); call off the main thread
    void ConnectAndJoin(const std::shared_ptr<Compete::Domain::CompeteRoom>& room, const CancellationToken& cancellationToken);
    void Disconnect();
    void ReturnToPublicPresence();
    void ApplyPublicLivePresencePreference();
    void SetReady(bool ready);
    void SendPromptResponse(const Compete::Domain::CompeteOrganizerPrompt& prompt, bool accepted);
    bool SendChatMessage(const std::string& text);
    bool TrySetLinkedSong(const std::shared_ptr<Compete::Domain::CompeteSongSelection>& song);
    void SendPresence(V1::LudusPlayState playState, V1::LudusDownloadState downloadState, const std::string& currentMapHash);
    bool SendReplayPacket(V1::ReplayStreamPacket packet);

  private:
    static constexpr float ReconnectMinDelaySeconds = 0.5f;
    static constexpr float ReconnectMaxDelaySeconds = 10.0f;
    static constexpr size_t GameplayMainThreadQueueActionBudget = 8;
    static constexpr float MainThreadQueueBacklogLogIntervalSeconds = 2.0f;
    static constexpr int TournamentJoinAckTimeoutMs = 3000;
    static constexpr long ConnectTimeoutMs = 10000;
    static constexpr std::chrono::minutes FreshGameSessionAuthGuard{3};
    static constexpr std::chrono::hours GameSessionReconnectRefreshInterval{3};
    static constexpr std::chrono::minutes GameSessionRefreshRetryDelay{10};

    using LoginStatus = Players::Services::GameSessionService::LoginStatus;

    // pc Task _connectTask; completed exactly once, waiters block on the shared future
    struct ConnectOperation
    {
        ConnectOperation() : future(promise.get_future().share()) {}

        bool IsCompleted() const { return settled.load(); }
        void Complete()
        {
            if (!settled.exchange(true))
                promise.set_value();
        }
        void Fail(std::exception_ptr exception)
        {
            if (!settled.exchange(true))
                promise.set_exception(std::move(exception));
        }

        std::promise<void> promise;
        std::shared_future<void> future;
        std::atomic<bool> settled{false};
    };

    // pc TaskCompletionSource<CompeteRoom> + its room id
    struct PendingTournamentJoin
    {
        explicit PendingTournamentJoin(std::string id) : future(promise.get_future().share()), roomId(std::move(id)) {}

        void TrySetResult(std::shared_ptr<Compete::Domain::CompeteRoom> room)
        {
            if (!settled.exchange(true))
                promise.set_value(std::move(room));
        }
        void TrySetException(std::exception_ptr exception)
        {
            if (!settled.exchange(true))
                promise.set_exception(std::move(exception));
        }

        std::promise<std::shared_ptr<Compete::Domain::CompeteRoom>> promise;
        std::shared_future<std::shared_ptr<Compete::Domain::CompeteRoom>> future;
        std::string roomId;
        std::atomic<bool> settled{false};
    };

    void ResetSessionConnection(bool clearChatMessages);
    void RestartDefaultSessionConnection();
    void CloseTournamentRoomFromServer();
    void EnterTournamentRoom(const std::shared_ptr<Compete::Domain::CompeteRoom>& room);
    void EnsureTournamentRoomSongReady(const std::shared_ptr<Compete::Domain::CompeteRoom>& room);
    std::shared_ptr<PendingTournamentJoin> BeginPendingTournamentJoin(const std::shared_ptr<Compete::Domain::CompeteRoom>& room);
    void CompletePendingTournamentJoin(const std::shared_ptr<Compete::Domain::CompeteRoom>& room);
    void RejectPendingTournamentJoin(const std::string& message);
    void ClearPendingTournamentJoin(const std::shared_ptr<PendingTournamentJoin>& pendingJoin);
    void ApplyRoomContext(V1::LudusRoomContextType roomContext, const std::string& tournamentId, const std::string& currentMatchId);
    void ApplyClientContext(const Protocol::DecodedLudusEnvelope& envelope);
    bool RequestAuthenticationRefresh();
    void UpdateViewerList(const std::optional<std::vector<V1::LiveRoomViewerState>>& viewers);
    // runs inline on the unity main thread, otherwise enqueues to the drain tick
    void RaiseOnMain(std::function<void()> action);
    void RequestClientType(V1::LudusClientType clientType);
    void SendDownloadState(V1::LudusDownloadState state, const std::string& errorMessage);
    std::string GetLocalPlayerId() const;
    std::string PublicPresenceMatchId() const;
    // null when already connected (pc Task.CompletedTask); main thread only
    std::shared_ptr<ConnectOperation> EnsureSessionConnection(const CancellationToken& cancellationToken);
    void OpenSessionConnection(const std::shared_ptr<ConnectOperation>& operation, CancellationToken cancellationToken);
    bool ShouldRefreshAuthenticationForConnection();
    bool AuthenticationRefreshIsRequired() const;
    void MarkAuthenticationAvailable(bool refreshAttempted);
    void GameSessionStatusChanged(LoginStatus status);
    void ReceiveFailed(const std::string& message);
    void SendFailed(const std::string& message);
    void TransportDisconnected();
    void ReconnectIfDue();
    void SendHeartbeatIfDue();
    void SendConnect();
    void PrepareConnectionAttempt();
    void ScheduleReconnect(const std::string& reason, std::optional<float> delayOverrideSeconds);
    void PreserveTournamentRoomForReconnect();
    void ResetSocketSessionContext();
    void ApplyDefaultSessionRoomContext();
    V1::LudusRoomContextType DefaultSessionRoomContext() const;
    // caller must hold _stateLock (reads _tournamentRoom)
    std::string ResolveCurrentMatchIdLocked(V1::LudusRoomContextType roomContext, const std::string& currentMatchId) const;
    bool CanSendHeartbeat();
    void ClearChatMessages();
    std::string ConnectionId() const;
    void SetConnectionId(const std::string& connectionId);
    CancellationToken ConnectionCancellationToken() const;
    void CancelConnectionCancellation();
    std::string LocalPlayerDisplayName() const;
    static std::string CleanDisplayName(const std::string& value);
    V1::LivePlayerPlatform LocalPlatform() const;
    static V1::LudusClientType NormalizeClientType(V1::LudusClientType clientType);
    static std::string NormalizeLudusUrl(const std::string& configuredUrl);

    // composition owned by this service, mirroring the pc readonly fields; il2cpp
    // objects never move, so the by-value members have stable addresses
    LudusMainThreadQueue _mainThread;
    std::thread::id _unityThreadId;
    Packets::LudusChatMessageBuffer _chatMessages;
    std::unique_ptr<LudusSessionTransport> _transport;
    std::unique_ptr<Packets::LudusPacketSender> _outgoing;
    std::unique_ptr<LudusMapStartCountdown> _mapStartCountdown;
    uint64_t _loginStatusToken = 0;
    std::unique_ptr<Compete::Packets::ILudusServerCommandSession> _commandSession;
    std::unique_ptr<ILudusSessionPacketContext> _packetContext;
    std::unique_ptr<Packets::LudusPacketDispatcher<ILudusSessionPacketContext>> _packetDispatcher;

    // pc leaves session state unsynchronized (single-threaded tasks); quest command
    // handlers run on worker threads, so anything they reach sits behind _stateLock
    mutable std::mutex _stateLock;
    std::shared_ptr<Compete::Domain::CompeteRoom> _tournamentRoom;
    std::shared_ptr<Compete::Domain::CompeteRoom> _pendingTournamentRoom;
    std::shared_ptr<PendingTournamentJoin> _pendingTournamentJoin;
    std::vector<V1::LiveRoomViewerState> _currentViewers;
    std::string _connectionId;
    std::string _currentMatchId;
    std::string _currentTournamentId;
    std::optional<CancellationSource> _connectionCancellation;
    V1::LudusRoomContextType _roomContext = V1::LudusRoomContextType::Unspecified;
    V1::LudusClientType _clientType = V1::LudusClientType::Player;

    // main thread only
    std::shared_ptr<ConnectOperation> _connectOperation;
    float _heartbeatIntervalSeconds = 5.0f;
    float _nextHeartbeatAt = 0.0f;
    float _nextMainThreadQueueBacklogLogAt = 0.0f;
    std::string _nextLudusUrl;
    float _nextReconnectAt = 0.0f;
    int _reconnectAttempt = 0;
    bool _reconnectScheduled = false;
    bool _active = false;
    bool _forceAuthenticationRefreshOnNextConnect = false;
    std::chrono::system_clock::time_point _lastConnectionAuthRefreshAtUtc{};
    std::chrono::system_clock::time_point _nextConnectionAuthRefreshAttemptAtUtc{};
};
