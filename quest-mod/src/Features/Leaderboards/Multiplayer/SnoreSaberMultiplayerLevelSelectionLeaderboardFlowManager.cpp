#include "Features/Leaderboards/Multiplayer/SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager.hpp"

#include <GlobalNamespace/MultiplayerLevelSelectionFlowCoordinator.hpp>
#include <GlobalNamespace/BeatmapKey.hpp>
#include <HMUI/ViewController.hpp>
#include <System/Action.hpp>
#include <UnityEngine/GameObject.hpp>
#include <custom-types/shared/delegate.hpp>
#include <functional>

#include "Utils/AsyncUtils.hpp"

using namespace BSML;
using namespace GlobalNamespace;

DEFINE_TYPE(SnoreSaber::UI::Multiplayer, SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager);

namespace SnoreSaber::UI::Multiplayer
{
    void SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::ctor(MainFlowCoordinator* mainFlowCoordinator, ServerPlayerListViewController* serverPlayerListViewController, PlatformLeaderboardViewController* platformLeaderboardViewController, LevelSelectionNavigationController* levelSelectionNavigationController)
    {
        _mainFlowCoordinator = mainFlowCoordinator;
        _serverPlayerListViewController = serverPlayerListViewController;
        _platformLeaderboardViewController = platformLeaderboardViewController;
        _levelSelectionNavigationController = levelSelectionNavigationController;
        _currentlyInMulti = false;
        _performingFirstActivation = false;
        _changeDelegatesSubscribed = false;
    }

    void SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::Initialize()
    {
        didActivateDelegate = { &SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::LevelSelectionNavigationController_didActivateEvent, this };
        didDeactivateDelegate = { &SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::LevelSelectionNavigationController_didDeactivateEvent, this };

        _levelSelectionNavigationController->___didActivateEvent += didActivateDelegate;
        _levelSelectionNavigationController->___didDeactivateEvent += didDeactivateDelegate;
    }

    void SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::Dispose()
    {
        if(_levelSelectionNavigationController) {
            _levelSelectionNavigationController->___didActivateEvent -= didActivateDelegate;
            _levelSelectionNavigationController->___didDeactivateEvent -= didDeactivateDelegate;
            if (_changeDelegatesSubscribed)
            {
                _levelSelectionNavigationController->___didChangeDifficultyBeatmapEvent -= didChangeDifficultyBeatmapDelegate;
                _levelSelectionNavigationController->___didChangeLevelDetailContentEvent -= didChangeLevelDetailContentDelegate;
                _changeDelegatesSubscribed = false;
            }
        }

        _currentlyInMulti = false;
    }

    void SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::LevelSelectionNavigationController_didActivateEvent(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (!InMulti())
            return;

        if (firstActivation)
            _performingFirstActivation = true;

        if (_changeDelegatesSubscribed)
        {
            _currentlyInMulti = true;
            return;
        }

        didChangeDifficultyBeatmapDelegate = { &SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::LevelSelectionNavigationController_didChangeDifficultyBeatmapEvent, this };
        didChangeLevelDetailContentDelegate = { &SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::LevelSelectionNavigationController_didChangeLevelDetailContentEvent, this };

        _levelSelectionNavigationController->___didChangeDifficultyBeatmapEvent += didChangeDifficultyBeatmapDelegate;
        _levelSelectionNavigationController->___didChangeLevelDetailContentEvent += didChangeLevelDetailContentDelegate;
        _changeDelegatesSubscribed = true;
        _currentlyInMulti = true;
    }

    void SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::LevelSelectionNavigationController_didDeactivateEvent(bool removedFromHierarchy, bool screenSystemDisabling)
    {
        if (!_changeDelegatesSubscribed)
            return;

        _currentlyInMulti = false;
        _levelSelectionNavigationController->___didChangeDifficultyBeatmapEvent -= didChangeDifficultyBeatmapDelegate;
        _levelSelectionNavigationController->___didChangeLevelDetailContentEvent -= didChangeLevelDetailContentDelegate;
        _changeDelegatesSubscribed = false;
    }

    void SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::LevelSelectionNavigationController_didChangeLevelDetailContentEvent(UnityW<LevelSelectionNavigationController> controller, StandardLevelDetailViewController::ContentType contentType)
    {
        ShowLeaderboard();
    }

    void SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::LevelSelectionNavigationController_didChangeDifficultyBeatmapEvent(UnityW<LevelSelectionNavigationController> controller)
    {
        ShowLeaderboard();
    }

    void SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::HideLeaderboard()
    {
        if (_platformLeaderboardViewController->isInViewControllerHierarchy)
        {
            auto currentFlowCoordinator = _mainFlowCoordinator->YoungestChildFlowCoordinatorOrSelf();
            if (!currentFlowCoordinator.try_cast<MultiplayerLevelSelectionFlowCoordinator>().has_value())
                return;

            currentFlowCoordinator->SetRightScreenViewController(nullptr, HMUI::ViewController::AnimationType::Out);
        }
    }

    void SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::ShowLeaderboard()
    {
        if (!InMulti())
            return;

        if(!_levelSelectionNavigationController->beatmapKey.IsValid()) {
            HideLeaderboard();
            return;
        }

        auto beatmapKey = _levelSelectionNavigationController->beatmapKey;
        _platformLeaderboardViewController->SetData(byref(beatmapKey));
        auto currentFlowCoordinator = _mainFlowCoordinator->YoungestChildFlowCoordinatorOrSelf();
        currentFlowCoordinator->SetRightScreenViewController(_platformLeaderboardViewController, HMUI::ViewController::AnimationType::In);

        _serverPlayerListViewController->gameObject->SetActive(false); // copied from pcvr version: This is a bandaid fix, first time startup it gets stuck while animating kinda like the issue we had before (TODO: Fix in 2024)

        // Copied from the pcvr version, but still a bandaid
        if (_performingFirstActivation)
        {
            _performingFirstActivation = false;

            SafePtr<SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager> self(this);

            SnoreSaber::Utils::Async::After(0.25f, [self] {
                self->_platformLeaderboardViewController->Refresh(true, true);
            });
        }
    }

    bool SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager::InMulti()
    {
        if (_currentlyInMulti)
            return true;

        auto currentFlowCoordinator = _mainFlowCoordinator->YoungestChildFlowCoordinatorOrSelf();
        return currentFlowCoordinator.try_cast<MultiplayerLevelSelectionFlowCoordinator>().has_value();
    }
} // namespace SnoreSaber::UI::Multiplayer
