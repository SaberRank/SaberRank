#pragma once

#include "Core/Configuration/SettingsService.hpp"
#include "Features/Live/Ludus/Domain/LiveChatEntry.hpp"
#include "Features/Live/Ludus/Services/LiveChatLinkService.hpp"
#include "Features/Live/Ludus/Services/LudusSessionService.hpp"

#include <HMUI/ViewController.hpp>
#include <TMPro/TMP_FontAsset.hpp>
#include <TMPro/TextAlignmentOptions.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <UnityEngine/Color.hpp>
#include <UnityEngine/Coroutine.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/RectTransform.hpp>
#include <UnityEngine/Transform.hpp>
#include <UnityEngine/Vector2.hpp>
#include <bsml/shared/BSML/Components/Keyboard/ModalKeyboard.hpp>
#include <bsml/shared/BSML/Parsing/BSMLParser.hpp>
#include <custom-types/shared/coroutine.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace SnoreSaber::Features::Live::Ludus::UI
{
    struct LiveChatFloatingRow
    {
        std::string title;
        std::string detail;
        std::string status;
        std::string displayText;
        bool isChat = false;
        std::optional<Services::LiveChatLinkTarget> linkTarget;

        bool HasAccent() const
        {
            return !isChat || linkTarget.has_value();
        }
    };
}

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Ludus::UI, LiveChatFloatingViewController, HMUI::ViewController) {
    DECLARE_INSTANCE_FIELD_PRIVATE(Core::Configuration::SettingsService*, _settings);
    DECLARE_INSTANCE_FIELD_PRIVATE(Services::LudusSessionService*, _ludusSession);
    DECLARE_INSTANCE_FIELD_PRIVATE(Services::LiveChatLinkService*, _linkService);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<BSML::ModalKeyboard>, chatKeyboard);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::GameObject>, _chatButtonObject);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::GameObject>, _keyboardDismissZonesObject);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityEngine::Coroutine*, _statusClearCoroutine);
    DECLARE_INSTANCE_FIELD_PRIVATE(TMPro::TMP_FontAsset*, _chatFont);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, _messageMeasurementText);

    DECLARE_INSTANCE_METHOD(StringW, get_chatDraft);
    DECLARE_INSTANCE_METHOD(void, set_chatDraft, StringW value);
    DECLARE_INSTANCE_METHOD(void, ChatEntered, StringW value);

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnDestroy, &HMUI::ViewController::OnDestroy);

    DECLARE_INJECT_METHOD(void, Construct,
                          Core::Configuration::SettingsService* settings,
                          Services::LudusSessionService* ludusSession,
                          Services::LiveChatLinkService* linkService);
    DECLARE_CTOR(ctor);

  public:
    static constexpr float ChatWidth = 120.0f;
    static constexpr float ChatHeight = 140.0f;

    void SetMessages(const std::vector<Domain::LiveChatEntry>& messages);
    void SetViewerCount(int viewerCount);
    void RefreshLayoutSettings();
    void SetStatus(const std::string& value);
    void ResumeStatusAutoClear();

  private:
    std::shared_ptr<BSML::BSMLParser> _parser;
    std::string _chatDraft;
    std::string _status;
    std::string _viewerStatus;
    float _appliedTextScale = -1.0f;
    int _statusVersion = 0;
    bool _statusAutoClear = false;
    std::vector<Domain::LiveChatEntry> _currentMessages;
    std::vector<LiveChatFloatingRow> _visibleRows;
    std::vector<UnityW<UnityEngine::GameObject>> _messageObjects;
    uint64_t _statusChangedToken = 0;
    uint64_t _resolvedTextChangedToken = 0;

    void SetStatus(const std::string& value, bool autoClear);
    // mirrors the pc status property setter (assign + re-render)
    void ApplyStatus(const std::string& value);
    void StartStatusAutoClearIfReady();
    custom_types::Helpers::Coroutine ClearStatusAfterDelay(int statusVersion);
    void RebuildMessages();
    void Parsed();
    void SendChatValue(const std::string& value);
    void OpenLink(const Services::LiveChatLinkTarget& target);
    float CurrentTextScale();
    bool HasStatusLine();
    void RenderMessageRows();
    float RowHeightFor(const LiveChatFloatingRow& row);
    void EnsureChatButton();
    void OpenKeyboard();
    void HideKeyboard();
    custom_types::Helpers::Coroutine PositionKeyboardNextFrame();
    void PositionKeyboard();
    void ApplyKeyboardSizing(UnityEngine::RectTransform* rectTransform);
    void ApplyKeyboardBackgroundSizing(UnityEngine::RectTransform* rectTransform);
    void EnsureKeyboardDismissZones();
    void AddKeyboardDismissZone(UnityEngine::Transform* parent, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta);
    void ClearMessageObjects();
    void AddViewerStatusLine();
    void AddStatusLine();
    void AddMessageRow(const LiveChatFloatingRow& row, float y, float height);
    void AddAccent(UnityEngine::Transform* parent, UnityEngine::Color color, float height);
    float MeasureMessageTextHeight(const std::string& value);
    TMPro::TextMeshProUGUI* MessageMeasurementText();
    TMPro::TextMeshProUGUI* AddText(UnityEngine::Transform* parent, const std::string& value, UnityEngine::Color color, float fontSize, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta, TMPro::TextAlignmentOptions alignment, bool wordWrapping = false);
    void ConfigureText(TMPro::TextMeshProUGUI* text, UnityEngine::Color color, float fontSize, TMPro::TextAlignmentOptions alignment, bool wordWrapping);
    TMPro::TMP_FontAsset* ChatFont();
};
