#pragma once

#include <GlobalNamespace/PlatformLeaderboardViewController.hpp>
#include <HMUI/ImageView.hpp>
#include <HMUI/ModalView.hpp>
#include <UnityEngine/MonoBehaviour.hpp>
#include <UnityEngine/UI/Button.hpp>
#include "Features/Leaderboards/UI/ScoreDetails/LeaderboardScoreInfoButtonHandler.hpp"
#include "Features/Leaderboards/Domain/LeaderboardDetails.hpp"
#include "Features/Leaderboards/Domain/Score.hpp"
#include "Features/Leaderboards/UI/Panel/Banner.hpp"

using namespace GlobalNamespace;

namespace SnoreSaber::UI::Other::SnoreSaberLeaderboardView
{
    enum PageDirection
    {
        Up,
        Down,
    };

    extern SafePtrUnity<SnoreSaber::UI::Other::Banner> SnoreSaberBanner;
    extern SafePtrUnity<SnoreSaber::CustomTypes::Components::LeaderboardScoreInfoButtonHandler> leaderboardScoreInfoButtonHandler;
    extern std::vector<SafePtrUnity<HMUI::ImageView>> _cellClickingImages;

    void OnSoftRestart();

    void EarlyDidActivate(PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHeirarchy, bool screenSystemEnabling);
    void DidActivate(PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHeirarchy, bool screenSystemEnabling);
    void DidDeactivate();
    void ChangeScope(PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry);
    void RefreshLeaderboard();
    void RefreshLeaderboard(BeatmapLevel* beatmapLevel, BeatmapKey beatmapKey, LeaderboardTableView* tableView, PlatformLeaderboardsModel::ScoresScope scope, LoadingControl* loadingControl, int refreshId);
    void SetRankedStatus(const Data::LeaderboardDetails& leaderboardInfo);
    int GetPlayerScoreIndex(std::vector<Data::Score> scores);
    void SetErrorState(LoadingControl* loadingControl, std::string errorText = "Failed to load leaderboard, score won't upload", bool showRefreshButton = true);

    void DirectionalButtonClicked(PageDirection direction);
    void SetPlayButtonState(bool state);
    void SetUploadState(bool state, bool success, std::string errorMessage = "<color=#fc8181>Upload failed</color>");
    void CheckPage();

    void AllowReplayWatching(bool value);
    bool IsReplayWatchingAllowed();

} // namespace SnoreSaber::UI::Other::SnoreSaberLeaderboardView
