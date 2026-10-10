

#include <GlobalNamespace/LoadingControl.hpp>
#include <GlobalNamespace/PlatformLeaderboardViewController.hpp>
#include <GlobalNamespace/PlatformLeaderboardsHandler.hpp>
#include <GlobalNamespace/PlatformLeaderboardsModel.hpp>
#include <GlobalNamespace/StandardLevelDetailView.hpp>
#include <GlobalNamespace/StandardLevelDetailViewController.hpp>
#include <HMUI/CurvedCanvasSettingsHelper.hpp>
#include <HMUI/CurvedTextMeshPro.hpp>
#include <HMUI/IconSegmentedControl.hpp>
#include <HMUI/ImageView.hpp>
#include <HMUI/Screen.hpp>
#include <HMUI/StackLayoutGroup.hpp>
#include <HMUI/ViewController.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <System/String.hpp>
#include <System/Action.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Rect.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/SpriteMeshType.hpp>
#include <UnityEngine/SpriteRenderer.hpp>
#include <UnityEngine/Texture2D.hpp>
#include <UnityEngine/UI/Button.hpp>
#include <UnityEngine/UI/LayoutElement.hpp>
#include <beatsaber-hook/shared/utils/hooking.hpp>
#include <bsml/shared/Helpers/utilities.hpp>
#include <custom-types/shared/delegate.hpp>
#include <bsml/shared/BSML/Components/Backgroundable.hpp>
#include <bsml/shared/BSML/FloatingScreen/FloatingScreen.hpp>
#include <bsml/shared/BSML-Lite.hpp>
#include <bsml/shared/Helpers/getters.hpp>
#include <utility>

#include "Features/Leaderboards/UI/SnoreSaberLeaderboardView.hpp"
#include "Features/Leaderboards/LeaderboardBeatmapController.hpp"
#include "Features/Leaderboards/LeaderboardInteractionController.hpp"
#include "Features/Leaderboards/LeaderboardPresentationController.hpp"
#include "Features/Leaderboards/Domain/InternalLeaderboard.hpp"
#include "Features/Leaderboards/Domain/LeaderboardScreenState.hpp"
#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"
#include "Data/Private/Settings.hpp"
#include "Features/ScoreSubmission/ScoreSubmissionController.hpp"
#include "Features/Leaderboards/Services/LeaderboardScreenLoader.hpp"
#include "Features/Leaderboards/Services/LeaderboardScreenSession.hpp"
#include "Features/Players/Services/PlayerService.hpp"
#include "Features/Replays/ReplayLoader.hpp"
#include "Core/Presentation/SnoreSaberUIMaterials.hpp"
#include "Sprites.hpp"
#include "Features/Players/Profile/ProfilePictureView.hpp"
#include "Utils/AsyncUtils.hpp"
#include "Utils/UIUtils.hpp"
#include "Utils/GCUtil.hpp"
#include "logging.hpp"

using namespace HMUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace BSML;
using namespace BSML::Lite;
using namespace BSML::Helpers;
using namespace GlobalNamespace;
using namespace SnoreSaber;
using namespace SnoreSaber::CustomTypes;
using namespace SnoreSaber::Services;
using namespace SnoreSaber::Data::Private;

namespace SnoreSaber::UI::Other::SnoreSaberLeaderboardView
{
    namespace
    {
        using LeaderboardBeatmapController = SnoreSaber::Features::Leaderboards::LeaderboardBeatmapController;
        using LeaderboardScreenLoader = SnoreSaber::Features::Leaderboards::Services::LeaderboardScreenLoader;
        using LeaderboardScreenSession = SnoreSaber::Features::Leaderboards::Services::LeaderboardScreenSession;
        using LeaderboardInteractionController = SnoreSaber::Features::Leaderboards::LeaderboardInteractionController;
        using LeaderboardPresentationController = SnoreSaber::Features::Leaderboards::LeaderboardPresentationController;

        LeaderboardScreenSession* GetLeaderboardSession()
        {
            return BSML::Helpers::GetDiContainer()->TryResolve<LeaderboardScreenSession*>();
        }

        LeaderboardScreenLoader* GetLeaderboardLoader()
        {
            return BSML::Helpers::GetDiContainer()->TryResolve<LeaderboardScreenLoader*>();
        }

        LeaderboardBeatmapController* GetLeaderboardBeatmapController()
        {
            return BSML::Helpers::GetDiContainer()->TryResolve<LeaderboardBeatmapController*>();
        }

        LeaderboardInteractionController* GetLeaderboardInteractionController()
        {
            return BSML::Helpers::GetDiContainer()->TryResolve<LeaderboardInteractionController*>();
        }

        LeaderboardPresentationController* GetLeaderboardPresentationController()
        {
            return BSML::Helpers::GetDiContainer()->TryResolve<LeaderboardPresentationController*>();
        }

        SnoreSaber::Core::Presentation::SnoreSaberUIMaterials* GetUIMaterials()
        {
            return BSML::Helpers::GetDiContainer()->Resolve<SnoreSaber::Core::Presentation::SnoreSaberUIMaterials*>();
        }
    }

    SafePtrUnity<SnoreSaber::UI::Other::Banner> SnoreSaberBanner;

    SafePtrUnity<SnoreSaber::CustomTypes::Components::LeaderboardScoreInfoButtonHandler> leaderboardScoreInfoButtonHandler;

    SafePtrUnity<PlatformLeaderboardViewController> _platformLeaderboardViewController;

    SafePtrUnity<UnityEngine::UI::Button> _pageUpButton;
    SafePtrUnity<UnityEngine::UI::Button> _pageDownButton;

    std::vector<SafePtrUnity<HMUI::ImageView>> _cellClickingImages;

    bool _activated = false;

    bool _allowReplayWatching = true;
    int _authRequestId = 0;

    void OnSoftRestart()
    {
        _activated = false;
        _authRequestId++;
        if (auto session = GetLeaderboardSession())
        {
            session->Reset();
        }
        _allowReplayWatching = true;

        SnoreSaberBanner = nullptr;
        leaderboardScoreInfoButtonHandler = nullptr;
        _platformLeaderboardViewController = nullptr;
        _pageUpButton = nullptr;
        _pageDownButton = nullptr;

        if (auto presentationController = GetLeaderboardPresentationController())
        {
            presentationController->Reset();
        }
        _cellClickingImages.clear();
    }

    void ByeImages()
    {
        if (auto presentationController = GetLeaderboardPresentationController())
        {
            presentationController->ClearAvatars();
        }
    }

    void EarlyDidActivate(PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHeirarchy,
                     bool screenSystemEnabling)
    {
        if (auto beatmapController = GetLeaderboardBeatmapController())
        {
            beatmapController->OnLeaderboardSet(self->_beatmapKey);
        }
    }

    void DidActivate(PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHeirarchy,
                     bool screenSystemEnabling)
    {
        if (firstActivation || !_platformLeaderboardViewController)
        {
            INFO("PlatformLeaderboardViewController firstActivation");

            _platformLeaderboardViewController = self;

            // Page Buttons

            if (!_pageUpButton)
            {
                _pageUpButton = CreateUIButton(self->transform, "", "SettingsButton", Vector2(20.0f, -21.0f), Vector2(5.0f, 5.0f),
                                                                     [=]() {
                                                                         DirectionalButtonClicked(PageDirection::Up);
                                                                     });

                SetButtonSprites(_pageUpButton.ptr(), Base64ToSprite(carat_up_inactive_base64),
                                                       Base64ToSprite(carat_up_base64));

                auto rectTransform = _pageUpButton->transform->GetChild(0).cast<RectTransform>();
                rectTransform->sizeDelta = {10.0f, 10.0f};
            }

            if (!_pageDownButton)
            {
                _pageDownButton = CreateUIButton(self->transform, "", "SettingsButton", Vector2(20.0f, -60.0f), Vector2(5.0f, 5.0f),
                                                                       [=]() {
                                                                           DirectionalButtonClicked(PageDirection::Down);
                                                                       });

                SetButtonSprites(_pageDownButton.ptr(), Base64ToSprite(carat_down_inactive_base64),
                                                       Base64ToSprite(carat_down_base64));
                auto rectTransform = _pageDownButton->transform->GetChild(0).cast<RectTransform>();
                rectTransform->sizeDelta = {10.0f, 10.0f};
            }

            // RedBrumbler top panel

            SnoreSaberBanner = ::SnoreSaber::UI::Other::Banner::Create(self->transform);
            auto playerProfileModal = ::SnoreSaber::UI::Other::PlayerProfileModal::Create(self->transform);
            SnoreSaberBanner->playerProfileModal = playerProfileModal;

            auto questPairingModal = ::SnoreSaber::Features::Players::UI::QuestPairingModal::Create(self->transform);
            SnoreSaberBanner->questPairingModal = questPairingModal;
            auto bannerForPairing = SnoreSaberBanner;
            auto leaderboardForPairing = _platformLeaderboardViewController;
            questPairingModal->onPaired = [bannerForPairing, leaderboardForPairing]() {
                if (bannerForPairing)
                {
                    bannerForPairing->set_prompt("<color=#89fc81>Successfully signed in to SnoreSaber</color>", 2);
                }
                if (leaderboardForPairing)
                {
                    leaderboardForPairing->Refresh(true, true);
                }
            };

            bool supportedBeatmap = Utils::SnoreSaberBeatmapKey::IsSupported(self->_beatmapKey);
            if (supportedBeatmap)
                SnoreSaberBanner->Prompt("Signing into SnoreSaber...", true, 5.0f, nullptr);
            auto newGo = GameObject::New_ctor();
            auto t = newGo->transform;
            t->transform->SetParent(self->transform, false);
            t->localScale = {1, 1, 1};

            leaderboardScoreInfoButtonHandler = newGo->AddComponent<SnoreSaber::CustomTypes::Components::LeaderboardScoreInfoButtonHandler*>();
            auto replayLoader = BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::ReplaySystem::ReplayLoader*>();
            leaderboardScoreInfoButtonHandler->Setup(replayLoader);
            leaderboardScoreInfoButtonHandler->scoreInfoModal->playerProfileModal = playerProfileModal;

            auto presentationController = GetLeaderboardPresentationController();
            if (presentationController)
            {
                presentationController->Reset();
                presentationController->BindControls(self, SnoreSaberBanner.ptr(), leaderboardScoreInfoButtonHandler.ptr(), self->_leaderboardTableView, self->_loadingControl, _pageUpButton.ptr(), _pageDownButton.ptr());
            }

            // profile pictures
            auto pfpVertical = CreateVerticalLayoutGroup(self->transform);
            pfpVertical->rectTransform->anchoredPosition = Vector2(-21, -1);
            pfpVertical->spacing = -20.15;

            auto uiMaterials = GetUIMaterials();
            auto nullSprite = uiMaterials->BlankSprite();

            for (int i = 0; i < 10; ++i) {
                auto rowHorizontal = CreateHorizontalLayoutGroup(pfpVertical->transform);
                rowHorizontal->childForceExpandHeight = true;
                rowHorizontal->childAlignment = TextAnchor::MiddleCenter;

                auto rowStack = CreateStackLayoutGroup(rowHorizontal->transform);
                
                auto image = CreateImage(rowStack->transform, nullSprite, {0.0f, 0.0f}, {4.75f, 4.75f});
                image->preserveAspect = true;
                auto imageLayout = image->gameObject->GetComponent<UnityEngine::UI::LayoutElement*>();
                imageLayout->preferredHeight = 4.75f;
                imageLayout->preferredWidth = 4.75f;

                auto loadingIndicator = UIUtils::CreateLoadingIndicator(rowStack->transform);
                auto loadingIndicatorLayout = loadingIndicator->gameObject->GetComponent<UnityEngine::UI::LayoutElement*>();
                loadingIndicatorLayout->preferredWidth = 3.75f;
                loadingIndicatorLayout->preferredHeight = 3.75f;
                loadingIndicator->SetActive(false);

                if (presentationController)
                {
                    presentationController->AddAvatar(ProfilePictureView(image, loadingIndicator));
                }
            }

            // cell clickers
            auto clickVertical = CreateVerticalLayoutGroup(self->transform);
            clickVertical->rectTransform->anchoredPosition = Vector2(5, -1);
            clickVertical->spacing = -20.25;

            auto mat_UINoGlowRoundEdge = uiMaterials->RoundedImageMaterial();

            _cellClickingImages.clear();
            for (int i = 0; i < 10; ++i) {
                auto rowHorizontal = CreateHorizontalLayoutGroup(clickVertical->transform);
                rowHorizontal->childForceExpandHeight = true;
                rowHorizontal->childAlignment = TextAnchor::MiddleCenter;

                auto rowStack = CreateStackLayoutGroup(rowHorizontal->transform);
                
                auto image = CreateImage(rowStack->transform, nullSprite, {0.0f, 0.0f}, {72.0f, 5.75f});
                image->material = mat_UINoGlowRoundEdge;
                image->preserveAspect = true;
                auto imageLayout = image->gameObject->GetComponent<UnityEngine::UI::LayoutElement*>();
                imageLayout->preferredWidth = 72.0f;
                imageLayout->preferredHeight = 5.75f;

                _cellClickingImages.emplace_back(image);
            }

            if (supportedBeatmap)
            {
                int authRequestId = ++_authRequestId;
                auto bannerSafe = SnoreSaberBanner;
                auto leaderboardControllerSafe = _platformLeaderboardViewController;
                PlayerService::AuthenticateUser([authRequestId, bannerSafe, leaderboardControllerSafe](PlayerService::LoginStatus loginStatus) {
                    SnoreSaber::Utils::Async::Main([=]() {
                        if (authRequestId != _authRequestId || !bannerSafe || !leaderboardControllerSafe)
                        {
                            return;
                        }

                        switch (loginStatus)
                        {
                            case PlayerService::LoginStatus::Success: {
                                bannerSafe->set_prompt("<color=#89fc81>Successfully signed in to SnoreSaber</color>", 2);
                                INFO("Refresh 1");
                                leaderboardControllerSafe->Refresh(true, true);
                                break;
                            }
                            case PlayerService::LoginStatus::Error: {
                                bannerSafe->set_prompt("<color=#fc8181>Not signed in</color> - select 'Sign in' on the SnoreSaber banner", -1);
                                // the local player panel never loads without auth, so clear its
                                // spinner or it covers the sign-in text and eats the clicks
                                bannerSafe->set_loading(false);
                                bannerSafe->set_topText("<b>Sign in to SnoreSaber</b>");
                                break;
                            }
                            case PlayerService::LoginStatus::None:
                            case PlayerService::LoginStatus::InProgress: {
                                break;
                            }
                        }
                    });
                });
            }
            else
            {
                SnoreSaberBanner->set_prompt(Utils::SnoreSaberBeatmapKey::IsWip(self->_beatmapKey) ? "SnoreSaber doesn't support WIP levels" : "SnoreSaber doesn't support this level", 5);
            }
        }

        // we have to set this up again, because locationFilterMode could have changed
        Sprite* globalLeaderboardIcon = self->_globalLeaderboardIcon;
        Sprite* friendsLeaderboardIcon = self->_friendsLeaderboardIcon;
        Sprite* aroundPlayerLeaderboardIcon = self->_aroundPlayerLeaderboardIcon;
        Sprite* countryLeaderboardIcon = Base64ToSprite(country_base64);
        countryLeaderboardIcon->textureRect.size = {64.0f, 64.0f};

        IconSegmentedControl* scopeSegmentedControl = self->_scopeSegmentedControl;

        std::string mode = Settings::locationFilterMode;
        for (auto& c : mode)
        {
            c = tolower(c);
        }

        ::Array<IconSegmentedControl::DataItem*>* array = ::Array<IconSegmentedControl::DataItem*>::New({
            IconSegmentedControl::DataItem::New_ctor(globalLeaderboardIcon, "Global", true),
            IconSegmentedControl::DataItem::New_ctor(aroundPlayerLeaderboardIcon, "Around You", true),
            IconSegmentedControl::DataItem::New_ctor(friendsLeaderboardIcon, "Friends", true),
            IconSegmentedControl::DataItem::New_ctor(countryLeaderboardIcon, mode == "region" ? "Region" : "Country", true),
        });

        scopeSegmentedControl->SetData(array);

        ByeImages();

        _activated = true;
    }

    void DidDeactivate()
    {
        if (SnoreSaberBanner && SnoreSaberBanner->playerProfileModal)
            SnoreSaberBanner->playerProfileModal->Hide();
        if (leaderboardScoreInfoButtonHandler && leaderboardScoreInfoButtonHandler->scoreInfoModal)
            leaderboardScoreInfoButtonHandler->scoreInfoModal->Hide();
    }

    void ApplyLeaderboardState(Data::LeaderboardScreenState state, BeatmapLevel* beatmapLevel, BeatmapKey beatmapKey, LeaderboardTableView* tableView,
                               LoadingControl* loadingControl, int maxScore)
    {
        (void)beatmapLevel;
        (void)beatmapKey;
        (void)maxScore;

        auto presentationController = GetLeaderboardPresentationController();
        if (!presentationController)
        {
            return;
        }

        presentationController->BindControls(_platformLeaderboardViewController.ptr(), SnoreSaberBanner.ptr(), leaderboardScoreInfoButtonHandler.ptr(), tableView, loadingControl, _pageUpButton.ptr(), _pageDownButton.ptr());
        presentationController->ApplyState(state);
    }

    void RefreshLeaderboard(BeatmapLevel* beatmapLevel, BeatmapKey beatmapKey, LeaderboardTableView* tableView,
                            PlatformLeaderboardsModel::ScoresScope scope, LoadingControl* loadingControl,
                            int refreshId)
    {
        auto scoreSubmissionController = BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Features::ScoreSubmission::ScoreSubmissionController*>();
        if (scoreSubmissionController && scoreSubmissionController->IsUploading())
        {
            return;
        }

        if (!_activated)
        {
            return;
        }

        auto session = GetLeaderboardSession();
        auto loader = GetLeaderboardLoader();
        if (!session || !loader)
        {
            return;
        }

        int page = session->Page();
        bool filterAroundCountry = session->FilterAroundCountry();
        ApplyLeaderboardState(Data::LeaderboardScreenState::Loading(page, session->CanPageScope(scope, filterAroundCountry)), beatmapLevel, beatmapKey, tableView, loadingControl, 0);

        (void)refreshId;
        int sessionRefreshId = session->BeginRefresh(scope, filterAroundCountry);

        SafePtr<LeaderboardScreenSession> sessionSafe(session);
        SafePtr<LeaderboardScreenLoader> loaderSafe(loader);
        SafePtr<BeatmapLevel> beatmapLevelSafe(beatmapLevel);
        SafePtrUnity<LoadingControl> loadingControlSafe(loadingControl);
        SafePtrUnity<LeaderboardTableView> tableViewSafe(tableView);

        SnoreSaber::Utils::Async::After(0.5f, [sessionSafe, loaderSafe, beatmapLevelSafe, beatmapKey, scope, loadingControlSafe, tableViewSafe, sessionRefreshId] {
            if (!sessionSafe->IsCurrentRefresh(sessionRefreshId))
            {
                return;
            }

            SnoreSaber::Utils::Async::Run([sessionSafe, loaderSafe, beatmapLevelSafe, beatmapKey, scope, loadingControlSafe, tableViewSafe, sessionRefreshId] {
                loaderSafe->Load(beatmapLevelSafe.ptr(),
                    beatmapKey,
                    scope,
                    sessionSafe->Page(),
                    sessionSafe->FilterAroundCountry(),
                    gc_aware_function([sessionSafe, beatmapLevelSafe, beatmapKey, loadingControlSafe, tableViewSafe, sessionRefreshId](SnoreSaber::Features::Leaderboards::Services::LoadResult result) {
                        SnoreSaber::Utils::Async::Main([sessionSafe, beatmapLevelSafe, beatmapKey, loadingControlSafe, tableViewSafe, sessionRefreshId, result]() {
                            if (!sessionSafe->IsCurrentRefresh(sessionRefreshId)) {
                                return; // we need to check this again, since some time may have passed due to waiting for leaderboard data
                            }
                            ApplyLeaderboardState(result.state, beatmapLevelSafe.ptr(), beatmapKey, tableViewSafe.ptr(), loadingControlSafe.ptr(), result.maxScore);
                        });
                    }));
            });
        });
    }

    void SetRankedStatus(const Data::LeaderboardDetails& leaderboardInfo)
    {
        if (auto presentationController = GetLeaderboardPresentationController())
        {
            presentationController->SetRankedStatus(leaderboardInfo);
        }
    }

    void SetErrorState(LoadingControl* loadingControl, std::string errorText, bool showRefreshButton)
    {
        auto presentationController = GetLeaderboardPresentationController();
        if (!presentationController)
        {
            return;
        }

        presentationController->BindControls(_platformLeaderboardViewController.ptr(), SnoreSaberBanner.ptr(), leaderboardScoreInfoButtonHandler.ptr(), nullptr, loadingControl, _pageUpButton.ptr(), _pageDownButton.ptr());
        presentationController->SetErrorState(errorText, showRefreshButton);
    }
    
    void RefreshLeaderboard()
    {
        if (!_activated)
        {
            return;
        }

        _platformLeaderboardViewController->Refresh(true, true);
    }

    void ChangeScope(PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry)
    {
        if (!_activated)
        {
            return;
        }

        if (auto interactionController = GetLeaderboardInteractionController())
        {
            interactionController->SelectScope(scope, filterAroundCountry);
        }
        _platformLeaderboardViewController->Refresh(true, true);
        CheckPage();
    }

    void CheckPage()
    {
        auto interactionController = GetLeaderboardInteractionController();
        if (interactionController && interactionController->CanPageUp())
        {
            _pageUpButton->interactable = true;
        }
        else
        {
            _pageUpButton->interactable = false;
        }
    }

    void DirectionalButtonClicked(PageDirection direction)
    {
        auto interactionController = GetLeaderboardInteractionController();
        if (!interactionController)
        {
            return;
        }

        switch (direction)
        {
            case PageDirection::Up: {
                interactionController->PageUp();
                break;
            }
            case PageDirection::Down: {
                interactionController->PageDown();
                break;
            }
        }
        _platformLeaderboardViewController->Refresh(true, true);
        CheckPage();
    }

    void SetUploadState(bool state, bool success, std::string errorMessage)
    {
        if (auto presentationController = GetLeaderboardPresentationController())
        {
            presentationController->SetUploadState(state, success, errorMessage);
        }
    }

    void AllowReplayWatching(bool value)
    {
        _allowReplayWatching = value;
    }

    bool IsReplayWatchingAllowed()
    {
        return _allowReplayWatching;
    }

} // namespace SnoreSaber::UI::Other::SnoreSaberLeaderboardView
