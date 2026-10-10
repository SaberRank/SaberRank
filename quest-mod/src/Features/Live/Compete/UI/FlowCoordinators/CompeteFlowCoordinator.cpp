#include "Features/Live/Compete/UI/FlowCoordinators/CompeteFlowCoordinator.hpp"

#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"
#include "Utils/AsyncUtils.hpp"
#include "logging.hpp"

#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/PlayerSettingsPanelController.hpp>
#include <System/Action.hpp>
#include <bsml/shared/Helpers/creation.hpp>
#include <custom-types/shared/delegate.hpp>

#include <fmt/core.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <thread>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::UI::FlowCoordinators, CompeteFlowCoordinator);

namespace SnoreSaber::Features::Live::Compete::UI::FlowCoordinators
{
    namespace
    {
        constexpr int LoadingTransitionDelayMs = 450;
        constexpr int CancellationPollMs = 25;

        bool EqualsIgnoreCase(const std::string& left, const std::string& right)
        {
            return left.size() == right.size() &&
                   std::equal(left.begin(), left.end(), right.begin(), [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
        }

        bool ContainsIgnoreCase(const std::string& value, std::string_view needle)
        {
            std::string lowerValue = value;
            std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), [](unsigned char c) { return std::tolower(c); });
            std::string lowerNeedle(needle);
            std::transform(lowerNeedle.begin(), lowerNeedle.end(), lowerNeedle.begin(), [](unsigned char c) { return std::tolower(c); });
            return lowerValue.find(lowerNeedle) != std::string::npos;
        }

        bool IsTournamentJoinBlockedStatus(const std::string& status)
        {
            if (status.empty())
            {
                return false;
            }

            return ContainsIgnoreCase(status, "denied mods") || ContainsIgnoreCase(status, "requires SnoreSaber to report installed mods");
        }

        bool SongChanged(const std::shared_ptr<Domain::CompeteSongSelection>& previous, const std::shared_ptr<Domain::CompeteSongSelection>& next)
        {
            if (previous == next)
            {
                return false;
            }

            if (!previous || !next)
            {
                return true;
            }

            if (!EqualsIgnoreCase(previous->mapHash, next->mapHash))
            {
                return true;
            }

            return !EqualsIgnoreCase(previous->difficulty, next->difficulty) || !EqualsIgnoreCase(previous->characteristic, next->characteristic);
        }

        void SleepWithCancellation(const CancellationToken& token, int milliseconds)
        {
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
            while (std::chrono::steady_clock::now() < deadline)
            {
                token.ThrowIfCancellationRequested();
                std::this_thread::sleep_for(std::chrono::milliseconds(CancellationPollMs));
            }
        }

        System::Action* MakeAction(std::function<void()> callback)
        {
            return custom_types::MakeDelegate<System::Action*>(classof(System::Action*), std::move(callback));
        }
    }

    void CompeteFlowCoordinator::ctor()
    {
        INVOKE_CTOR();
        // declaring a ctor shadows the base .ctor, so chain it by hand or
        // _mainScreenViewControllers stays null and presenting throws an NRE
        HMUI::FlowCoordinator::_ctor();
    }

    void CompeteFlowCoordinator::Construct(
        Services::CompeteDirectoryService* directoryService,
        Ludus::Services::LudusSessionService* ludusSession,
        Services::CompeteGameplayState* competeGameplayState,
        ViewControllers::Entry::CompeteModeSelectionViewController* modeSelectionViewController,
        ::SnoreSaber::Features::Live::UI::ViewControllers::TournamentBrowserViewController* tournamentBrowserViewController,
        ViewControllers::Rooms::CompeteRoomListViewController* roomListViewController,
        ViewControllers::Room::Center::CompeteRoomViewController* roomViewController,
        ViewControllers::Room::Left::CompetePlayerListViewController* playerListViewController,
        GlobalNamespace::GameplaySetupViewController* gameplaySetupViewController,
        Leaderboards::Services::LeaderboardScreenSession* leaderboardSession,
        GlobalNamespace::PlatformLeaderboardViewController* platformLeaderboardViewController,
        GlobalNamespace::LevelSelectionNavigationController* levelSelectionNavigationController,
        ViewControllers::CodeEntry::CompeteCodeEntryViewController* codeEntryViewController,
        ViewControllers::Shared::CompeteLoadingViewController* loadingViewController)
    {
        _directoryService = directoryService;
        _ludusSession = ludusSession;
        _competeGameplayState = competeGameplayState;
        _modeSelectionViewController = modeSelectionViewController;
        _tournamentBrowserViewController = tournamentBrowserViewController;
        _roomListViewController = roomListViewController;
        _roomViewController = roomViewController;
        _playerListViewController = playerListViewController;
        _gameplaySetupViewController = gameplaySetupViewController;
        _leaderboardSession = leaderboardSession;
        _platformLeaderboardViewController = platformLeaderboardViewController;
        _levelSelectionNavigationController = levelSelectionNavigationController;
        _codeEntryViewController = codeEntryViewController;
        _loadingViewController = loadingViewController;

        _modeSelectionViewController->BrowserSelected.Add([this] { SelectBrowser(); });
        _modeSelectionViewController->JoinViaCodeSelected.Add([this] { SelectJoinViaCode(); });
        _roomListViewController->RefreshRequested.Add([this] { RefreshRooms(); });
        _roomListViewController->RoomSelected.Add([this](const Domain::CompeteRoom& room) { SelectRoom(room); });
        _roomViewController->ReadyToggled.Add([this] { ToggleReady(); });
        _roomViewController->PlayersPanelSelected.Add([this] { ShowPlayersPanel(HMUI::ViewController::AnimationType::In); });
        _roomViewController->LeaderboardPanelSelected.Add([this] { ShowLeaderboardPanel(HMUI::ViewController::AnimationType::In); });
        _roomViewController->PromptAnswered.Add([this](const Domain::CompeteOrganizerPrompt& prompt, bool accepted) { PromptWasAnswered(prompt, accepted); });
        _codeEntryViewController->JoinRequested.Add([this](const std::string& code) { JoinViaCode(code); });

        // ludus session events can fire from command handler worker threads; keep
        // all flow state main-thread only by marshalling here
        _ludusSession->RoomUpdated.Add([this](const std::shared_ptr<Domain::CompeteRoom>& room) {
            Utils::Async::Main([this, room] { RoomWasUpdated(room); });
        });
        _ludusSession->RoomClosed.Add([this] { Utils::Async::Main([this] { RoomWasClosed(); }); });
        _ludusSession->PromptReceived.Add([this](const Domain::CompeteOrganizerPrompt& prompt) { ReceivePrompt(prompt); });
        _ludusSession->MapStartCountdownChanged.Add([this](const std::optional<Domain::CompeteMapStartCountdown>& countdown) {
            Utils::Async::Main([this, countdown] { MapStartCountdownWasChanged(countdown); });
        });
        _ludusSession->StatusChanged.Add([this](const std::string& status) {
            Utils::Async::Main([this, status] { LudusStatusChanged(status); });
        });
        // single-subscriber slot on the gameplay state; fired on the main thread
        _competeGameplayState->liveGameplayActiveChanged = [this](bool active) { LiveGameplayActiveChanged(active); };
    }

    void CompeteFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (!_tournamentBrowserViewController)
        {
            _tournamentBrowserViewController = BSML::Helpers::CreateViewController<::SnoreSaber::Features::Live::UI::ViewControllers::TournamentBrowserViewController*>();
        }

        SubscribeTournamentBrowserEvents();

        if (firstActivation)
        {
            SetTitle("SnoreSaber Compete", HMUI::ViewController::AnimationType::None);
            showBackButton = true;
            ProvideInitialViewControllers(_modeSelectionViewController.unsafePtr(), nullptr, nullptr, nullptr, nullptr);
        }
    }

    void CompeteFlowCoordinator::DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling)
    {
        UnsubscribeTournamentBrowserEvents();

        if (removedFromHierarchy)
        {
            {
                std::scoped_lock lock(_promptMutex);
                _pendingPrompts = {};
                _promptShowing = false;
            }

            _roomTransitioning = false;
            _loadingTransitioning = false;
            _selectedRoom.reset();
            _rightPanelShowingLeaderboard = false;
            if (_roomViewController)
            {
                _roomViewController->HideMapStartCountdown();
            }

            if (_loadingCancellation)
            {
                _loadingCancellation->Cancel();
            }

            RestoreMenuLeaderboard();
            _ludusSession->ReturnToPublicPresence();
            SetLeftScreenViewController(nullptr, HMUI::ViewController::AnimationType::None);
            SetRightScreenViewController(nullptr, HMUI::ViewController::AnimationType::None);
        }
    }

    void CompeteFlowCoordinator::BackButtonWasPressed(HMUI::ViewController* topViewController)
    {
        if (_loadingTransitioning)
        {
            return;
        }

        if (topViewController == _roomViewController.unsafePtr())
        {
            BackToRooms();
            return;
        }

        if (topViewController == _roomListViewController.unsafePtr())
        {
            BackToTournaments();
            return;
        }

        if (topViewController == _tournamentBrowserViewController.unsafePtr() || topViewController == _codeEntryViewController.unsafePtr())
        {
            BackToModeSelection();
            return;
        }

        DidFinishEvent.Invoke();
    }

    void CompeteFlowCoordinator::SelectBrowser()
    {
        LoadTournaments(true);
    }

    void CompeteFlowCoordinator::SelectJoinViaCode()
    {
        _codeEntryViewController->Reset();
        PresentViewController(_codeEntryViewController.unsafePtr(), nullptr, HMUI::ViewController::AnimationDirection::Horizontal, false);
    }

    void CompeteFlowCoordinator::RefreshTournaments()
    {
        LoadTournaments(false);
    }

    void CompeteFlowCoordinator::SelectTournament(const Domain::CompeteTournament& tournament)
    {
        INFO("Live tournament selected: {}", tournament.id);
        _selectedTournament = tournament;
        _roomListViewController->SetTournament(tournament);
        LoadRooms(true);
    }

    void CompeteFlowCoordinator::RefreshRooms()
    {
        if (!_selectedTournament)
        {
            return;
        }

        LoadRooms(false);
    }

    void CompeteFlowCoordinator::SelectRoom(const Domain::CompeteRoom& room)
    {
        if (_roomTransitioning || _loadingTransitioning)
        {
            return;
        }

        EnterRoom(std::make_shared<Domain::CompeteRoom>(room), false, [this](const std::string& message) { RoomJoinFailed(message); });
    }

    void CompeteFlowCoordinator::BackToModeSelection()
    {
        auto top = topViewController.unsafePtr();
        if (top == _tournamentBrowserViewController.unsafePtr() || top == _codeEntryViewController.unsafePtr())
        {
            _selectedTournament.reset();
            DismissViewController(top, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);
        }
    }

    void CompeteFlowCoordinator::BackToTournaments()
    {
        if (IsTop(_roomListViewController.unsafePtr()))
        {
            _selectedTournament.reset();
            DismissViewController(_roomListViewController.unsafePtr(), HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);
        }
    }

    void CompeteFlowCoordinator::BackToRooms()
    {
        if (IsTop(_roomViewController.unsafePtr()))
        {
            LeaveRoomView(true);
        }
    }

    void CompeteFlowCoordinator::ToggleReady()
    {
        if (!_selectedRoom)
        {
            return;
        }

        _ludusSession->SetReady(!_selectedRoom->localPlayerReady);
    }

    void CompeteFlowCoordinator::JoinViaCode(const std::string& code)
    {
        _codeEntryViewController->SetStatus("Looking up room...");
        Utils::Async::Run([this, code] {
            try
            {
                auto room = std::make_shared<Domain::CompeteRoom>(_directoryService->GetRoomByInviteCode(code, CancellationToken()));
                Utils::Async::Main([this, room] {
                    _selectedTournament.reset();
                    _codeEntryViewController->SetStatus("");
                    EnterRoom(room, true, [this](const std::string&) { _codeEntryViewController->SetStatus("That room could not be joined."); });
                });
            }
            catch (const std::exception& ex)
            {
                WARN("Failed to join live room by code: {}", ex.what());
                Utils::Async::Main([this] { _codeEntryViewController->SetStatus("No room was found for that code."); });
            }
        });
    }

    void CompeteFlowCoordinator::LoadTournaments(bool present)
    {
        if (!_tournamentBrowserViewController)
        {
            _tournamentBrowserViewController = BSML::Helpers::CreateViewController<::SnoreSaber::Features::Live::UI::ViewControllers::TournamentBrowserViewController*>();
            SubscribeTournamentBrowserEvents();
        }

        if (present)
        {
            PresentWithLoading(_tournamentBrowserViewController.unsafePtr(), "Loading tournaments...", [this](const CancellationToken& token) {
                auto tournaments = _directoryService->GetActiveTournaments(token);
                Utils::Async::Main([this, tournaments = std::move(tournaments)] { _tournamentBrowserViewController->SetTournaments(tournaments); });
            });
            return;
        }

        Utils::Async::Run([this] {
            try
            {
                auto tournaments = _directoryService->GetActiveTournaments(CancellationToken());
                Utils::Async::Main([this, tournaments = std::move(tournaments)] { _tournamentBrowserViewController->SetTournaments(tournaments); });
            }
            catch (const std::exception& ex)
            {
                WARN("Failed to refresh live tournaments: {}", ex.what());
            }
        });
    }

    void CompeteFlowCoordinator::LoadRooms(bool present, const std::string& refreshingStatus, const std::string& finishedStatus)
    {
        if (!_selectedTournament)
        {
            return;
        }

        std::string tournamentId = _selectedTournament->id;
        if (present)
        {
            PresentWithLoading(_roomListViewController.unsafePtr(), "Loading rooms...", [this, tournamentId](const CancellationToken& token) {
                auto rooms = _directoryService->GetJoinableRooms(tournamentId, token);
                Utils::Async::Main([this, rooms = std::move(rooms)] { _roomListViewController->SetRooms(rooms); });
            });
            return;
        }

        _roomListViewController->SetStatus(refreshingStatus);
        _roomListViewController->SetRefreshing(true);
        Utils::Async::Run([this, tournamentId, finishedStatus] {
            try
            {
                auto rooms = _directoryService->GetJoinableRooms(tournamentId, CancellationToken());
                Utils::Async::Main([this, rooms = std::move(rooms), finishedStatus] {
                    _roomListViewController->SetRooms(rooms);
                    _roomListViewController->SetStatus(finishedStatus);
                });
            }
            catch (const std::exception& ex)
            {
                WARN("Failed to refresh live rooms: {}", ex.what());
                Utils::Async::Main([this] { _roomListViewController->SetStatus("Couldn't refresh rooms."); });
            }

            Utils::Async::Main([this] { _roomListViewController->SetRefreshing(false); });
        });
    }

    void CompeteFlowCoordinator::EnterRoom(const std::shared_ptr<Domain::CompeteRoom>& room, bool roomAlreadyLoaded, std::function<void(const std::string&)> failed)
    {
        _roomTransitioning = true;
        PresentWithLoading(
            _roomViewController.unsafePtr(), "Joining room...",
            [this, room, roomAlreadyLoaded](const CancellationToken& token) {
                _selectedRoom = roomAlreadyLoaded ? room : std::make_shared<Domain::CompeteRoom>(_directoryService->GetRoom(room->tournamentId, room->id, token));
                _ludusSession->ConnectAndJoin(_selectedRoom, token);
            },
            [this] {
                if (_selectedRoom)
                {
                    _roomViewController->SetRoom(*_selectedRoom);
                    _playerListViewController->SetRoom(*_selectedRoom);
                }

                _gameplaySetupViewController->Setup(false, true, true, false,
                                                    GlobalNamespace::PlayerSettingsPanelController::PlayerSettingsPanelLayout::Singleplayer);
                SetLeftScreenViewController(_gameplaySetupViewController.unsafePtr(), HMUI::ViewController::AnimationType::In);
                ShowPlayersPanel(HMUI::ViewController::AnimationType::In);
            },
            [this] { RoomTransitionFinished(); }, std::move(failed));
    }

    void CompeteFlowCoordinator::PresentWithLoading(HMUI::ViewController* viewController, const std::string& loadingMessage,
                                                    std::function<void(const CancellationToken&)> load, std::function<void()> beforeShowTarget,
                                                    std::function<void()> finishedCallback, std::function<void(const std::string&)> failedCallback)
    {
        if (_loadingTransitioning)
        {
            WARN("Live compete present skipped; a loading transition is still pending");
            return;
        }

        if (_loadingCancellation)
        {
            _loadingCancellation->Cancel();
        }

        _loadingCancellation = std::make_shared<CancellationSource>();
        CancellationToken token = _loadingCancellation->Token();
        _loadingTransitioning = true;
        _loadingViewController->SetMessage(loadingMessage);
        PresentViewController(_loadingViewController.unsafePtr(), nullptr, HMUI::ViewController::AnimationDirection::Horizontal, false);

        Utils::Async::Run([this, viewController, load = std::move(load), beforeShowTarget = std::move(beforeShowTarget),
                           finishedCallback = std::move(finishedCallback), failedCallback = std::move(failedCallback), token] {
            try
            {
                SleepWithCancellation(token, LoadingTransitionDelayMs);
                load(token);
                Utils::Async::Main([this, viewController, beforeShowTarget, finishedCallback, token] {
                    if (token.IsCancellationRequested())
                    {
                        _loadingTransitioning = false;
                        _roomTransitioning = false;
                        return;
                    }

                    if (beforeShowTarget)
                    {
                        beforeShowTarget();
                    }

                    auto finished = MakeAction([this, finishedCallback] {
                        _loadingTransitioning = false;
                        if (finishedCallback)
                        {
                            finishedCallback();
                        }
                    });
                    ReplaceTopViewController(viewController, finished, HMUI::ViewController::AnimationType::In,
                                             HMUI::ViewController::AnimationDirection::Horizontal);
                });
            }
            catch (const OperationCanceledException&)
            {
                Utils::Async::Main([this] {
                    _loadingTransitioning = false;
                    _roomTransitioning = false;
                });
            }
            catch (const std::exception& ex)
            {
                std::string message = ex.what();
                WARN("Live compete load failed: {}", message);
                if (IsTournamentJoinBlockedStatus(message))
                {
                    Utils::Async::Main([this, message] { _loadingViewController->SetMessage(message, false); });
                    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                }

                Utils::Async::Main([this, message, failedCallback] {
                    _loadingTransitioning = false;
                    _roomTransitioning = false;
                    if (failedCallback)
                    {
                        failedCallback(message);
                    }

                    if (IsTop(_loadingViewController.unsafePtr()))
                    {
                        DismissViewController(_loadingViewController.unsafePtr(), HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);
                    }
                });
            }
        });
    }

    void CompeteFlowCoordinator::ReceivePrompt(const Domain::CompeteOrganizerPrompt& prompt)
    {
        {
            std::scoped_lock lock(_promptMutex);
            _pendingPrompts.push(prompt);
        }

        Utils::Async::Main([this] { DrainPromptQueue(); });
    }

    void CompeteFlowCoordinator::DrainPromptQueue()
    {
        if (_competeGameplayState->IsLiveGameplayActive() || !IsTop(_roomViewController.unsafePtr()) || !_roomViewController->ReadyForPrompt() ||
            _roomTransitioning || _promptShowing)
        {
            return;
        }

        std::optional<Domain::CompeteOrganizerPrompt> prompt;
        {
            std::scoped_lock lock(_promptMutex);
            if (_pendingPrompts.empty())
            {
                return;
            }

            prompt = _pendingPrompts.front();
            _pendingPrompts.pop();
            _promptShowing = true;
        }

        _roomViewController->ShowPrompt(*prompt);
    }

    void CompeteFlowCoordinator::RoomTransitionFinished()
    {
        _roomTransitioning = false;
        DrainPromptQueue();
    }

    void CompeteFlowCoordinator::PromptWasAnswered(const Domain::CompeteOrganizerPrompt& prompt, bool accepted)
    {
        _ludusSession->SendPromptResponse(prompt, accepted);
        if (accepted)
        {
            INFO("Live tournament prompt answered: accepted");
        }
        else
        {
            INFO("Live tournament prompt answered: declined");
        }

        {
            std::scoped_lock lock(_promptMutex);
            _promptShowing = false;
        }

        DrainPromptQueue();
    }

    void CompeteFlowCoordinator::RoomWasUpdated(const std::shared_ptr<Domain::CompeteRoom>& room)
    {
        if (!_selectedRoom || !room || room->id != _selectedRoom->id)
        {
            return;
        }

        bool songChanged = SongChanged(_selectedRoom->song, room->song);
        _selectedRoom = room;
        if (!CanRenderRoomUi())
        {
            _roomUiPending = true;
            _roomUiPendingSongChanged |= songChanged;
            return;
        }

        RenderRoomUi(*room, songChanged);
    }

    bool CompeteFlowCoordinator::CanRenderRoomUi()
    {
        return !_competeGameplayState->IsLiveGameplayActive() && IsTop(_roomViewController.unsafePtr());
    }

    void CompeteFlowCoordinator::RenderRoomUi(const Domain::CompeteRoom& room, bool songChanged)
    {
        _roomViewController->SetRoom(room);
        _playerListViewController->SetRoom(room);
        if (_rightPanelShowingLeaderboard && songChanged && !RefreshRoomLeaderboard())
        {
            ShowPlayersPanel(HMUI::ViewController::AnimationType::In);
        }

        _roomUiPending = false;
        _roomUiPendingSongChanged = false;
    }

    void CompeteFlowCoordinator::ApplyPendingRoomUi()
    {
        if (!_roomUiPending || !_selectedRoom || !CanRenderRoomUi())
        {
            return;
        }

        RenderRoomUi(*_selectedRoom, _roomUiPendingSongChanged);
    }

    void CompeteFlowCoordinator::LiveGameplayActiveChanged(bool active)
    {
        if (active)
        {
            return;
        }

        if (_roomClosedDuringGameplay)
        {
            _roomClosedDuringGameplay = false;
            std::string status = _gameplayRoomCloseStatus;
            _gameplayRoomCloseStatus.clear();
            HandleRoomWasClosed(status);
            return;
        }

        ApplyPendingRoomUi();
        DrainPromptQueue();
    }

    void CompeteFlowCoordinator::RoomWasClosed()
    {
        std::string roomCloseStatus = _roomCloseStatus;
        _roomCloseStatus.clear();
        if (_competeGameplayState->IsLiveGameplayActive())
        {
            _roomClosedDuringGameplay = true;
            _gameplayRoomCloseStatus = roomCloseStatus;
            return;
        }

        HandleRoomWasClosed(roomCloseStatus);
    }

    void CompeteFlowCoordinator::HandleRoomWasClosed(const std::string& roomCloseStatus)
    {
        if (_roomTransitioning && IsTop(_loadingViewController.unsafePtr()) && IsTournamentJoinBlockedStatus(roomCloseStatus))
        {
            return;
        }

        if (IsTop(_roomViewController.unsafePtr()))
        {
            LeaveRoomView(false);
            RefreshRoomsAfterClose(roomCloseStatus);
            return;
        }

        if (_roomTransitioning && IsTop(_loadingViewController.unsafePtr()))
        {
            if (_loadingCancellation)
            {
                _loadingCancellation->Cancel();
            }

            _selectedRoom.reset();
            _loadingTransitioning = false;
            _roomTransitioning = false;
            DismissViewController(_loadingViewController.unsafePtr(), HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);
            if (!_selectedTournament)
            {
                _codeEntryViewController->SetStatus(roomCloseStatus.empty() ? "That room could not be joined." : roomCloseStatus);
            }
            else
            {
                RefreshRoomsAfterClose(roomCloseStatus);
            }
        }
    }

    void CompeteFlowCoordinator::RoomJoinFailed(const std::string& message)
    {
        WARN("Failed to join live room: {}", message);
        std::string status = IsTournamentJoinBlockedStatus(message) ? message : "That room could not be joined.";
        LoadRooms(false, fmt::format("{} Refreshing rooms...", status), status);
    }

    void CompeteFlowCoordinator::RefreshRoomsAfterClose(const std::string& status)
    {
        if (!_selectedTournament)
        {
            _codeEntryViewController->SetStatus(status.empty() ? "Room closed." : status);
            return;
        }

        LoadRooms(false, status.empty() ? "Room closed. Refreshing rooms..." : status, status);
    }

    void CompeteFlowCoordinator::LeaveRoomView(bool returnToPublicPresence)
    {
        ClearPrompts();
        _selectedRoom.reset();
        _roomUiPending = false;
        _roomUiPendingSongChanged = false;
        _rightPanelShowingLeaderboard = false;
        _roomViewController->HideMapStartCountdown();
        RestoreMenuLeaderboard();
        if (returnToPublicPresence)
        {
            _ludusSession->ReturnToPublicPresence();
        }

        SetLeftScreenViewController(nullptr, HMUI::ViewController::AnimationType::Out);
        SetRightScreenViewController(nullptr, HMUI::ViewController::AnimationType::Out);
        _roomTransitioning = true;
        DismissViewController(_roomViewController.unsafePtr(), HMUI::ViewController::AnimationDirection::Horizontal,
                              MakeAction([this] { RoomTransitionFinished(); }), false);
    }

    void CompeteFlowCoordinator::ShowPlayersPanel(HMUI::ViewController::AnimationType animationType)
    {
        _rightPanelShowingLeaderboard = false;
        SetRightScreenViewController(_playerListViewController.unsafePtr(), animationType);
    }

    void CompeteFlowCoordinator::ShowLeaderboardPanel(HMUI::ViewController::AnimationType animationType)
    {
        _rightPanelShowingLeaderboard = true;
        if (!RefreshRoomLeaderboard())
        {
            ShowPlayersPanel(animationType);
            return;
        }

        SetRightScreenViewController(_platformLeaderboardViewController.unsafePtr(), animationType);
    }

    bool CompeteFlowCoordinator::RefreshRoomLeaderboard()
    {
        auto song = _selectedRoom ? _selectedRoom->song : nullptr;
        if (!song)
        {
            return false;
        }

        GlobalNamespace::BeatmapKey beatmapKey = *song->beatmapKey;
        if (!beatmapKey.levelId || !::SnoreSaber::Utils::SnoreSaberBeatmapKey::IsSupported(beatmapKey))
        {
            return false;
        }

        _platformLeaderboardViewController->SetData(byref(beatmapKey));
        return true;
    }

    void CompeteFlowCoordinator::RestoreMenuLeaderboard()
    {
        if (_levelSelectionNavigationController)
        {
            GlobalNamespace::BeatmapKey beatmapKey = _levelSelectionNavigationController->beatmapKey;
            if (beatmapKey.levelId && ::SnoreSaber::Utils::SnoreSaberBeatmapKey::IsSupported(beatmapKey))
            {
                _platformLeaderboardViewController->SetData(byref(beatmapKey));
                return;
            }
        }

        _leaderboardSession->ClearBeatmap();
    }

    void CompeteFlowCoordinator::MapStartCountdownWasChanged(const std::optional<Domain::CompeteMapStartCountdown>& countdown)
    {
        if (_competeGameplayState->IsLiveGameplayActive())
        {
            return;
        }

        if (!countdown)
        {
            _roomViewController->HideMapStartCountdown();
            return;
        }

        if (!_selectedRoom || countdown->matchId != _selectedRoom->id || !IsTop(_roomViewController.unsafePtr()))
        {
            return;
        }

        _roomViewController->ShowMapStartCountdown(*countdown);
    }

    void CompeteFlowCoordinator::LudusStatusChanged(const std::string& status)
    {
        if (IsTournamentJoinBlockedStatus(status))
        {
            _roomCloseStatus = status;
        }

        INFO("Ludus: {}", status);
    }

    void CompeteFlowCoordinator::ClearPrompts()
    {
        _roomViewController->ClearPrompt();

        std::scoped_lock lock(_promptMutex);
        _pendingPrompts = {};
        _promptShowing = false;
    }

    void CompeteFlowCoordinator::SubscribeTournamentBrowserEvents()
    {
        if (_tournamentBrowserEventsSubscribed)
        {
            return;
        }

        if (!_tournamentBrowserViewController)
        {
            WARN("Live compete tournament browser is unavailable.");
            return;
        }

        _browserRefreshToken = _tournamentBrowserViewController->RefreshRequested.Add([this] { RefreshTournaments(); });
        _browserSelectedToken = _tournamentBrowserViewController->TournamentSelected.Add(
            [this](const Domain::CompeteTournament& tournament) { SelectTournament(tournament); });
        _tournamentBrowserEventsSubscribed = true;
    }

    void CompeteFlowCoordinator::UnsubscribeTournamentBrowserEvents()
    {
        if (!_tournamentBrowserEventsSubscribed)
        {
            return;
        }

        if (!_tournamentBrowserViewController)
        {
            _browserRefreshToken = 0;
            _browserSelectedToken = 0;
            _tournamentBrowserEventsSubscribed = false;
            return;
        }

        _tournamentBrowserViewController->RefreshRequested.Remove(_browserRefreshToken);
        _tournamentBrowserViewController->TournamentSelected.Remove(_browserSelectedToken);
        _browserRefreshToken = 0;
        _browserSelectedToken = 0;
        _tournamentBrowserEventsSubscribed = false;
    }

    bool CompeteFlowCoordinator::IsTop(HMUI::ViewController* viewController)
    {
        return topViewController.unsafePtr() == viewController;
    }
}
