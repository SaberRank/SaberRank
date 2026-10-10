#pragma once

#include "Features/Live/Cancellation.hpp"
#include "Features/Players/Profile/PlayerProfileModal.hpp"
#include "Features/Players/UI/QuestPairingModal.hpp"

#include <HMUI/ImageView.hpp>
#include <HMUI/ModalView.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <UnityEngine/Color.hpp>
#include <UnityEngine/MonoBehaviour.hpp>
#include <UnityEngine/Sprite.hpp>
#include <UnityEngine/Transform.hpp>
#include <UnityEngine/UI/VerticalLayoutGroup.hpp>
#include <custom-types/shared/coroutine.hpp>
#include <custom-types/shared/macros.hpp>
#include <paper2_scotland2/shared/string_convert.hpp>
#include <bsml/shared/BSML/Components/Backgroundable.hpp>
#include <bsml/shared/BSML/Components/ClickableImage.hpp>
#include <bsml/shared/BSML-Lite.hpp>
#include <cstdint>
#include <memory>
#include <string_view>

DECLARE_CLASS_CODEGEN(SnoreSaber::UI::Other, Banner, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_FIELD(UnityW<BSML::Backgroundable>, bg);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ImageView>, bgImage);
    DECLARE_INSTANCE_FIELD(UnityW<BSML::ClickableText>, topText);
    DECLARE_INSTANCE_FIELD(UnityW<BSML::ClickableText>, bottomText);
    DECLARE_INSTANCE_FIELD(UnityW<SnoreSaber::UI::Other::PlayerProfileModal>, playerProfileModal);
    DECLARE_INSTANCE_FIELD(UnityW<SnoreSaber::Features::Players::UI::QuestPairingModal>, questPairingModal);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::UI::VerticalLayoutGroup>, loadingVertical);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ImageView>, logoImage);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, promptText);

public:

    static SnoreSaber::UI::Other::Banner * Create(UnityEngine::Transform * parent);
    void Setup();
    void OpenMainMenuFlowCoordinator();
    void OpenSettingsFlowCoordinator();
    void OpenCompeteFlowCoordinator();
    void OpenPlayerInfoModal();
    void OpenSongInBrowser();

    void set_prompt(std::string text, int dismissTime, bool loading = false);
    void DismissLoadingPrompt();
    void set_color(UnityEngine::Color color);
    void set_special_panel(bool usesWilliumsPanel, bool usesDenyahPanel);

    void set_ranking(int rank, float pp);
    void set_status(std::string_view status, int scoreboardId);

    void set_loading(bool value);
    void Prompt(std::string status, bool loadingIndicator, float dismiss,
                std::function<void()> callback);

    void set_topText(std::u16string_view newText);
    void set_topText(std::string_view newText) { set_topText(Paper::StringConvert::from_utf8(newText)); };
    void set_bottomText(std::u16string_view newText);
    void set_bottomText(std::string_view newText) { set_bottomText(Paper::StringConvert::from_utf8(newText)); };

private:
    static constexpr float LogoBlinkSeconds = 1.0f;
    static constexpr float SpecialPanelColorSpeed = 0.1f;
    float logoBlinkElapsed = 0.0f;
    float specialPanelValue = 0.0f;
    bool logoHighlighted = false;
    bool usesWilliumsPanel = false;
    bool usesDenyahPanel = false;
    int scoreboardId;
    int promptVersion = 0;
    bool promptLoading = false;
    UnityW<UnityEngine::Sprite> defaultBackgroundSprite;
    UnityW<UnityEngine::Sprite> denyahBackgroundSprite;
    UnityW<BSML::ClickableImage> competeButton;
    bool competeButtonActive = false;
    bool hasActiveTournaments = false;
    std::shared_ptr<SnoreSaber::Features::Live::CancellationSource> tournamentActionsCancellation;
    uint64_t loginStatusToken = 0;
    static constexpr const UnityEngine::Color defaultColor = {0, 0.47, 0.72, 1.0};
    static constexpr const UnityEngine::Color logoHighlightColor = {0.60f, 0.80f, 1.0f, 1.0f};
    static constexpr float PromptTweenTime = 0.5f;
    custom_types::Helpers::Coroutine SetPrompt(
        std::string status, bool loadingIndicator, float dismiss,
        std::function<void()> callback);
    void TickLogoBlink(float deltaTime);
    void TickSpecialPanel(float deltaTime);
    void ApplyTournamentActionVisibility();
    void RefreshTournamentActionVisibility();
    void CancelTournamentActionRefresh();
    void BeginPrompt(std::string_view text);
    void AnimatePromptIn();
    void DismissPrompt(float dismissTime, float tweenTime = PromptTweenTime);
    void KillPromptTweens();
    float GetPromptAlpha();
    void SetPromptAlpha(float alpha);
};
