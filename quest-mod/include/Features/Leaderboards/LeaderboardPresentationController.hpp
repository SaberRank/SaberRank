#pragma once

#include "Features/Leaderboards/Domain/LeaderboardDetails.hpp"
#include "Features/Leaderboards/Domain/LeaderboardScreenState.hpp"
#include "Features/Leaderboards/UI/Avatars/LeaderboardAvatarHost.hpp"
#include "Features/Leaderboards/UI/Panel/Banner.hpp"
#include "Features/Leaderboards/UI/ScoreDetails/LeaderboardScoreInfoButtonHandler.hpp"
#include "Features/Leaderboards/Services/LeaderboardTweeningService.hpp"
#include "Features/Players/Domain/LocalPlayerPanelState.hpp"
#include "Features/Players/Profile/ProfilePictureView.hpp"
#include "Features/Players/Services/LocalPlayerPanelSession.hpp"

#include <GlobalNamespace/LeaderboardTableView.hpp>
#include <GlobalNamespace/LoadingControl.hpp>
#include <GlobalNamespace/PlatformLeaderboardViewController.hpp>
#include <System/Threading/CancellationTokenSource.hpp>
#include <UnityEngine/UI/Button.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Leaderboards, LeaderboardPresentationController, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::PlatformLeaderboardViewController*, _platformLeaderboardViewController);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::LeaderboardTableView*, _tableView);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::LoadingControl*, _loadingControl);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityEngine::UI::Button*, _pageUpButton);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityEngine::UI::Button*, _pageDownButton);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::UI::Other::Banner*, _banner);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::CustomTypes::Components::LeaderboardScoreInfoButtonHandler*, _scoreInfoButtonHandler);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Players::Services::LocalPlayerPanelSession*, _localPlayerPanelSession);
    DECLARE_INSTANCE_FIELD_PRIVATE(System::Threading::CancellationTokenSource*, _avatarCancellation);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Leaderboards::Services::LeaderboardTweeningService*, _leaderboardTweeningService);
    DECLARE_CTOR(ctor, SnoreSaber::Features::Players::Services::LocalPlayerPanelSession* localPlayerPanelSession,
                 SnoreSaber::Features::Leaderboards::Services::LeaderboardTweeningService* leaderboardTweeningService);

  public:
    void Reset();
    void BindControls(GlobalNamespace::PlatformLeaderboardViewController* platformLeaderboardViewController,
                      SnoreSaber::UI::Other::Banner* banner,
                      SnoreSaber::CustomTypes::Components::LeaderboardScoreInfoButtonHandler* scoreInfoButtonHandler,
                      GlobalNamespace::LeaderboardTableView* tableView,
                      GlobalNamespace::LoadingControl* loadingControl,
                      UnityEngine::UI::Button* pageUpButton,
                      UnityEngine::UI::Button* pageDownButton);
    void AddAvatar(SnoreSaber::UI::Other::ProfilePictureView avatar);
    void ClearAvatars();
    void ApplyState(const SnoreSaber::Data::LeaderboardScreenState& state);
    void SetRankedStatus(const SnoreSaber::Data::LeaderboardDetails& leaderboardInfo);
    void SetErrorState(const std::string& errorText, bool showRefreshButton);
    void SetUploadState(bool state, bool success, const std::string& errorMessage);
    void SetPrompt(const std::string& text, int dismissTime);
    void ApplyLocalPlayerPanelState(SnoreSaber::Data::LocalPlayerPanelState state);

  private:
    void ApplyPageControls(const SnoreSaber::Data::LeaderboardScreenState& state);
    void ResetAvatarCancellation();

    SnoreSaber::UI::Other::LeaderboardAvatarHost _avatarHost;
};
