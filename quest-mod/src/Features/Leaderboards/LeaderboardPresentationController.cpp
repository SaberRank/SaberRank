#include "Features/Leaderboards/LeaderboardPresentationController.hpp"

#include "Features/Players/Services/PlayerService.hpp"
#include "Utils/AsyncUtils.hpp"

#include <utility>
#include <fmt/format.h>

DEFINE_TYPE(SnoreSaber::Features::Leaderboards, LeaderboardPresentationController);

namespace SnoreSaber::Features::Leaderboards
{
    void LeaderboardPresentationController::ctor(SnoreSaber::Features::Players::Services::LocalPlayerPanelSession* localPlayerPanelSession,
                                                 SnoreSaber::Features::Leaderboards::Services::LeaderboardTweeningService* leaderboardTweeningService)
    {
        INVOKE_CTOR();
        _localPlayerPanelSession = localPlayerPanelSession;
        _leaderboardTweeningService = leaderboardTweeningService;
        _localPlayerPanelSession->SetStateChangedCallback([this](SnoreSaber::Data::LocalPlayerPanelState state) {
            ApplyLocalPlayerPanelState(std::move(state));
        });
    }

    void LeaderboardPresentationController::Reset()
    {
        _platformLeaderboardViewController = nullptr;
        _tableView = nullptr;
        _loadingControl = nullptr;
        _pageUpButton = nullptr;
        _pageDownButton = nullptr;
        _banner = nullptr;
        _scoreInfoButtonHandler = nullptr;

        if (_avatarCancellation)
        {
            _avatarCancellation->Cancel();
            _avatarCancellation->Dispose();
            _avatarCancellation = nullptr;
        }

        _avatarHost.Reset();
    }

    void LeaderboardPresentationController::BindControls(GlobalNamespace::PlatformLeaderboardViewController* platformLeaderboardViewController,
                                                         SnoreSaber::UI::Other::Banner* banner,
                                                         SnoreSaber::CustomTypes::Components::LeaderboardScoreInfoButtonHandler* scoreInfoButtonHandler,
                                                         GlobalNamespace::LeaderboardTableView* tableView,
                                                         GlobalNamespace::LoadingControl* loadingControl,
                                                         UnityEngine::UI::Button* pageUpButton,
                                                         UnityEngine::UI::Button* pageDownButton)
    {
        _platformLeaderboardViewController = platformLeaderboardViewController;
        _banner = banner;
        _scoreInfoButtonHandler = scoreInfoButtonHandler;
        _tableView = tableView;
        _loadingControl = loadingControl;
        _pageUpButton = pageUpButton;
        _pageDownButton = pageDownButton;

        auto currentPanelState = _localPlayerPanelSession->CurrentState();
        auto initialPanelState = SnoreSaber::Data::LocalPlayerPanelState::Initial();
        if (_banner &&
            (!currentPanelState.isLoaded ||
             currentPanelState.player.has_value() ||
             !currentPanelState.promptErrorText.empty() ||
             currentPanelState.globalRankingText != initialPanelState.globalRankingText))
        {
            ApplyLocalPlayerPanelState(std::move(currentPanelState));
        }
    }

    void LeaderboardPresentationController::AddAvatar(SnoreSaber::UI::Other::ProfilePictureView avatar)
    {
        _avatarHost.AddAvatar(avatar);
    }

    void LeaderboardPresentationController::ClearAvatars()
    {
        _avatarHost.ClearAvatars();
    }

    void LeaderboardPresentationController::ApplyState(const SnoreSaber::Data::LeaderboardScreenState& state)
    {
        if (!_loadingControl)
        {
            return;
        }

        ApplyPageControls(state);

        if (_leaderboardTweeningService)
        {
            _leaderboardTweeningService->ClearTweensByPrefix("avatar");
        }

        if (state.status == Data::LeaderboardScreenStatus::Loading)
        {
            _loadingControl->ShowLoading("");
            ResetAvatarCancellation();
            ClearAvatars();
            return;
        }

        if (_banner)
        {
            _banner->DismissLoadingPrompt();
        }

        if (state.status == Data::LeaderboardScreenStatus::Loaded && state.leaderboard.has_value() && state.leaderboard->leaderboard.has_value() && state.leaderboard->leaderboardMap.has_value())
        {
            auto& internalLeaderboard = state.leaderboard.value();
            auto& leaderboardDetails = internalLeaderboard.leaderboardMap->leaderboardInfo.leaderboard;
            SetRankedStatus(leaderboardDetails);
            if (_tableView)
            {
                _tableView->SetScores(internalLeaderboard.leaderboardItems.getPtr(), state.playerScoreIndex);
            }
            if (!_avatarCancellation)
            {
                ResetAvatarCancellation();
            }
            _avatarHost.LoadAvatars(internalLeaderboard, _avatarCancellation->Token);
            _loadingControl->ShowText("", false);
            _loadingControl->Hide();
            if (_scoreInfoButtonHandler)
            {
                _scoreInfoButtonHandler->set_scoreCollection(internalLeaderboard.leaderboardMap.value().scores);
            }
            return;
        }

        if (state.leaderboard.has_value() && state.leaderboard->leaderboardMap.has_value())
        {
            SetRankedStatus(state.leaderboard->leaderboardMap->leaderboardInfo.leaderboard);
        }
        else if (state.status == Data::LeaderboardScreenStatus::NoLeaderboard && _banner)
        {
            _banner->set_status(state.rankedStatus, 0);
        }
        SetErrorState(state.errorText, state.showRefreshButton);
        ClearAvatars();
    }

    void LeaderboardPresentationController::SetRankedStatus(const SnoreSaber::Data::LeaderboardDetails& leaderboardInfo)
    {
        if (!_banner)
        {
            return;
        }

        if (leaderboardInfo.status == Data::LeaderboardStatus::Ranked)
        {
            if (leaderboardInfo.positiveModifiers)
            {
                _banner->set_status(fmt::format("Ranked (DA = +0.02, GN +0.04)\nStars: {:.2f}", leaderboardInfo.stars), leaderboardInfo.id);
            }
            else
            {
                _banner->set_status(fmt::format("Ranked (modifiers disabled)\nStars: {:.2f}", leaderboardInfo.stars), leaderboardInfo.id);
            }
            return;
        }

        if (leaderboardInfo.status == Data::LeaderboardStatus::Qualified)
        {
            _banner->set_status("Qualified", leaderboardInfo.id);
            return;
        }

        if (leaderboardInfo.status == Data::LeaderboardStatus::Loved)
        {
            _banner->set_status("Loved", leaderboardInfo.id);
            return;
        }
        _banner->set_status("Unranked", leaderboardInfo.id);
    }

    void LeaderboardPresentationController::SetErrorState(const std::string& errorText, bool showRefreshButton)
    {
        if (!_loadingControl)
        {
            return;
        }

        _loadingControl->Hide();
        _loadingControl->ShowText(errorText, showRefreshButton);
    }

    void LeaderboardPresentationController::SetUploadState(bool state, bool success, const std::string& errorMessage)
    {
        SnoreSaber::Utils::Async::Main([this, state, success, errorMessage]() {
            if (!_platformLeaderboardViewController || !_banner)
            {
                return;
            }

            if (state)
            {
                _platformLeaderboardViewController->_loadingControl->ShowLoading("");
                _banner->set_loading(true);
                _banner->set_prompt("Uploading snore...", -1, true);
                ClearAvatars();
                return;
            }

            _platformLeaderboardViewController->Refresh(true, true);

            if (success)
            {
                _banner->set_prompt("<color=#89fc81>Snore uploaded successfully</color>", 5);
                SnoreSaber::Services::PlayerService::UpdatePlayerInfo();
            }
            else
            {
                _banner->set_prompt(errorMessage, 5);
            }
        });
    }

    void LeaderboardPresentationController::SetPrompt(const std::string& text, int dismissTime)
    {
        if (!_banner)
        {
            return;
        }

        _banner->set_prompt(text, dismissTime);
    }

    void LeaderboardPresentationController::ApplyLocalPlayerPanelState(SnoreSaber::Data::LocalPlayerPanelState state)
    {
        SnoreSaber::Utils::Async::Main([this, state = std::move(state)] {
            if (!_banner)
            {
                return;
            }

            _banner->set_loading(!state.isLoaded);
            if (!state.isLoaded)
            {
                return;
            }

            if (state.player.has_value())
            {
                _banner->set_special_panel(state.usesWilliumsPanel, state.usesDenyahPanel);
                _banner->set_ranking(state.player->rank, state.player->pp);
                return;
            }

            _banner->set_loading(false);
            _banner->set_special_panel(false, false);
            if (!state.promptErrorText.empty())
            {
                _banner->Prompt(state.promptErrorText, false, state.promptDismissTime, nullptr);
            }
            else if (!state.globalRankingText.empty())
            {
                _banner->set_prompt(state.globalRankingText, 5);
            }
        });
    }

    void LeaderboardPresentationController::ApplyPageControls(const SnoreSaber::Data::LeaderboardScreenState& state)
    {
        if (_pageUpButton)
        {
            _pageUpButton->interactable = state.canPageUp;
        }
        if (_pageDownButton)
        {
            _pageDownButton->interactable = state.canPageDown;
        }
    }

    void LeaderboardPresentationController::ResetAvatarCancellation()
    {
        if (_avatarCancellation)
        {
            _avatarCancellation->Cancel();
            _avatarCancellation->Dispose();
        }
        _avatarCancellation = System::Threading::CancellationTokenSource::New_ctor();
    }
}
