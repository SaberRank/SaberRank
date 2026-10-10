#include "Features/Live/Ludus/Services/LudusSessionService.hpp"

#include "Core/Api/SnoreSaberUrls.hpp"
#include "Data/Private/Settings.hpp"
#include "Features/Live/Compete/Packets/CompeteLudusCommandSession.hpp"
#include "Features/Live/Compete/Packets/CompeteLudusPacketDispatcher.hpp"
#include "Features/Live/Compete/Packets/Handlers/LoadSongCommandHandler.hpp"
#include "Features/Live/Ludus/Services/LudusInstalledMods.hpp"
#include "Features/Live/Ludus/Services/LudusSessionPacketContext.hpp"
#include "Utils/AsyncUtils.hpp"
#include "logging.hpp"

#include <UnityEngine/Time.hpp>

#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>
#include <regex>
#include <stdexcept>

DEFINE_TYPE(SnoreSaber::Features::Live::Ludus::Services, LudusSessionService);

namespace SnoreSaber::Features::Live::Ludus::Services
{
    namespace
    {
        bool IsSpace(char c)
        {
            return c == ' ' || c == '\n' || c == '\r' || c == '\t';
        }

        std::string Trim(const std::string& value)
        {
            size_t start = 0;
            size_t end = value.size();
            while (start < end && IsSpace(value[start]))
            {
                start++;
            }
            while (end > start && IsSpace(value[end - 1]))
            {
                end--;
            }
            return value.substr(start, end - start);
        }

        bool StartsWithCaseInsensitive(const std::string& value, const std::string& prefix)
        {
            if (value.size() < prefix.size())
            {
                return false;
            }
            return std::equal(prefix.begin(), prefix.end(), value.begin(), [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
        }

        // pc awaits main-thread continuations; quest blocks the worker until the
        // scheduled action ran (never call from the main thread)
        void RunOnMainBlocking(const std::function<void()>& action)
        {
            std::promise<void> done;
            auto future = done.get_future();
            SnoreSaber::Utils::Async::Main([&action, &done] {
                try
                {
                    action();
                    done.set_value();
                }
                catch (...)
                {
                    done.set_exception(std::current_exception());
                }
            });
            future.get();
        }

        template <typename T>
        void WaitWithCancellation(const std::shared_future<T>& future, const SnoreSaber::Features::Live::CancellationToken& cancellationToken)
        {
            while (future.wait_for(std::chrono::milliseconds(25)) != std::future_status::ready)
            {
                cancellationToken.ThrowIfCancellationRequested();
            }
        }
    }

    void LudusSessionService::ctor(
            Players::Services::GameSessionService* gameSessionService,
            Core::SnoreSaberRuntimeInfo* runtimeInfo,
            Core::Timing::SnoreSaberClock* clock,
            Compete::Services::CompeteSongService* songService,
            Compete::Services::CompeteDirectoryService* directoryService,
            Compete::Services::CompeteGameplayLauncher* gameplayLauncher,
            Compete::Services::CompeteGameplayControl* gameplayControl,
            Compete::Services::CompeteGameplayState* competeGameplayState,
            Replay::LiveReplayStreamingService* replayStreamingService)
    {
        INVOKE_CTOR();
        // zenject constructs this on the unity main thread
        _unityThreadId = std::this_thread::get_id();
        _gameSessionService = gameSessionService;
        _runtimeInfo = runtimeInfo;
        _clock = clock;
        _songService = songService;
        _directoryService = directoryService;
        _gameplayLauncher = gameplayLauncher;
        _gameplayControl = gameplayControl;
        _competeGameplayState = competeGameplayState;
        _replayStreamingService = replayStreamingService;

        _transport = std::make_unique<LudusSessionTransport>(&_mainThread);
        _outgoing = std::make_unique<Packets::LudusPacketSender>(
                [this](std::vector<uint8_t> bytes) { _transport->Send(std::move(bytes)); },
                [this](std::function<std::vector<uint8_t>()> bytesFactory) { return _transport->SendDeferred(std::move(bytesFactory)); },
                clock);
        _mapStartCountdown = std::make_unique<LudusMapStartCountdown>(
                &_mainThread,
                [this]() -> std::string {
                    std::lock_guard lock(_stateLock);
                    return _tournamentRoom ? _tournamentRoom->id : std::string();
                },
                clock);
        _commandSession = std::make_unique<Compete::Packets::CompeteLudusCommandSession>(
                Compete::Packets::CompeteLudusCommandSession::Delegates{
                        .localPlayerId = [this] { return GetLocalPlayerId(); },
                        .getTournamentRoom = [this]() -> std::shared_ptr<Compete::Domain::CompeteRoom> {
                            std::lock_guard lock(_stateLock);
                            return _tournamentRoom;
                        },
                        .setTournamentRoom = [this](const std::shared_ptr<Compete::Domain::CompeteRoom>& room) {
                            std::lock_guard lock(_stateLock);
                            _tournamentRoom = room;
                        },
                        .connectionCancellationToken = [this] { return ConnectionCancellationToken(); },
                        // command handlers may run on async worker threads (pc runs them on the unity
                        // sync context); marshal event raises to main so ui subscribers stay safe
                        .playerFollowRequested = [this](int viewerCount) { RaiseOnMain([this, viewerCount] { PlayerFollowRequested.Invoke(viewerCount); }); },
                        .viewersUpdated = [this](const std::optional<std::vector<V1::LiveRoomViewerState>>& viewers) { RaiseOnMain([this, viewers] { UpdateViewerList(viewers); }); },
                        .roomUpdated = [this](const std::shared_ptr<Compete::Domain::CompeteRoom>& room) {
                            RaiseOnMain([this, room] {
                                RoomUpdated.Invoke(room);
                                CompletePendingTournamentJoin(room);
                            });
                        },
                        .promptReceived = [this](const Compete::Domain::CompeteOrganizerPrompt& prompt) { RaiseOnMain([this, prompt] { PromptReceived.Invoke(prompt); }); },
                        .statusChanged = [this](const std::string& status) { RaiseOnMain([this, status] { StatusChanged.Invoke(status); }); },
                        .closeTournamentRoom = [this] { RaiseOnMain([this] { CloseTournamentRoomFromServer(); }); },
                        .sendDownloadState = [this](V1::LudusDownloadState state, const std::string& errorMessage) { SendDownloadState(state, errorMessage); },
                        .sendPresence = [this](V1::LudusPlayState playState, V1::LudusDownloadState downloadState, const std::string& currentMapHash) { SendPresence(playState, downloadState, currentMapHash); },
                        .beginMapStartCountdown = [this](const std::string& matchId, int delayMs, const CancellationToken& cancellationToken) { return _mapStartCountdown->Begin(matchId, delayMs, cancellationToken); },
                        .tryCancelPendingMapStart = [this](const std::string& matchId) { return _mapStartCountdown->TryCancel(matchId); },
                        .completePendingMapStart = [this](const std::string& matchId, const CancellationToken& countdownToken) { _mapStartCountdown->Complete(matchId, countdownToken); },
                },
                songService,
                directoryService,
                gameplayLauncher,
                gameplayControl);
        _packetContext = std::make_unique<LudusSessionPacketContext>(
                LudusSessionPacketContext::Delegates{
                        .getLastReceivedSequence = [this] { return _outgoing->lastReceivedSequence; },
                        .setLastReceivedSequence = [this](uint64_t sequence) { _outgoing->lastReceivedSequence = sequence; },
                        .getConnectionId = [this] { return ConnectionId(); },
                        .setConnectionId = [this](const std::string& connectionId) { SetConnectionId(connectionId); },
                        .getHeartbeatIntervalSeconds = [this] { return _heartbeatIntervalSeconds; },
                        .setHeartbeatIntervalSeconds = [this](float heartbeatIntervalSeconds) { _heartbeatIntervalSeconds = heartbeatIntervalSeconds; },
                        .getClientType = [this] {
                            std::lock_guard lock(_stateLock);
                            return _clientType;
                        },
                        .getRoomContext = [this] {
                            std::lock_guard lock(_stateLock);
                            return _roomContext;
                        },
                        .getCurrentLudusMatchId = [this] { return CurrentLudusMatchId(); },
                        .getPendingTournamentRoom = [this]() -> std::shared_ptr<Compete::Domain::CompeteRoom> {
                            std::lock_guard lock(_stateLock);
                            return _pendingTournamentRoom;
                        },
                        .applyClientContext = [this](const Protocol::DecodedLudusEnvelope& envelope) { ApplyClientContext(envelope); },
                        .closeTournamentRoom = [this] { CloseTournamentRoomFromServer(); },
                        .disconnect = [this] { Disconnect(); },
                        .enterTournamentRoom = [this](const std::shared_ptr<Compete::Domain::CompeteRoom>& room) { EnterTournamentRoom(room); },
                        .rejectPendingTournamentJoin = [this](const std::string& message) { RejectPendingTournamentJoin(message); },
                        .requestAuthenticationRefresh = [this] { return RequestAuthenticationRefresh(); },
                        .scheduleNextHeartbeat = [this] { _nextHeartbeatAt = UnityEngine::Time::get_realtimeSinceStartup() + _heartbeatIntervalSeconds; },
                        .scheduleReconnect = [this](const std::string& reason, std::optional<float> delayOverrideSeconds) { ScheduleReconnect(reason, delayOverrideSeconds); },
                        .sendPresence = [this](V1::LudusPlayState playState, V1::LudusDownloadState downloadState, const std::string& currentMapHash) { SendPresence(playState, downloadState, currentMapHash); },
                        .setReconnectUrl = [this](const std::string& url) { _nextLudusUrl = url; },
                        .notifyChatMessagesChanged = [this](const std::vector<Domain::LiveChatEntry>& messages) { ChatMessagesChanged.Invoke(messages); },
                        .notifyStatusChanged = [this](const std::string& status) { StatusChanged.Invoke(status); },
                });
        _packetDispatcher = std::make_unique<Packets::LudusPacketDispatcher<ILudusSessionPacketContext>>(
                Compete::Packets::CompeteLudusPacketDispatcher::CreateDefault(_commandSession.get(), &_chatMessages, clock));

        _transport->MessageReceived = [this](const std::vector<uint8_t>& bytes) { _packetDispatcher->Handle(*_packetContext, bytes); };
        _transport->ReceiveFailed = [this](std::string message) { ReceiveFailed(message); };
        _transport->SendFailed = [this](std::string message) { SendFailed(message); };
        _transport->ReconnectRequested = [this](std::string reason) { ScheduleReconnect(reason, std::nullopt); };
        _transport->Disconnected = [this] { TransportDisconnected(); };
        _mapStartCountdown->changed = [this](const std::optional<Compete::Domain::CompeteMapStartCountdown>& countdown) { MapStartCountdownChanged.Invoke(countdown); };
    }

    bool LudusSessionService::IsConnectedToLudus()
    {
        return _transport->IsOpen() && !ConnectionId().empty();
    }

    bool LudusSessionService::IsInTournamentRoom()
    {
        if (!_transport->IsOpen())
        {
            return false;
        }

        std::lock_guard lock(_stateLock);
        return !_connectionId.empty() && _roomContext == V1::LudusRoomContextType::Tournament && !_currentMatchId.empty();
    }

    bool LudusSessionService::IsInPublicPresence()
    {
        if (!_transport->IsOpen())
        {
            return false;
        }

        std::lock_guard lock(_stateLock);
        return !_connectionId.empty() && _roomContext == V1::LudusRoomContextType::PublicPresence;
    }

    std::string LudusSessionService::CurrentLudusMatchId() const
    {
        std::lock_guard lock(_stateLock);
        return _currentMatchId;
    }

    std::string LudusSessionService::LocalPlayerId() const
    {
        return GetLocalPlayerId();
    }

    std::string LudusSessionService::SnoreSaberPlayerId() const
    {
        auto session = _gameSessionService->GetGameSession();
        return session ? session->playerId : "";
    }

    std::string LudusSessionService::GameSessionId() const
    {
        auto session = _gameSessionService->GetGameSession();
        return session ? session->sessionId : "";
    }

    int LudusSessionService::LocalAuthType() const
    {
        auto info = _gameSessionService->GetLocalPlayerInfo();
        return info ? info->authType : -1;
    }

    std::vector<V1::LiveRoomViewerState> LudusSessionService::CurrentViewers() const
    {
        std::lock_guard lock(_stateLock);
        return _currentViewers;
    }

    int LudusSessionService::CurrentViewerCount() const
    {
        std::lock_guard lock(_stateLock);
        return static_cast<int>(_currentViewers.size());
    }

    std::vector<Domain::LiveChatEntry> LudusSessionService::CurrentChatMessages() const
    {
        return _chatMessages.MessagesFor(CurrentLudusMatchId());
    }

    void LudusSessionService::Initialize()
    {
        INFO("Ludus session initialized.");
        _loginStatusToken = _gameSessionService->AddLoginStatusChangedHandler([this](LoginStatus status, const std::string&) { GameSessionStatusChanged(status); });
        _replayStreamingService->AttachLudus(this);
        if (_gameSessionService->HasAuthenticatedSession() || _gameSessionService->GetStatus() == LoginStatus::Success)
        {
            EnsureSessionConnection(CancellationToken{});
        }
    }

    void LudusSessionService::Tick()
    {
        size_t actionBudget = _competeGameplayState->IsLiveGameplayActive() ? GameplayMainThreadQueueActionBudget : std::numeric_limits<size_t>::max();
        size_t remainingActions = _mainThread.Drain(actionBudget);
        if (remainingActions > 0 && _competeGameplayState->IsLiveGameplayActive() && UnityEngine::Time::get_realtimeSinceStartup() >= _nextMainThreadQueueBacklogLogAt)
        {
            _nextMainThreadQueueBacklogLogAt = UnityEngine::Time::get_realtimeSinceStartup() + MainThreadQueueBacklogLogIntervalSeconds;
            WARN("Ludus: deferred {} main-thread messages during live gameplay.", remainingActions);
        }
        ReconnectIfDue();
        SendHeartbeatIfDue();
    }

    void LudusSessionService::Dispose()
    {
        _gameSessionService->RemoveLoginStatusChangedHandler(_loginStatusToken);
        _loginStatusToken = 0;
        _replayStreamingService->AttachLudus(nullptr);
        Disconnect();
    }

    void LudusSessionService::ConnectAndJoin(const std::shared_ptr<Compete::Domain::CompeteRoom>& room, const CancellationToken& cancellationToken)
    {
        if (!room)
        {
            throw std::invalid_argument("room");
        }

        {
            std::lock_guard lock(_stateLock);
            _tournamentRoom = room;
            _pendingTournamentRoom = room;
        }
        auto pendingJoin = BeginPendingTournamentJoin(room);

        try
        {
            std::shared_ptr<ConnectOperation> operation;
            RunOnMainBlocking([&] { operation = EnsureSessionConnection(cancellationToken); });
            if (operation)
            {
                WaitWithCancellation(operation->future, cancellationToken);
                operation->future.get();
            }

            RunOnMainBlocking([&] {
                if (IsConnectedToLudus())
                {
                    EnterTournamentRoom(room);
                }
            });

            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(TournamentJoinAckTimeoutMs);
            bool acknowledged = false;
            while (std::chrono::steady_clock::now() < deadline)
            {
                cancellationToken.ThrowIfCancellationRequested();
                if (pendingJoin->future.wait_for(std::chrono::milliseconds(25)) == std::future_status::ready)
                {
                    acknowledged = true;
                    break;
                }
            }
            cancellationToken.ThrowIfCancellationRequested();
            if (acknowledged)
            {
                pendingJoin->future.get();
            }
        }
        catch (...)
        {
            ClearPendingTournamentJoin(pendingJoin);
            throw;
        }
        ClearPendingTournamentJoin(pendingJoin);

        std::shared_ptr<Compete::Domain::CompeteRoom> currentRoom;
        {
            std::lock_guard lock(_stateLock);
            currentRoom = _tournamentRoom;
        }
        EnsureTournamentRoomSongReady(currentRoom);
    }

    void LudusSessionService::Disconnect()
    {
        ResetSessionConnection(true);
    }

    void LudusSessionService::ResetSessionConnection(bool clearChatMessages)
    {
        _active = false;
        _reconnectScheduled = false;
        _nextReconnectAt = 0.0f;
        _reconnectAttempt = 0;
        {
            std::lock_guard lock(_stateLock);
            _roomContext = V1::LudusRoomContextType::Unspecified;
            _pendingTournamentRoom = nullptr;
            _tournamentRoom = nullptr;
        }
        UpdateViewerList(std::nullopt);
        if (clearChatMessages)
        {
            ClearChatMessages();
        }
        _nextLudusUrl.clear();
        _mapStartCountdown->Cancel();

        // quest addition: abort any in-flight blocking connect (pc's socket token dies with the socket)
        CancelConnectionCancellation();
        _transport->DisposeSocket();
        {
            std::lock_guard lock(_stateLock);
            _connectionId.clear();
            _clientType = V1::LudusClientType::Player;
            _currentMatchId.clear();
            _currentTournamentId.clear();
        }
        _connectOperation = nullptr;
    }

    void LudusSessionService::RestartDefaultSessionConnection()
    {
        ResetSessionConnection(false);
        EnsureSessionConnection(CancellationToken{});
    }

    void LudusSessionService::ReturnToPublicPresence()
    {
        _mapStartCountdown->Cancel();
        {
            std::lock_guard lock(_stateLock);
            _pendingTournamentRoom = nullptr;
            _tournamentRoom = nullptr;
        }
        UpdateViewerList(std::nullopt);
        _nextLudusUrl.clear();

        ApplyDefaultSessionRoomContext();
    }

    void LudusSessionService::CloseTournamentRoomFromServer()
    {
        ReturnToPublicPresence();
        RoomClosed.Invoke();
    }

    void LudusSessionService::ApplyPublicLivePresencePreference()
    {
        bool keepTournamentRoom;
        {
            std::lock_guard lock(_stateLock);
            keepTournamentRoom = _roomContext == V1::LudusRoomContextType::Tournament || _tournamentRoom || _pendingTournamentRoom;
        }
        if (keepTournamentRoom)
        {
            INFO("Ludus: Public live presence preference updated; keeping current tournament room.");
            return;
        }

        if (Data::Private::Settings::publicLivePresenceOptOut)
        {
            _replayStreamingService->StopPublicPresenceStream();
        }

        RestartDefaultSessionConnection();
    }

    void LudusSessionService::EnterTournamentRoom(const std::shared_ptr<Compete::Domain::CompeteRoom>& room)
    {
        if (!room || !IsConnectedToLudus())
        {
            return;
        }

        {
            std::lock_guard lock(_stateLock);
            _pendingTournamentRoom = nullptr;
            _tournamentRoom = room;
        }
        RequestClientType(V1::LudusClientType::Player);
        ApplyRoomContext(V1::LudusRoomContextType::Tournament, room->tournamentId, room->id);
        ChatMessagesChanged.Invoke(CurrentChatMessages());
        auto installedMods = LudusInstalledMods::List();
        std::string connectionId = ConnectionId();
        _outgoing->SetRoomContext(V1::LudusRoomContextType::Tournament, room->tournamentId, installedMods, connectionId);
        _outgoing->JoinRoom(room->id, installedMods, connectionId);
    }

    void LudusSessionService::EnsureTournamentRoomSongReady(const std::shared_ptr<Compete::Domain::CompeteRoom>& room)
    {
        if (!room || !room->song || room->song->beatmapLevel)
        {
            return;
        }

        auto song = Compete::Packets::Handlers::LoadSongCommandHandler::SongCommandFromSelection(room->song);
        if (!song)
        {
            return;
        }

        // pc RunTask fire-and-forget; the service is app-scoped, safe to capture across the worker
        Utils::Async::Run([this, song = std::move(song)] {
            try
            {
                Compete::Packets::Handlers::LoadSongCommandHandler::EnsureSongReady(*_commandSession, song);
            }
            catch (const OperationCanceledException&)
            {
            }
            catch (const std::exception& ex)
            {
                ERROR("Failed to ensure tournament room song ready: {}", ex.what());
            }
        });
    }

    std::shared_ptr<LudusSessionService::PendingTournamentJoin> LudusSessionService::BeginPendingTournamentJoin(const std::shared_ptr<Compete::Domain::CompeteRoom>& room)
    {
        auto pendingJoin = std::make_shared<PendingTournamentJoin>(room ? room->id : "");
        {
            std::lock_guard lock(_stateLock);
            _pendingTournamentJoin = pendingJoin;
        }
        return pendingJoin;
    }

    void LudusSessionService::CompletePendingTournamentJoin(const std::shared_ptr<Compete::Domain::CompeteRoom>& room)
    {
        std::shared_ptr<PendingTournamentJoin> pendingJoin;
        {
            std::lock_guard lock(_stateLock);
            pendingJoin = _pendingTournamentJoin;
        }
        if (!pendingJoin || !room || room->id != pendingJoin->roomId)
        {
            return;
        }

        pendingJoin->TrySetResult(room);
    }

    void LudusSessionService::RejectPendingTournamentJoin(const std::string& message)
    {
        std::shared_ptr<PendingTournamentJoin> pendingJoin;
        {
            std::lock_guard lock(_stateLock);
            pendingJoin = _pendingTournamentJoin;
        }
        if (!pendingJoin)
        {
            return;
        }

        pendingJoin->TrySetException(std::make_exception_ptr(std::runtime_error(message.empty() ? "That room could not be joined." : message)));
    }

    void LudusSessionService::ClearPendingTournamentJoin(const std::shared_ptr<PendingTournamentJoin>& pendingJoin)
    {
        std::lock_guard lock(_stateLock);
        if (_pendingTournamentJoin != pendingJoin)
        {
            return;
        }

        _pendingTournamentJoin = nullptr;
    }

    void LudusSessionService::ApplyRoomContext(V1::LudusRoomContextType roomContext, const std::string& tournamentId, const std::string& currentMatchId)
    {
        std::lock_guard lock(_stateLock);
        _roomContext = roomContext;
        _currentTournamentId = tournamentId;
        _currentMatchId = ResolveCurrentMatchIdLocked(roomContext, currentMatchId);
    }

    void LudusSessionService::ApplyClientContext(const Protocol::DecodedLudusEnvelope& envelope)
    {
        {
            std::lock_guard lock(_stateLock);
            _clientType = NormalizeClientType(envelope.ClientType);
        }
        ApplyRoomContext(envelope.RoomContext, envelope.TournamentId, envelope.CurrentMatchId);
        ChatMessagesChanged.Invoke(CurrentChatMessages());
    }

    bool LudusSessionService::RequestAuthenticationRefresh()
    {
        auto lastAuthenticatedAtUtc = _gameSessionService->LastAuthenticatedAtUtc();
        if (lastAuthenticatedAtUtc != std::chrono::system_clock::time_point{} && std::chrono::system_clock::now() - lastAuthenticatedAtUtc < FreshGameSessionAuthGuard)
        {
            _nextConnectionAuthRefreshAttemptAtUtc = lastAuthenticatedAtUtc + FreshGameSessionAuthGuard;
            WARN("Ludus: ignoring authentication refresh request because the game session was refreshed recently.");
            return false;
        }

        _forceAuthenticationRefreshOnNextConnect = true;
        _nextConnectionAuthRefreshAttemptAtUtc = {};
        return true;
    }

    void LudusSessionService::RaiseOnMain(std::function<void()> action)
    {
        if (std::this_thread::get_id() == _unityThreadId)
        {
            action();
            return;
        }

        _mainThread.Enqueue(std::move(action));
    }

    void LudusSessionService::UpdateViewerList(const std::optional<std::vector<V1::LiveRoomViewerState>>& viewers)
    {
        std::vector<V1::LiveRoomViewerState> list = viewers.value_or(std::vector<V1::LiveRoomViewerState>{});
        {
            std::lock_guard lock(_stateLock);
            _currentViewers = list;
        }
        ViewerListUpdated.Invoke(list);
    }

    void LudusSessionService::RequestClientType(V1::LudusClientType clientType)
    {
        if (!IsConnectedToLudus())
        {
            return;
        }

        std::string connectionId;
        {
            std::lock_guard lock(_stateLock);
            if (_clientType == clientType)
            {
                return;
            }
            connectionId = _connectionId;
        }
        _outgoing->SetClientType(clientType, connectionId);
    }

    void LudusSessionService::SetReady(bool ready)
    {
        std::shared_ptr<Compete::Domain::CompeteRoom> room;
        std::string connectionId;
        {
            std::lock_guard lock(_stateLock);
            room = _tournamentRoom;
            connectionId = _connectionId;
        }
        if (!room || connectionId.empty())
        {
            return;
        }

        _outgoing->ReadyState(room->id, ready, connectionId);
        std::vector<Compete::Domain::CompetePlayer> players;
        players.reserve(room->players.size());
        for (const auto& player : room->players)
        {
            players.push_back(player.isLocalPlayer
                                      ? Compete::Domain::CompetePlayer{player.name, ready ? "Ready" : "Waiting", player.teamId, player.rank, true, player.playerId, player.isBot, player.avatarUrl}
                                      : player);
        }
        auto updated = std::make_shared<Compete::Domain::CompeteRoom>(room->WithPlayers(std::move(players), ready));
        {
            std::lock_guard lock(_stateLock);
            _tournamentRoom = updated;
        }
        RoomUpdated.Invoke(updated);
    }

    void LudusSessionService::SendPromptResponse(const Compete::Domain::CompeteOrganizerPrompt& prompt, bool accepted)
    {
        std::shared_ptr<Compete::Domain::CompeteRoom> room;
        std::string connectionId;
        {
            std::lock_guard lock(_stateLock);
            room = _tournamentRoom;
            connectionId = _connectionId;
        }
        if (connectionId.empty())
        {
            return;
        }

        std::string matchId = prompt.matchId.empty() ? (room ? room->id : "") : prompt.matchId;
        _outgoing->PromptResponse(prompt, matchId, GetLocalPlayerId(), accepted, connectionId);
    }

    bool LudusSessionService::SendChatMessage(const std::string& text)
    {
        std::string trimmed = Trim(text);
        if (!IsConnectedToLudus() || trimmed.empty())
        {
            return false;
        }

        std::string matchId;
        std::string connectionId;
        {
            std::lock_guard lock(_stateLock);
            matchId = _currentMatchId;
            connectionId = _connectionId;
        }
        if (matchId.empty())
        {
            return false;
        }

        _outgoing->ChatMessage(matchId, trimmed, LocalPlayerDisplayName(), connectionId);
        return true;
    }

    bool LudusSessionService::TrySetLinkedSong(const std::shared_ptr<Compete::Domain::CompeteSongSelection>& song)
    {
        if (!song)
        {
            return false;
        }

        std::shared_ptr<Compete::Domain::CompeteRoom> updated;
        {
            std::lock_guard lock(_stateLock);
            if (!_tournamentRoom)
            {
                return false;
            }
            updated = std::make_shared<Compete::Domain::CompeteRoom>(_tournamentRoom->WithSong(song));
            _tournamentRoom = updated;
        }
        RoomUpdated.Invoke(updated);
        return true;
    }

    void LudusSessionService::SendPresence(V1::LudusPlayState playState, V1::LudusDownloadState downloadState, const std::string& currentMapHash)
    {
        if (!_transport->IsOpen())
        {
            return;
        }

        std::string matchId;
        std::string connectionId;
        {
            std::lock_guard lock(_stateLock);
            matchId = _currentMatchId;
            connectionId = _connectionId;
        }
        if (connectionId.empty() || matchId.empty())
        {
            return;
        }

        _outgoing->Presence(playState, downloadState, matchId, currentMapHash, connectionId);
    }

    void LudusSessionService::SendDownloadState(V1::LudusDownloadState state, const std::string& errorMessage)
    {
        std::shared_ptr<Compete::Domain::CompeteRoom> room;
        std::string connectionId;
        {
            std::lock_guard lock(_stateLock);
            room = _tournamentRoom;
            connectionId = _connectionId;
        }
        if (room)
        {
            _outgoing->DownloadState(room->id, state, errorMessage, connectionId);
        }
    }

    bool LudusSessionService::SendReplayPacket(V1::ReplayStreamPacket packet)
    {
        if (!IsConnectedToLudus())
        {
            return false;
        }

        std::string matchId;
        std::string connectionId;
        {
            std::lock_guard lock(_stateLock);
            matchId = _currentMatchId;
            connectionId = _connectionId;
        }
        if (packet.PlayerId.empty())
        {
            packet.PlayerId = GetLocalPlayerId();
        }
        if (packet.MatchId.empty())
        {
            packet.MatchId = matchId;
        }

        return _outgoing->ReplayPacket(std::move(packet), connectionId);
    }

    std::string LudusSessionService::GetLocalPlayerId() const
    {
        auto info = _gameSessionService->GetLocalPlayerInfo();
        if (info)
        {
            return info->playerId;
        }
        auto session = _gameSessionService->GetGameSession();
        return session ? session->playerId : "";
    }

    std::string LudusSessionService::PublicPresenceMatchId() const
    {
        std::string playerId = GetLocalPlayerId();
        return playerId.empty() ? "" : "player:" + playerId;
    }

    std::shared_ptr<LudusSessionService::ConnectOperation> LudusSessionService::EnsureSessionConnection(const CancellationToken& cancellationToken)
    {
        if (IsConnectedToLudus())
        {
            return nullptr;
        }

        if (_connectOperation && !_connectOperation->IsCompleted())
        {
            return _connectOperation;
        }

        _connectOperation = std::make_shared<ConnectOperation>();
        OpenSessionConnection(_connectOperation, cancellationToken);
        return _connectOperation;
    }

    void LudusSessionService::OpenSessionConnection(const std::shared_ptr<ConnectOperation>& operation, CancellationToken cancellationToken)
    {
        if (IsConnectedToLudus())
        {
            operation->Complete();
            return;
        }

        bool hadCachedSession = _gameSessionService->HasAuthenticatedSession();
        bool forceAuthenticationRefresh = ShouldRefreshAuthenticationForConnection();
        // callback lands on the main thread
        _gameSessionService->EnsureAuthenticated(forceAuthenticationRefresh, [this, operation, cancellationToken, hadCachedSession, forceAuthenticationRefresh](LoginStatus status) {
            bool authenticated = status == LoginStatus::Success;
            bool usedCachedSessionAfterRefreshFailure = false;
            if (!authenticated && hadCachedSession && _gameSessionService->HasAuthenticatedSession() && !AuthenticationRefreshIsRequired())
            {
                _nextConnectionAuthRefreshAttemptAtUtc = std::chrono::system_clock::now() + GameSessionRefreshRetryDelay;
                authenticated = true;
                usedCachedSessionAfterRefreshFailure = true;
                WARN("Ludus: Game session refresh failed; retrying cached session.");
            }
            if (!authenticated || !_gameSessionService->HasAuthenticatedSession())
            {
                // pc surfaces this through RunTask fault logging
                ERROR("SnoreSaber game session is not available");
                operation->Fail(std::make_exception_ptr(std::runtime_error("SnoreSaber game session is not available")));
                return;
            }
            MarkAuthenticationAvailable(forceAuthenticationRefresh && !usedCachedSessionAfterRefreshFailure);

            PrepareConnectionAttempt();

            std::string url = NormalizeLudusUrl(_nextLudusUrl.empty() ? std::string(Core::Api::SnoreSaberUrls::LudusUrl) : _nextLudusUrl);
            INFO("Ludus: Connecting to {}", url);

            CancellationToken connectionToken = ConnectionCancellationToken().Linked(cancellationToken);
            Utils::Async::Run([this, operation, url, connectionToken] {
                std::string error;
                bool connected = false;
                try
                {
                    connectionToken.ThrowIfCancellationRequested();
                    connected = _transport->Connect(url, ConnectTimeoutMs, error);
                    connectionToken.ThrowIfCancellationRequested();
                }
                catch (const OperationCanceledException&)
                {
                    Utils::Async::Main([this, operation] {
                        _transport->DisposeSocket();
                        operation->Fail(std::make_exception_ptr(OperationCanceledException()));
                    });
                    return;
                }

                if (!connected)
                {
                    Utils::Async::Main([this, operation, error] {
                        _transport->DisposeSocket();
                        ScheduleReconnect(error, std::nullopt);
                        WARN("Ludus connection failed: {}", error);
                        operation->Fail(std::make_exception_ptr(std::runtime_error(error)));
                    });
                    return;
                }

                Utils::Async::Main([this, operation] {
                    _reconnectAttempt = 0;
                    SendConnect();
                    _transport->StartReceiveLoop();
                    operation->Complete();
                });
            });
        });
    }

    bool LudusSessionService::ShouldRefreshAuthenticationForConnection()
    {
        if (!_gameSessionService->HasAuthenticatedSession())
        {
            _forceAuthenticationRefreshOnNextConnect = false;
            return false;
        }

        auto now = std::chrono::system_clock::now();
        if (_forceAuthenticationRefreshOnNextConnect)
        {
            return true;
        }
        if (_lastConnectionAuthRefreshAtUtc == std::chrono::system_clock::time_point{})
        {
            return false;
        }
        if (now < _nextConnectionAuthRefreshAttemptAtUtc)
        {
            return false;
        }
        return now - _lastConnectionAuthRefreshAtUtc >= GameSessionReconnectRefreshInterval;
    }

    bool LudusSessionService::AuthenticationRefreshIsRequired() const
    {
        return _forceAuthenticationRefreshOnNextConnect;
    }

    void LudusSessionService::MarkAuthenticationAvailable(bool refreshAttempted)
    {
        if (refreshAttempted || _lastConnectionAuthRefreshAtUtc == std::chrono::system_clock::time_point{})
        {
            _lastConnectionAuthRefreshAtUtc = std::chrono::system_clock::now();
            _nextConnectionAuthRefreshAttemptAtUtc = {};
        }
        if (refreshAttempted)
        {
            _forceAuthenticationRefreshOnNextConnect = false;
        }
    }

    void LudusSessionService::GameSessionStatusChanged(LoginStatus status)
    {
        if (status != LoginStatus::Success)
        {
            return;
        }

        INFO("Ludus: authenticated session available, connecting session.");
        EnsureSessionConnection(CancellationToken{});
    }

    void LudusSessionService::ReceiveFailed(const std::string& message)
    {
        StatusChanged.Invoke(fmt::format("Ludus receive failed: {}", message));
        WARN("Ludus receive failed: {}", message);
    }

    void LudusSessionService::SendFailed(const std::string& message)
    {
        StatusChanged.Invoke(fmt::format("Ludus send failed: {}", message));
        WARN("Ludus send failed: {}", message);
    }

    void LudusSessionService::TransportDisconnected()
    {
        if (!_active)
        {
            return;
        }

        StatusChanged.Invoke("Ludus disconnected");
        WARN("Ludus disconnected.");
        ScheduleReconnect("disconnected", std::nullopt);
    }

    void LudusSessionService::ReconnectIfDue()
    {
        if (!_active || !_reconnectScheduled || UnityEngine::Time::get_realtimeSinceStartup() < _nextReconnectAt)
        {
            return;
        }

        _reconnectScheduled = false;
        _nextReconnectAt = 0.0f;
        EnsureSessionConnection(CancellationToken{});
    }

    void LudusSessionService::SendHeartbeatIfDue()
    {
        if (!CanSendHeartbeat() || UnityEngine::Time::get_realtimeSinceStartup() < _nextHeartbeatAt)
        {
            return;
        }

        _outgoing->Heartbeat(ConnectionId());
        _nextHeartbeatAt = UnityEngine::Time::get_realtimeSinceStartup() + _heartbeatIntervalSeconds;
    }

    void LudusSessionService::SendConnect()
    {
        auto session = _gameSessionService->GetGameSession();
        if (!session)
        {
            return;
        }

        _outgoing->Connect(
                *session,
                LocalPlatform(),
                Core::SnoreSaberRuntimeInfo::GameVersion(),
                Core::SnoreSaberRuntimeInfo::PluginVersion(),
                DefaultSessionRoomContext(),
                Data::Private::Settings::publicLivePresenceOptOut,
                LudusInstalledMods::List());
    }

    void LudusSessionService::PrepareConnectionAttempt()
    {
        _active = true;
        _outgoing->ResetSequences();
        SetConnectionId("");
        _heartbeatIntervalSeconds = 5.0f;
        _nextHeartbeatAt = 0.0f;
        _reconnectScheduled = false;
        _nextReconnectAt = 0.0f;
        {
            std::lock_guard lock(_stateLock);
            _connectionCancellation.emplace();
        }
        _transport->Prepare();
    }

    void LudusSessionService::ScheduleReconnect(const std::string& reason, std::optional<float> delayOverrideSeconds)
    {
        if (!_active || _reconnectScheduled)
        {
            return;
        }

        PreserveTournamentRoomForReconnect();
        ResetSocketSessionContext();
        float delay = delayOverrideSeconds.value_or(std::min(ReconnectMaxDelaySeconds, ReconnectMinDelaySeconds * std::pow(2.0f, static_cast<float>(_reconnectAttempt))));
        if (!delayOverrideSeconds.has_value())
        {
            _reconnectAttempt++;
        }

        _nextReconnectAt = UnityEngine::Time::get_realtimeSinceStartup() + delay;
        _reconnectScheduled = true;
        if (reason.empty())
        {
            WARN("Ludus reconnecting in {}s", delay);
        }
        else
        {
            WARN("Ludus reconnecting in {}s: {}", delay, reason);
        }
    }

    void LudusSessionService::PreserveTournamentRoomForReconnect()
    {
        std::lock_guard lock(_stateLock);
        if (_tournamentRoom && _roomContext == V1::LudusRoomContextType::Tournament)
        {
            _pendingTournamentRoom = _tournamentRoom;
        }
    }

    void LudusSessionService::ResetSocketSessionContext()
    {
        CancelConnectionCancellation();
        _transport->DisposeSocket();
        {
            std::lock_guard lock(_stateLock);
            _connectionId.clear();
            _clientType = V1::LudusClientType::Player;
            _roomContext = V1::LudusRoomContextType::Unspecified;
        }
        UpdateViewerList(std::nullopt);
    }

    void LudusSessionService::ApplyDefaultSessionRoomContext()
    {
        if (!IsConnectedToLudus())
        {
            EnsureSessionConnection(CancellationToken{});
            return;
        }

        V1::LudusRoomContextType roomContext = DefaultSessionRoomContext();
        _outgoing->SetRoomContext(roomContext, "", LudusInstalledMods::List(), ConnectionId());
        RequestClientType(V1::LudusClientType::Player);
        ApplyRoomContext(roomContext, "", roomContext == V1::LudusRoomContextType::PublicPresence ? PublicPresenceMatchId() : "");
        ChatMessagesChanged.Invoke(CurrentChatMessages());
        if (IsInPublicPresence())
        {
            SendPresence(V1::LudusPlayState::InMenus, V1::LudusDownloadState::None, "");
        }
    }

    V1::LudusRoomContextType LudusSessionService::DefaultSessionRoomContext() const
    {
        return Data::Private::Settings::publicLivePresenceOptOut
                       ? V1::LudusRoomContextType::Core
                       : V1::LudusRoomContextType::PublicPresence;
    }

    std::string LudusSessionService::ResolveCurrentMatchIdLocked(V1::LudusRoomContextType roomContext, const std::string& currentMatchId) const
    {
        if (!currentMatchId.empty())
        {
            return currentMatchId;
        }

        if (roomContext == V1::LudusRoomContextType::PublicPresence)
        {
            return PublicPresenceMatchId();
        }

        if (roomContext == V1::LudusRoomContextType::Tournament)
        {
            return _tournamentRoom ? _tournamentRoom->id : "";
        }

        return "";
    }

    bool LudusSessionService::CanSendHeartbeat()
    {
        return _active && IsConnectedToLudus();
    }

    void LudusSessionService::ClearChatMessages()
    {
        if (_chatMessages.Clear())
        {
            ChatMessagesChanged.Invoke(CurrentChatMessages());
        }
    }

    std::string LudusSessionService::ConnectionId() const
    {
        std::lock_guard lock(_stateLock);
        return _connectionId;
    }

    void LudusSessionService::SetConnectionId(const std::string& connectionId)
    {
        std::lock_guard lock(_stateLock);
        _connectionId = connectionId;
    }

    CancellationToken LudusSessionService::ConnectionCancellationToken() const
    {
        std::lock_guard lock(_stateLock);
        return _connectionCancellation ? _connectionCancellation->Token() : CancellationToken{};
    }

    void LudusSessionService::CancelConnectionCancellation()
    {
        std::lock_guard lock(_stateLock);
        if (_connectionCancellation)
        {
            _connectionCancellation->Cancel();
        }
    }

    std::string LudusSessionService::LocalPlayerDisplayName() const
    {
        auto info = _gameSessionService->GetLocalPlayerInfo();
        std::string name = CleanDisplayName(info ? info->playerName : "");
        return name.empty() ? "Player" : name;
    }

    std::string LudusSessionService::CleanDisplayName(const std::string& value)
    {
        if (value.empty())
        {
            return "";
        }

        static const std::regex markupTagPattern("<[^>\r\n]{1,128}>");
        std::string stripped = std::regex_replace(value, markupTagPattern, "");

        std::string result;
        size_t i = 0;
        while (i < stripped.size())
        {
            while (i < stripped.size() && IsSpace(stripped[i]))
            {
                i++;
            }
            size_t start = i;
            while (i < stripped.size() && !IsSpace(stripped[i]))
            {
                i++;
            }
            if (i > start)
            {
                if (!result.empty())
                {
                    result += ' ';
                }
                result.append(stripped, start, i - start);
            }
        }
        return result;
    }

    V1::LivePlayerPlatform LudusSessionService::LocalPlatform() const
    {
        auto info = _gameSessionService->GetLocalPlayerInfo();
        if (!info)
        {
            return V1::LivePlayerPlatform::Unspecified;
        }

        // quest auth types: 2 = quest, 3 = dev; both are oculus-platform clients
        switch (info->authType)
        {
            case 2:
            case 3:
                return V1::LivePlayerPlatform::Oculus;
            default:
                return V1::LivePlayerPlatform::Unspecified;
        }
    }

    V1::LudusClientType LudusSessionService::NormalizeClientType(V1::LudusClientType clientType)
    {
        return clientType == V1::LudusClientType::Unspecified
                       ? V1::LudusClientType::Player
                       : clientType;
    }

    std::string LudusSessionService::NormalizeLudusUrl(const std::string& configuredUrl)
    {
        std::string value = Trim(configuredUrl);
        if (value.find("://") == std::string::npos)
        {
            value = "wss://" + value;
        }
        else if (StartsWithCaseInsensitive(value, "https://"))
        {
            value = "wss://" + value.substr(8);
        }
        else if (StartsWithCaseInsensitive(value, "http://"))
        {
            value = "ws://" + value.substr(7);
        }

        size_t schemeEnd = value.find("://") + 3;
        size_t pathStart = value.find('/', schemeEnd);
        if (pathStart == std::string::npos)
        {
            return value + "/v1/connect";
        }
        if (pathStart == value.size() - 1)
        {
            return value + "v1/connect";
        }
        return value;
    }
}
