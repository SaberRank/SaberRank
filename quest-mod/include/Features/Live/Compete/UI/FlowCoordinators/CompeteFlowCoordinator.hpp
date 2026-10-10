#pragma once

#include "Features/Leaderboards/Services/LeaderboardScreenSession.hpp"
#include "Features/Live/Cancellation.hpp"
#include "Features/Live/Compete/Domain/CompeteMapStartCountdown.hpp"
#include "Features/Live/Compete/Domain/CompeteOrganizerPrompt.hpp"
#include "Features/Live/Compete/Domain/CompeteRoom.hpp"
#include "Features/Live/Compete/Domain/CompeteTournament.hpp"
#include "Features/Live/Compete/Services/CompeteDirectoryService.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayState.hpp"
#include "Features/Live/Compete/UI/ViewControllers/CodeEntry/CompeteCodeEntryViewController.hpp"
#include "Features/Live/Compete/UI/ViewControllers/Entry/CompeteModeSelectionViewController.hpp"
#include "Features/Live/Compete/UI/ViewControllers/Room/Center/CompeteRoomViewController.hpp"
#include "Features/Live/Compete/UI/ViewControllers/Room/Left/CompetePlayerListViewController.hpp"
#include "Features/Live/Compete/UI/ViewControllers/Rooms/CompeteRoomListViewController.hpp"
#include "Features/Live/Compete/UI/ViewControllers/Shared/CompeteLoadingViewController.hpp"
#include "Features/Live/Ludus/Services/LudusSessionService.hpp"
#include "Features/Live/UI/ViewControllers/TournamentBrowserViewController.hpp"
#include "Utils/Event.hpp"

#include <GlobalNamespace/GameplaySetupViewController.hpp>
#include <GlobalNamespace/LevelSelectionNavigationController.hpp>
#include <GlobalNamespace/PlatformLeaderboardViewController.hpp>
#include <HMUI/FlowCoordinator.hpp>
#include <HMUI/ViewController.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::UI::FlowCoordinators, CompeteFlowCoordinator, HMUI::FlowCoordinator) {
    DECLARE_INSTANCE_FIELD_PRIVATE(Services::CompeteDirectoryService*, _directoryService);
    DECLARE_INSTANCE_FIELD_PRIVATE(Ludus::Services::LudusSessionService*, _ludusSession);
    DECLARE_INSTANCE_FIELD_PRIVATE(Services::CompeteGameplayState*, _competeGameplayState);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<ViewControllers::Entry::CompeteModeSelectionViewController>, _modeSelectionViewController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<::SnoreSaber::Features::Live::UI::ViewControllers::TournamentBrowserViewController>, _tournamentBrowserViewController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<ViewControllers::Rooms::CompeteRoomListViewController>, _roomListViewController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<ViewControllers::Room::Center::CompeteRoomViewController>, _roomViewController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<ViewControllers::Room::Left::CompetePlayerListViewController>, _playerListViewController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::GameplaySetupViewController>, _gameplaySetupViewController);
    DECLARE_INSTANCE_FIELD_PRIVATE(Leaderboards::Services::LeaderboardScreenSession*, _leaderboardSession);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::PlatformLeaderboardViewController>, _platformLeaderboardViewController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::LevelSelectionNavigationController>, _levelSelectionNavigationController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<ViewControllers::CodeEntry::CompeteCodeEntryViewController>, _codeEntryViewController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<ViewControllers::Shared::CompeteLoadingViewController>, _loadingViewController);

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::FlowCoordinator::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidDeactivate, &HMUI::FlowCoordinator::DidDeactivate, bool removedFromHierarchy, bool screenSystemDisabling);
    DECLARE_OVERRIDE_METHOD_MATCH(void, BackButtonWasPressed, &HMUI::FlowCoordinator::BackButtonWasPressed, HMUI::ViewController* topViewController);

    // zenject skips parameterized ctors on components; deps arrive via the
    // registered [Inject] method, mirroring the pc Construct
    DECLARE_INJECT_METHOD(void, Construct,
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
                          ViewControllers::Shared::CompeteLoadingViewController* loadingViewController);
    DECLARE_CTOR(ctor);

  public:
    Utils::Event<> DidFinishEvent;

  private:
    std::mutex _promptMutex;
    std::queue<Domain::CompeteOrganizerPrompt> _pendingPrompts;
    std::optional<Domain::CompeteTournament> _selectedTournament;
    std::shared_ptr<Domain::CompeteRoom> _selectedRoom;
    bool _rightPanelShowingLeaderboard = false;
    bool _promptShowing = false;
    bool _roomTransitioning = false;
    bool _loadingTransitioning = false;
    bool _tournamentBrowserEventsSubscribed = false;
    bool _roomUiPending = false;
    bool _roomUiPendingSongChanged = false;
    bool _roomClosedDuringGameplay = false;
    std::string _roomCloseStatus;
    std::string _gameplayRoomCloseStatus;
    std::shared_ptr<CancellationSource> _loadingCancellation;
    uint64_t _browserRefreshToken = 0;
    uint64_t _browserSelectedToken = 0;

    void SelectBrowser();
    void SelectJoinViaCode();
    void RefreshTournaments();
    void SelectTournament(const Domain::CompeteTournament& tournament);
    void RefreshRooms();
    void SelectRoom(const Domain::CompeteRoom& room);
    void BackToModeSelection();
    void BackToTournaments();
    void BackToRooms();
    void ToggleReady();
    void JoinViaCode(const std::string& code);
    void LoadTournaments(bool present);
    void LoadRooms(bool present, const std::string& refreshingStatus = "", const std::string& finishedStatus = "");
    void EnterRoom(const std::shared_ptr<Domain::CompeteRoom>& room, bool roomAlreadyLoaded, std::function<void(const std::string&)> failed = nullptr);
    void PresentWithLoading(HMUI::ViewController* viewController, const std::string& loadingMessage,
                            std::function<void(const CancellationToken&)> load, std::function<void()> beforeShowTarget = nullptr,
                            std::function<void()> finishedCallback = nullptr, std::function<void(const std::string&)> failedCallback = nullptr);
    void ReceivePrompt(const Domain::CompeteOrganizerPrompt& prompt);
    void DrainPromptQueue();
    void RoomTransitionFinished();
    void PromptWasAnswered(const Domain::CompeteOrganizerPrompt& prompt, bool accepted);
    void RoomWasUpdated(const std::shared_ptr<Domain::CompeteRoom>& room);
    bool CanRenderRoomUi();
    void RenderRoomUi(const Domain::CompeteRoom& room, bool songChanged);
    void ApplyPendingRoomUi();
    void LiveGameplayActiveChanged(bool active);
    void RoomWasClosed();
    void HandleRoomWasClosed(const std::string& roomCloseStatus);
    void RoomJoinFailed(const std::string& message);
    void RefreshRoomsAfterClose(const std::string& status = "");
    void LeaveRoomView(bool returnToPublicPresence);
    void ShowPlayersPanel(HMUI::ViewController::AnimationType animationType);
    void ShowLeaderboardPanel(HMUI::ViewController::AnimationType animationType);
    bool RefreshRoomLeaderboard();
    void RestoreMenuLeaderboard();
    void MapStartCountdownWasChanged(const std::optional<Domain::CompeteMapStartCountdown>& countdown);
    void LudusStatusChanged(const std::string& status);
    void ClearPrompts();
    void SubscribeTournamentBrowserEvents();
    void UnsubscribeTournamentBrowserEvents();
    bool IsTop(HMUI::ViewController* viewController);
};
