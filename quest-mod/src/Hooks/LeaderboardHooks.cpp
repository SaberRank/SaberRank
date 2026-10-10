#include <GlobalNamespace/GameplayModifiers.hpp>
#include <GlobalNamespace/PlatformLeaderboardsModel.hpp>
#include <GlobalNamespace/BeatmapLevelsModel.hpp>
#include <GlobalNamespace/LevelCompletionResults.hpp>
#include <GlobalNamespace/MultiplayerLevelScenesTransitionSetupDataSO.hpp>
#include <GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp>
#include <GlobalNamespace/LeaderboardTableView.hpp>
#include <GlobalNamespace/LoadingControl.hpp>
#include <GlobalNamespace/PlatformLeaderboardViewController.hpp>
#include <HMUI/IconSegmentedControl.hpp>
#include <HMUI/SegmentedControl.hpp>
#include <System/Guid.hpp>
#include <System/String.hpp>
#include <beatsaber-hook/shared/utils/hooking.hpp>
#include <GlobalNamespace/MenuTransitionsHelper.hpp>
#include <UnityEngine/GameObject.hpp>
#include <bsml/shared/Helpers/getters.hpp>
#include <metacore/shared/game.hpp>

#include "hooks.hpp"
#include "Data/Private/Settings.hpp"
#include "Features/Leaderboards/UI/SnoreSaberLeaderboardView.hpp"
#include "Features/ScoreSubmission/Services/ScoreSubmissionService.hpp"
#include "Features/Leaderboards/UI/Components/CellClicker.hpp"
#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"
#include <algorithm>

using namespace HMUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;
using namespace SnoreSaber;
using namespace SnoreSaber::CustomTypes::Components;
using namespace SnoreSaber::UI::Other;

int _lastScopeIndex = 0;

MAKE_AUTO_HOOK_MATCH(
    PlatformLeaderboardViewController_DidActivate,
    &GlobalNamespace::PlatformLeaderboardViewController::DidActivate, void,
    GlobalNamespace::PlatformLeaderboardViewController* self,
    bool firstActivation, bool addedToHeirarchy, bool screenSystemEnabling)
{
    bool useSnoreSaber = SnoreSaber::Utils::SnoreSaberBeatmapKey::IsSupported(self->_beatmapKey);
    if (useSnoreSaber)
        SnoreSaber::UI::Other::SnoreSaberLeaderboardView::EarlyDidActivate(self, firstActivation, addedToHeirarchy, screenSystemEnabling);

    PlatformLeaderboardViewController_DidActivate(self, firstActivation, addedToHeirarchy, screenSystemEnabling);

    if (useSnoreSaber) {
        SnoreSaber::UI::Other::SnoreSaberLeaderboardView::DidActivate(self, firstActivation, addedToHeirarchy, screenSystemEnabling);
        auto segmentedControl = self->_scopeSegmentedControl;
        if (segmentedControl)
        {
            int cellCount = segmentedControl->_dataItems.size();
            if (cellCount > 0)
            {
                segmentedControl->SelectCellWithNumber(std::clamp(_lastScopeIndex, 0, cellCount - 1));
            }
        }
    }
}

MAKE_AUTO_HOOK_MATCH(
    PlatformLeaderboardViewController_DidDeactivate,
    &GlobalNamespace::PlatformLeaderboardViewController::DidDeactivate, void,
    GlobalNamespace::PlatformLeaderboardViewController* self,
    bool removedFromHierarchy, bool screenSystemEnabling)
{
    PlatformLeaderboardViewController_DidDeactivate(self, removedFromHierarchy, screenSystemEnabling);
    SnoreSaber::UI::Other::SnoreSaberLeaderboardView::DidDeactivate();
}

MAKE_AUTO_HOOK_MATCH(PlatformLeaderboardViewController_Refresh,
                     &GlobalNamespace::PlatformLeaderboardViewController::Refresh,
                     void, GlobalNamespace::PlatformLeaderboardViewController* self,
                     bool showLoadingIndicator, bool clear)
{
    if (!SnoreSaber::Utils::SnoreSaberBeatmapKey::IsSupported(self->_beatmapKey)) {
        PlatformLeaderboardViewController_Refresh(self, showLoadingIndicator, clear);
        return;
    }

    for (auto holder : SnoreSaberLeaderboardView::_cellClickingImages) {
        if (!holder || !holder.ptr() || !holder->gameObject) {
            continue;
        }

        CellClicker* existingCellClicker = holder->gameObject->GetComponent<CellClicker*>();
        if (existingCellClicker) {
            GameObject::Destroy(existingCellClicker);
        }
    }

    self->_hasScoresData = false;
    self->_scores = System::Collections::Generic::List_1<LeaderboardTableView::ScoreData*>::New_ctor();
    self->_leaderboardTableView->SetScores(self->_scores, self->_playerScorePos[(int)self->getStaticF__scoresScope()]);
    self->_loadingControl->ShowLoading(System::String::getStaticF_Empty());
    BeatmapLevelsModel* _beatmapLevelsModel = BSML::Helpers::GetDiContainer()->Resolve<BeatmapLevelsModel*>();
    BeatmapLevel* beatmapLevel = _beatmapLevelsModel->GetBeatmapLevel(self->_beatmapKey.levelId);
    static int unique_id = 0;
    SnoreSaber::UI::Other::SnoreSaberLeaderboardView::RefreshLeaderboard(beatmapLevel, self->_beatmapKey, self->_leaderboardTableView, self->getStaticF__scoresScope(), self->_loadingControl, ++unique_id);
}

MAKE_AUTO_HOOK_MATCH(PlatformLeaderboardViewController_HandleScopeSegmentedControlDidSelectCell, &GlobalNamespace::PlatformLeaderboardViewController::HandleScopeSegmentedControlDidSelectCell, void,
                     PlatformLeaderboardViewController* self, SegmentedControl* segmentedControl, int cellNumber)
{
    if (!SnoreSaber::Utils::SnoreSaberBeatmapKey::IsSupported(self->_beatmapKey)) {
        PlatformLeaderboardViewController_HandleScopeSegmentedControlDidSelectCell(self, segmentedControl, cellNumber);
        return;
    }

    bool filterAroundCountry = false;
    switch (cellNumber)
    {
        case 0: {
            self->setStaticF__scoresScope(PlatformLeaderboardsModel::ScoresScope::Global);
            break;
        }
        case 1: {
            self->setStaticF__scoresScope(PlatformLeaderboardsModel::ScoresScope::AroundPlayer);
            break;
        }
        case 2: {
            self->setStaticF__scoresScope(PlatformLeaderboardsModel::ScoresScope::Friends);
            break;
        }
        case 3: {
            self->setStaticF__scoresScope(PlatformLeaderboardsModel::ScoresScope::Global);
            filterAroundCountry = SnoreSaber::Data::Private::Settings::enableCountryLeaderboards;
            break;
        }
    }
    _lastScopeIndex = cellNumber;
    SnoreSaber::UI::Other::SnoreSaberLeaderboardView::ChangeScope(self->getStaticF__scoresScope(), filterAroundCountry);
}

MAKE_AUTO_HOOK_MATCH(StandardLevelScenesTransitionSetupDataSO_Finish, &GlobalNamespace::StandardLevelScenesTransitionSetupDataSO::Finish, void,
                     GlobalNamespace::StandardLevelScenesTransitionSetupDataSO* self,
                     GlobalNamespace::LevelCompletionResults* levelCompletionResults)
{
    auto scoreSubmissionService = BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Features::ScoreSubmission::Services::ScoreSubmissionService*>();
    if (scoreSubmissionService)
    {
        scoreSubmissionService->HandleStandardLevelFinished(self, levelCompletionResults);
    }
    StandardLevelScenesTransitionSetupDataSO_Finish(self, levelCompletionResults);
}

MAKE_AUTO_HOOK_MATCH(MultiplayerLevelScenesTransitionSetupDataSO_Finish, &GlobalNamespace::MultiplayerLevelScenesTransitionSetupDataSO::Finish, void,
                     GlobalNamespace::MultiplayerLevelScenesTransitionSetupDataSO* self,
                     GlobalNamespace::MultiplayerResultsData* multiplayerResultsData)
{
    auto scoreSubmissionService = BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Features::ScoreSubmission::Services::ScoreSubmissionService*>();
    if (scoreSubmissionService)
    {
        scoreSubmissionService->HandleMultiplayerLevelFinished(self, multiplayerResultsData);
    }
    MultiplayerLevelScenesTransitionSetupDataSO_Finish(self, multiplayerResultsData);
}
