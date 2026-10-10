#include "Features/Leaderboards/UI/Panel/Banner.hpp"

#include "Core/Api/SnoreSaberUrls.hpp"
#include "Data/Private/Settings.hpp"
#include "assets.hpp"
#include <HMUI/CurvedCanvasSettingsHelper.hpp>
#include <HMUI/ImageView.hpp>
#include <HMUI/ViewController.hpp>
#include "Features/Leaderboards/Services/LeaderboardTweeningService.hpp"
#include "Features/Live/Compete/Services/CompeteDirectoryService.hpp"
#include "Features/MainMenu/SnoreSaberMenuNavigator.hpp"
#include "Features/Players/Services/GameSessionService.hpp"
#include "Features/Players/Services/PlayerService.hpp"
#include "Sprites.hpp"
#include <System/Action.hpp>
#include <TMPro/TextAlignmentOptions.hpp>
#include <UnityEngine/Application.hpp>
#include <UnityEngine/CanvasGroup.hpp>
#include <UnityEngine/Rect.hpp>
#include <UnityEngine/RectOffset.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/Sprite.hpp>
#include <UnityEngine/SpriteMeshType.hpp>
#include <UnityEngine/Texture2D.hpp>
#include <UnityEngine/Time.hpp>
#include <UnityEngine/UI/LayoutElement.hpp>
#include <UnityEngine/UI/ContentSizeFitter.hpp>
#include <UnityEngine/WaitForSeconds.hpp>
#include "Utils/AsyncUtils.hpp"
#include "Utils/UIUtils.hpp"
#include "logging.hpp"
#include <custom-types/shared/delegate.hpp>
#include <bsml/shared/BSML/Components/Backgroundable.hpp>
#include <bsml/shared/Helpers/creation.hpp>
#include <bsml/shared/Helpers/getters.hpp>
#include <bsml/shared/Helpers/utilities.hpp>
#include <array>
#include <cmath>
#include "Utils/StrippedMethods.hpp"

DEFINE_TYPE(SnoreSaber::UI::Other, Banner);

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace HMUI;
using namespace BSML;
using namespace BSML::Helpers;
using namespace BSML::Lite;
using namespace SnoreSaber::Data::Private;

#define SetPreferredSize(identifier, width, height)                                         \
    auto layout##identifier = identifier->gameObject->GetComponent<LayoutElement*>(); \
    if (!layout##identifier)                                                                \
        layout##identifier = identifier->gameObject->AddComponent<LayoutElement*>();  \
    layout##identifier->preferredWidth = width;                                          \
    layout##identifier->preferredHeight = height

namespace SnoreSaber::UI::Other
{
    namespace
    {
        UnityEngine::Color EvaluateSpecialPanelColor(float value)
        {
            struct ColorKey
            {
                float time;
                UnityEngine::Color color;
            };

            static const std::array<ColorKey, 7> colorKeys = {
                ColorKey {0.00f, UnityEngine::Color(1.0f, 0.0f, 0.0f, 1.0f)},
                ColorKey {0.17f, UnityEngine::Color(1.0f, 0.5f, 0.0f, 1.0f)},
                ColorKey {0.34f, UnityEngine::Color(1.0f, 1.0f, 0.0f, 1.0f)},
                ColorKey {0.51f, UnityEngine::Color(0.0f, 1.0f, 0.0f, 1.0f)},
                ColorKey {0.68f, UnityEngine::Color(0.0f, 0.0f, 1.0f, 1.0f)},
                ColorKey {0.85f, UnityEngine::Color(0.5f, 0.0f, 0.5f, 1.0f)},
                ColorKey {1.00f, UnityEngine::Color(1.0f, 0.0f, 0.0f, 1.0f)}
            };

            float normalized = std::fmod(value, 1.0f);
            if (normalized < 0.0f)
            {
                normalized += 1.0f;
            }

            for (size_t i = 1; i < colorKeys.size(); ++i)
            {
                if (normalized <= colorKeys[i].time)
                {
                    float span = colorKeys[i].time - colorKeys[i - 1].time;
                    float amount = span <= 0.0f ? 0.0f : (normalized - colorKeys[i - 1].time) / span;
                    return UnityEngine::Color::Lerp(colorKeys[i - 1].color, colorKeys[i].color, amount);
                }
            }

            return colorKeys.back().color;
        }
    }

    Sprite* GetGameSprite(StringW name)
    {
        for (auto x : Resources::FindObjectsOfTypeAll<Sprite*>()) {
            if (x->name == name) {
                return x;
            }
        }
        return nullptr;
    }

    SnoreSaber::Features::MainMenu::SnoreSaberMenuNavigator* GetMenuNavigator()
    {
        auto container = BSML::Helpers::GetDiContainer();
        return container ? container->TryResolve<SnoreSaber::Features::MainMenu::SnoreSaberMenuNavigator*>() : nullptr;
    }

    SnoreSaber::Features::Leaderboards::Services::LeaderboardTweeningService* GetTweeningService()
    {
        auto container = BSML::Helpers::GetDiContainer();
        return container ? container->TryResolve<SnoreSaber::Features::Leaderboards::Services::LeaderboardTweeningService*>() : nullptr;
    }

    SnoreSaber::Features::Players::Services::GameSessionService* GetGameSessionService()
    {
        auto container = BSML::Helpers::GetDiContainer();
        return container ? container->TryResolve<SnoreSaber::Features::Players::Services::GameSessionService*>() : nullptr;
    }

    SnoreSaber::Features::Live::Compete::Services::CompeteDirectoryService* GetCompeteDirectoryService()
    {
        auto container = BSML::Helpers::GetDiContainer();
        return container ? container->TryResolve<SnoreSaber::Features::Live::Compete::Services::CompeteDirectoryService*>() : nullptr;
    }

    bool HasAuthenticatedGameSession()
    {
        auto session = GetGameSessionService();
        return session && session->HasAuthenticatedSession() &&
               session->GetStatus() == SnoreSaber::Services::PlayerService::LoginStatus::Success;
    }

    Banner* Banner::Create(Transform* parent)
    {
        auto panel = CreateCanvas();
        auto panelTransform = panel->transform;
        panelTransform->SetParent(parent, false);

        auto panelRectTransform = panel->GetComponent<RectTransform*>();
        panelRectTransform->localScale = Vector3(1, 1, 1);
        panelRectTransform->anchoredPosition = {7.5f, 50.0f};

        auto banner = panel->gameObject->AddComponent<Banner*>();
        banner->Setup();
        return banner;
    }

    void Banner::Setup()
    {
        // tbh with all this layout stuff I fuck around till it works the way I want it to
        auto horizon = CreateHorizontalLayoutGroup(transform);
        horizon->childForceExpandWidth = false;
        horizon->childForceExpandHeight = true;
        horizon->childControlWidth = false;
        horizon->childControlHeight = true;
        horizon->spacing = 2.0f;
        horizon->padding = RectOffset::New_ctor(2, 2, 2, 2);
        SetPreferredSize(horizon, 90.5, 14);

        auto buttonVertical = CreateVerticalLayoutGroup(horizon->transform);
        auto seperatorVertical = CreateVerticalLayoutGroup(horizon->transform);
        auto infoVertical = CreateVerticalLayoutGroup(horizon->transform);
        loadingVertical = CreateVerticalLayoutGroup(transform);
        auto competeVertical = CreateVerticalLayoutGroup(horizon->transform);
        auto settingsVertical = CreateVerticalLayoutGroup(horizon->transform);

        SetPreferredSize(buttonVertical, 10, 10);
        SetPreferredSize(seperatorVertical, 0.5f, 10);
        SetPreferredSize(infoVertical, 62, 10);
        SetPreferredSize(loadingVertical, 10, 10);
        SetPreferredSize(competeVertical, 6, 10);
        SetPreferredSize(settingsVertical, 6, 10);

        bg = horizon->gameObject->AddComponent<Backgroundable*>();
        bg->ApplyBackground("title-gradient");
        bg->ApplyAlpha(1.0f);

        bgImage = bg->gameObject->GetComponentInChildren<ImageView*>();
        bgImage->_skew = 0.18f;
        bgImage->_gradient = true;
        bgImage->_gradientDirection = 0;
        bgImage->_color0 = Color(1, 1, 1, 1);
        bgImage->_color1 = Color(1, 1, 1, 0);
        bgImage->_curvedCanvasSettingsHelper->Reset();
        defaultBackgroundSprite = bgImage->sprite;

        set_color(defaultColor);

        // main menu button setup
        float buttonSize = 10.0f;
        auto btn = CreateUIButton(buttonVertical->transform, "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&Banner::OpenMainMenuFlowCoordinator, this));
        btn->transform->GetChild(0).cast<RectTransform>()->sizeDelta = {buttonSize, buttonSize};

        auto snoresaber_active = Base64ToSprite(SnoreSaber_Active);
        auto snoresaber_inactive = Base64ToSprite(SnoreSaber_Inactive);
        SetButtonSprites(btn, snoresaber_inactive, snoresaber_active);
        auto btnImageView = btn->gameObject->GetComponentInChildren<ImageView*>();
        logoImage = btnImageView;
        btnImageView->_skew = 0.18f;
        BSML::Lite::AddHoverHint(btn->gameObject, "Opens the SnoreSaber main menu");
        auto btnLayout = buttonVertical->gameObject->AddComponent<LayoutElement*>();
        btnLayout->preferredWidth = buttonSize;

        // seperator setup
        auto texture = Texture2D::get_whiteTexture();
        auto seperatorSprite = Sprite::Create(texture, Rect(0.0f, 0.0f, (float)texture->width, (float)texture->height), Vector2(0.5f, 0.5f), 1024.0f, 1u, SpriteMeshType::FullRect, Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);

        auto image = CreateImage(seperatorVertical->transform, seperatorSprite, Vector2(0, 0), Vector2(0, 0));
        image->_skew = 0.18f;
        auto imageLayout = image->gameObject->AddComponent<LayoutElement*>();
        imageLayout->preferredWidth = 1.0f;

        // info setup
        topText = CreateClickableText(infoVertical->transform, "");
        bottomText = CreateClickableText(infoVertical->transform, "");

        SafePtrUnity<Banner> self(this);
        topText->onClick += [self]() { self->OpenPlayerInfoModal(); };
        bottomText->onClick += [self]() { self->OpenSongInBrowser(); };

        auto loadingHorizontal = CreateHorizontalLayoutGroup(loadingVertical->transform);
        UIUtils::CreateLoadingIndicator(loadingHorizontal->transform);
        SetPreferredSize(loadingHorizontal, 10, 10);

        // compete button setup; hidden until an active tournament is available
        competeButton = CreateClickableImage(competeVertical->transform, BSML::Utilities::LoadSpriteRaw(IncludedAssets::light_sabers_png), std::bind(&Banner::OpenCompeteFlowCoordinator, this), Vector2(0, 0), Vector2(6, 6));
        competeButton->preserveAspect = true;
        BSML::Lite::AddHoverHint(competeButton->gameObject, "Enter tournament");
        competeButton->gameObject->SetActive(false);

        // settings button setup
        auto setbtn = CreateClickableImage(settingsVertical->transform, GetGameSprite("SettingsIcon"), std::bind(&Banner::OpenSettingsFlowCoordinator, this), Vector2(0, 0), Vector2(6, 6));
        setbtn->preserveAspect = true;
        BSML::Lite::AddHoverHint(setbtn->gameObject, "Opens the SnoreSaber Settings menu");
        auto setbtnLayout = settingsVertical->gameObject->AddComponent<LayoutElement*>();
        setbtnLayout->preferredWidth = 6;
        auto setbtnRectTransform = settingsVertical->rectTransform;
        setbtnRectTransform->anchoredPosition = {35.0f, 0.0f};


        auto promptRoot = CreateHorizontalLayoutGroup(transform);
        promptRoot->childAlignment = TextAnchor::UpperLeft;
        promptRoot->childForceExpandWidth = false;
        promptRoot->spacing = 1.0f;

        RectTransform* promptRootRect = promptRoot->rectTransform;
        promptRootRect->anchoredPosition = {0.0f, 10.3f};

        LayoutElement* promptElement = promptRoot->GetComponent<LayoutElement*>();
        promptElement->preferredHeight = 7.0f;
        promptElement->preferredWidth = 87.0f;

        ContentSizeFitter* promptFitter = promptRoot->GetComponent<ContentSizeFitter*>();
        promptFitter->horizontalFit = ContentSizeFitter::FitMode::PreferredSize;

        HorizontalLayoutGroup* textGroup = CreateHorizontalLayoutGroup(promptRootRect);
        textGroup->rectTransform->anchoredPosition = {0.0f, 10.0f};

        promptText = CreateText(textGroup->transform, "...", TMPro::FontStyles::Normal);
        promptText->alignment = TMPro::TextAlignmentOptions::BottomLeft;

        if (auto session = GetGameSessionService())
        {
            loginStatusToken = session->AddLoginStatusChangedHandler([self](SnoreSaber::Services::PlayerService::LoginStatus status, const std::string&) {
                if (!self)
                {
                    return;
                }

                if (status == SnoreSaber::Services::PlayerService::LoginStatus::Success)
                {
                    self->RefreshTournamentActionVisibility();
                    return;
                }

                self->hasActiveTournaments = false;
                self->CancelTournamentActionRefresh();
                self->ApplyTournamentActionVisibility();
            });
        }
        RefreshTournamentActionVisibility();
    }

    void Banner::OpenMainMenuFlowCoordinator()
    {
        if (!Settings::hasClickedSnoreSaberLogo)
        {
            Settings::hasClickedSnoreSaberLogo = true;
            Settings::SaveSettings();
            if (logoImage)
            {
                logoImage->color = Color::get_white();
            }
        }

        auto navigator = GetMenuNavigator();
        if (!navigator)
        {
            ERROR("SnoreSaber menu navigator is not available");
            return;
        }

        navigator->ShowMain();
    }

    void Banner::OpenSettingsFlowCoordinator()
    {
        auto navigator = GetMenuNavigator();
        if (!navigator)
        {
            ERROR("SnoreSaber menu navigator is not available");
            return;
        }

        navigator->ShowSettings();
    }

    void Banner::OpenCompeteFlowCoordinator()
    {
        if (!competeButtonActive)
        {
            return;
        }

        // a managed exception escaping the click handler aborts the game through
        // BSML's OnPointerClick, so keep it contained here
        try
        {
            auto navigator = GetMenuNavigator();
            if (!navigator)
            {
                ERROR("SnoreSaber menu navigator is not available");
                return;
            }

            navigator->ShowCompete();
        }
        catch (const std::exception& e)
        {
            ERROR("Failed to open compete flow: {}", e.what());
        }
    }

    void Banner::OpenPlayerInfoModal()
    {
        // just make sure to have this actually assigned
        auto localPlayerId = SnoreSaber::Services::PlayerService::GetLocalPlayerId();
        if (!localPlayerId.empty() && playerProfileModal && Object::IsNativeObjectAlive(playerProfileModal))
        {
            playerProfileModal->Show(localPlayerId);
        }
        else if (localPlayerId.empty() && questPairingModal && Object::IsNativeObjectAlive(questPairingModal))
        {
            // signed out: offer the device pairing flow instead of the profile
            questPairingModal->Show();
        }
    }

    void Banner::OpenSongInBrowser()
    {
        StrippedMethods::UnityEngine::Application::OpenURL(SnoreSaber::Core::Api::SnoreSaberUrls::Leaderboard(scoreboardId));
    }

    void Banner::Update()
    {
        float deltaTime = Time::get_deltaTime();
        TickLogoBlink(deltaTime);
        TickSpecialPanel(deltaTime);
        ApplyTournamentActionVisibility();
    }

    void Banner::OnDestroy()
    {
        CancelTournamentActionRefresh();
        if (loginStatusToken != 0)
        {
            if (auto session = GetGameSessionService())
            {
                session->RemoveLoginStatusChangedHandler(loginStatusToken);
            }
            loginStatusToken = 0;
        }

        if (denyahBackgroundSprite)
        {
            auto texture = denyahBackgroundSprite->texture;
            UnityEngine::Object::Destroy(denyahBackgroundSprite.ptr());
            if (texture)
            {
                UnityEngine::Object::Destroy(texture);
            }
        }
        denyahBackgroundSprite = nullptr;
    }

    void Banner::TickLogoBlink(float deltaTime)
    {
        if (!logoImage || Settings::hasClickedSnoreSaberLogo)
        {
            return;
        }

        logoBlinkElapsed += deltaTime;
        if (logoBlinkElapsed < LogoBlinkSeconds)
        {
            return;
        }

        logoBlinkElapsed = 0.0f;
        logoHighlighted = !logoHighlighted;
        logoImage->color = logoHighlighted ? logoHighlightColor : Color::get_white();
    }

    void Banner::TickSpecialPanel(float deltaTime)
    {
        if (!bgImage || !usesWilliumsPanel || usesDenyahPanel)
        {
            return;
        }

        set_color(EvaluateSpecialPanelColor(specialPanelValue));
        specialPanelValue += deltaTime * SpecialPanelColorSpeed;
        if (specialPanelValue > 1.0f)
        {
            specialPanelValue = 0.0f;
        }
    }

    void Banner::ApplyTournamentActionVisibility()
    {
        competeButtonActive = HasAuthenticatedGameSession() && hasActiveTournaments;
        if (competeButton && competeButton->gameObject->activeSelf != competeButtonActive)
        {
            competeButton->gameObject->SetActive(competeButtonActive);
        }
    }

    void Banner::RefreshTournamentActionVisibility()
    {
        if (!HasAuthenticatedGameSession())
        {
            hasActiveTournaments = false;
            ApplyTournamentActionVisibility();
            return;
        }

        CancelTournamentActionRefresh();
        auto directoryService = GetCompeteDirectoryService();
        if (!directoryService)
        {
            return;
        }

        auto refreshSource = std::make_shared<SnoreSaber::Features::Live::CancellationSource>();
        tournamentActionsCancellation = refreshSource;

        // directory service is container-rooted; the banner is only reached through the SafePtr
        SafePtrUnity<Banner> self(this);
        SnoreSaber::Utils::Async::Run([self, refreshSource, directoryService] {
            bool hasTournaments = false;
            bool completed = false;
            try
            {
                hasTournaments = !directoryService->GetActiveTournaments(refreshSource->Token()).empty();
                completed = true;
            }
            catch (const SnoreSaber::Features::Live::OperationCanceledException&)
            {
            }
            catch (const std::exception& exception)
            {
                WARN("Failed to refresh live tournament availability: {:s}", exception.what());
                completed = true;
            }

            SnoreSaber::Utils::Async::Main([self, refreshSource, hasTournaments, completed] {
                if (!self || self->tournamentActionsCancellation != refreshSource)
                {
                    return;
                }

                if (completed)
                {
                    self->hasActiveTournaments = hasTournaments;
                }
                self->tournamentActionsCancellation = nullptr;
                self->ApplyTournamentActionVisibility();
            });
        });
    }

    void Banner::CancelTournamentActionRefresh()
    {
        if (!tournamentActionsCancellation)
        {
            return;
        }

        tournamentActionsCancellation->Cancel();
        tournamentActionsCancellation = nullptr;
    }

    void Banner::set_special_panel(bool usesWilliumsPanel, bool usesDenyahPanel)
    {
        this->usesWilliumsPanel = usesWilliumsPanel;
        this->usesDenyahPanel = usesDenyahPanel;

        if (!bgImage)
        {
            return;
        }

        if (usesDenyahPanel)
        {
            if (!denyahBackgroundSprite)
            {
                denyahBackgroundSprite = BSML::Utilities::LoadSpriteRaw(IncludedAssets::bri_ish_png);
            }
            if (denyahBackgroundSprite)
            {
                bgImage->sprite = denyahBackgroundSprite;
            }
            bgImage->_gradient = false;
            bgImage->color = Color::get_white();
            bgImage->_color0 = Color::get_white();
            bgImage->_color1 = Color::get_white();
            bgImage->_curvedCanvasSettingsHelper->Reset();
            return;
        }

        if (defaultBackgroundSprite)
        {
            bgImage->sprite = defaultBackgroundSprite;
        }
        bgImage->_gradient = true;
        bgImage->_gradientDirection = 0;
        bgImage->_color0 = Color(1, 1, 1, 1);
        bgImage->_color1 = Color(1, 1, 1, 0);
        bgImage->_curvedCanvasSettingsHelper->Reset();

        if (!usesWilliumsPanel)
        {
            set_color(defaultColor);
        }
    }

    void Banner::Prompt(std::string status, bool loadingIndicator, float dismiss,
                        std::function<void()> callback)
    {
        this->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(SetPrompt(status, loadingIndicator, dismiss, callback)));
    }

    custom_types::Helpers::Coroutine Banner::SetPrompt(
        std::string status, bool showIndicator, float dismiss,
        std::function<void()> callback)
    {
        if (!Settings::showStatusText) {
            co_return;
        }

        int promptId = ++promptVersion;
        promptLoading = showIndicator;
        BeginPrompt(status);

        std::string text = status;

        for (int i = 1; i < (dismiss * 2) + 1; i++)
        {
            co_yield reinterpret_cast<System::Collections::IEnumerator*>(
                CRASH_UNLESS(WaitForSeconds::New_ctor(0.5f)));

            if (promptId != promptVersion)
                co_return;

            // Couldn't get the loading indicator to work so right now it just displays
            // dots as the loading indicator
            if (showIndicator)
            {
                if (i % 4 != 0)
                {
                    text = text + ".";
                    promptText->text = text;
                }
                else
                {
                    for (int k = 0; k < 3; k++)
                    {
                        text.pop_back();
                    }
                    promptText->text = text;
                }
            }
        }

        if (dismiss > 0 && promptId == promptVersion)
        {
            promptLoading = false;
            DismissPrompt(0.0f);
        }

        if (callback)
        {
            callback();
        }

        co_return;
    }

    void Banner::set_prompt(std::string text, int dismissTime, bool loading)
    {
        if (!Settings::showStatusText) {
            return;
        }

        ++promptVersion;
        promptLoading = loading;
        BeginPrompt(text);
        if (dismissTime != -1)
        {
            DismissPrompt(static_cast<float>(dismissTime));
        }
    }

    void Banner::DismissLoadingPrompt()
    {
        if (!promptLoading)
        {
            return;
        }

        promptLoading = false;
        ++promptVersion;
        DismissPrompt(0.0f);
    }

    void Banner::BeginPrompt(std::string_view text)
    {
        bool wasActive = !static_cast<std::string>(promptText->text).empty();
        promptText->text = text;
        if (wasActive)
        {
            KillPromptTweens();
            SetPromptAlpha(1.0f);
        }
        else
        {
            AnimatePromptIn();
        }
    }

    void Banner::AnimatePromptIn()
    {
        auto tweeningService = GetTweeningService();
        if (!tweeningService || !promptText)
        {
            return;
        }

        auto promptRoot = promptText->rectTransform;
        float promptY = promptRoot->localPosition.y;
        tweeningService->CreatePromptTween("panel_prompt_show", 0.0f, 1.0f, PromptTweenTime, 0.0f, promptRoot, promptY, promptY);
    }

    void Banner::DismissPrompt(float dismissTime, float tweenTime)
    {
        int promptId = promptVersion;
        SafePtrUnity<Banner> self(this);
        auto tweeningService = GetTweeningService();
        if (!tweeningService || !promptText)
        {
            SnoreSaber::Utils::Async::After(dismissTime, [self, promptId] {
                if (self && self->promptVersion == promptId)
                {
                    self->promptText->text = std::string();
                    self->promptLoading = false;
                }
            });
            return;
        }

        float startValue = 1.0f;
        if (dismissTime <= 0.0f)
        {
            // fade out from wherever the show tween got to
            startValue = GetPromptAlpha();
            tweeningService->KillTween("panel_prompt_show");
        }

        auto promptRoot = promptText->rectTransform;
        float promptY = promptRoot->localPosition.y;
        tweeningService->CreatePromptTween("panel_prompt_dismiss", startValue, 0.0f, tweenTime, dismissTime, promptRoot, promptY, promptY, [self, promptId] {
            if (self && self->promptVersion == promptId)
            {
                self->promptText->text = std::string();
                self->SetPromptAlpha(1.0f);
                self->promptLoading = false;
            }
        });
    }

    void Banner::KillPromptTweens()
    {
        auto tweeningService = GetTweeningService();
        if (tweeningService)
        {
            tweeningService->KillTween("panel_prompt_show");
            tweeningService->KillTween("panel_prompt_dismiss");
        }
    }

    float Banner::GetPromptAlpha()
    {
        if (!promptText)
        {
            return 1.0f;
        }

        auto canvasGroup = promptText->GetComponent<CanvasGroup*>();
        return canvasGroup ? canvasGroup->alpha : 1.0f;
    }

    void Banner::SetPromptAlpha(float alpha)
    {
        if (!promptText)
        {
            return;
        }

        auto canvasGroup = promptText->GetComponent<CanvasGroup*>();
        if (canvasGroup)
        {
            canvasGroup->alpha = alpha;
        }
    }

    void Banner::set_color(UnityEngine::Color color)
    {
        bgImage->color = color;
    }

    void Banner::set_loading(bool value)
    {
        loadingVertical->gameObject->SetActive(value);
        topText->gameObject->SetActive(!value);
        bottomText->gameObject->SetActive(!value);
    }

    void Banner::set_ranking(int rank, float pp)
    {
        if (Settings::showLocalPlayerRank) {
            set_topText(fmt::format("<b><color=#FFDE1A>Global Ranking: </color></b>#{:d}<size=3> (<color=#6772E5>{:.2f} ZZ</color></size>)", rank, pp));
        } else {
            set_topText("<b>Hidden</b>");
        }
        set_loading(false);
    }

    void Banner::set_status(std::string_view status, int scoreboardId)
    {
        set_bottomText(fmt::format("<b><color=#FFDE1A>Ranked Status:</color></b> {:s}", status.data()));
        this->scoreboardId = scoreboardId;
        set_loading(false);
    }

    void Banner::set_topText(std::u16string_view newText)
    {
        topText->text = u"<i>" + std::u16string(newText) + u"</i>";
        topText->gameObject->SetActive(true);
    }

    void Banner::set_bottomText(std::u16string_view newText)
    {
        bottomText->text = u"<i>" + std::u16string(newText) + u"</i>";
        bottomText->gameObject->SetActive(true);
    }
} // namespace SnoreSaber::UI::Other
