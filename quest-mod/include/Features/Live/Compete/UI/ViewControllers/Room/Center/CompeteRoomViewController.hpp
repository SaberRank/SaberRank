#pragma once

#include "Features/Live/Compete/Domain/CompeteMapStartCountdown.hpp"
#include "Features/Live/Compete/Domain/CompeteOrganizerPrompt.hpp"
#include "Features/Live/Compete/Domain/CompeteRoom.hpp"
#include "Features/Live/Compete/UI/Components/CompeteSongPreview.hpp"
#include "Utils/Event.hpp"

#include <HMUI/ImageView.hpp>
#include <HMUI/ViewController.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/UI/Button.hpp>
#include <UnityEngine/UI/LayoutElement.hpp>
#include <bsml/shared/BSML/Parsing/BSMLParser.hpp>
#include <custom-types/shared/macros.hpp>

#include <memory>
#include <optional>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::UI::ViewControllers::Room::Center, CompeteRoomViewController, HMUI::ViewController) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, roomTitleText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<HMUI::ImageView>, songCover);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::UI::LayoutElement>, songTextColumn);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songNameText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songDetailText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songDifficultyText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::GameObject>, songContentObject);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::GameObject>, songStatusObject);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::GameObject>, songEmptyObject);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songStatusText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songDurationText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songBpmText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songNpsText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songNotesText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songObstaclesText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songBombsText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songNjsText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songJumpDistanceText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, songStarsText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::UI::Button>, readyButton);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, promptMessageText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::UI::Button>, promptPrimaryButton);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::UI::Button>, promptSecondaryButton);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, countdownNumberText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, countdownDetailText);

    DECLARE_INSTANCE_METHOD(void, ToggleReadyClicked);
    DECLARE_INSTANCE_METHOD(void, ShowPlayersClicked);
    DECLARE_INSTANCE_METHOD(void, ShowLeaderboardClicked);
    DECLARE_INSTANCE_METHOD(void, PromptPrimaryClicked);
    DECLARE_INSTANCE_METHOD(void, PromptSecondaryClicked);

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_CTOR(ctor);

  public:
    Utils::Event<> ReadyToggled;
    Utils::Event<> PlayersPanelSelected;
    Utils::Event<> LeaderboardPanelSelected;
    Utils::Event<const Domain::CompeteOrganizerPrompt&, bool> PromptAnswered;

    bool ReadyForPrompt();
    void SetRoom(const Domain::CompeteRoom& room);
    void ShowPrompt(const Domain::CompeteOrganizerPrompt& prompt);
    void ClearPrompt();
    void ShowMapStartCountdown(const Domain::CompeteMapStartCountdown& countdown);
    void HideMapStartCountdown();

  private:
    std::shared_ptr<BSML::BSMLParser> _parser;
    Components::CompeteSongPreview _songPreview;
    std::optional<Domain::CompeteOrganizerPrompt> _activePrompt;
    std::string _roomTitle = "Room";
    std::string _songStatus;
    std::string _readyText = "Ready";

    void AnswerPrompt(bool accepted);
    void BindSongPreview();
    void ApplyRoomTexts();
    void ApplySongTexts();
    void ApplyVisibility();
};
