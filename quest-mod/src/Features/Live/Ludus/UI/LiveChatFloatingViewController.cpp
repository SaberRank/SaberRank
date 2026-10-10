#include "Features/Live/Ludus/UI/LiveChatFloatingViewController.hpp"

#include "Utils/AsyncUtils.hpp"
#include "assets.hpp"
#include "logging.hpp"

#include <System/Collections/IEnumerator.hpp>
#include <TMPro/TextOverflowModes.hpp>
#include <UnityEngine/Camera.hpp>
#include <UnityEngine/Events/UnityAction.hpp>
#include <UnityEngine/Quaternion.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/UI/Button.hpp>
#include <UnityEngine/UI/Image.hpp>
#include <UnityEngine/UI/Selectable.hpp>
#include <UnityEngine/Vector3.hpp>
#include <UnityEngine/WaitForSeconds.hpp>
#include <bsml/shared/BSML.hpp>
#include <bsml/shared/Helpers/getters.hpp>
#include <custom-types/shared/delegate.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>

using namespace UnityEngine;

DEFINE_TYPE(SnoreSaber::Features::Live::Ludus::UI, LiveChatFloatingViewController);

namespace SnoreSaber::Features::Live::Ludus::UI
{
    namespace
    {
        constexpr int VisibleMessageCount = 10;
        constexpr float FooterReserve = 10.0f;
        constexpr float FooterWithStatusReserve = 18.0f;
        constexpr float TopPadding = 4.0f;
        constexpr float MessageRowMinHeight = 8.4f;
        constexpr float MessageTextWidth = LiveChatFloatingViewController::ChatWidth - 11.0f;
        constexpr float MessageTextFontSize = 3.4f;
        constexpr float MessageTextVerticalPadding = 2.0f;
        constexpr float TextLineSpacing = 1.5f;
        constexpr float KeyboardDistance = 0.75f;
        constexpr float KeyboardVerticalOffset = -0.28f;
        constexpr float KeyboardWidth = 50.0f;
        constexpr float KeyboardHeight = 28.0f;
        constexpr float KeyboardContentScale = 0.46f;
        constexpr float KeyboardDismissWidth = 150.0f;
        constexpr float KeyboardDismissHeight = 90.0f;
        constexpr float StatusAutoClearSeconds = 3.0f;
        constexpr const char* DefaultStatus = "Spectator chat";

        constexpr Color ChatBackground(0.0f, 0.0f, 0.0f, 0.12f);
        constexpr Color ChatHighlight(0.980f, 0.800f, 0.082f, 0.08f);
        constexpr Color ChatAccent(0.980f, 0.800f, 0.082f, 1.0f);
        constexpr Color LinkAccent(0.18f, 0.75f, 1.0f, 1.0f);
        constexpr const char* ChatNameColor = "#CDEEFF";
        constexpr const char* LogNameColor = "#BBBBBB";
        constexpr const char* TimeColor = "#BBBBBB";
        constexpr Color MessageColor(1.0f, 1.0f, 1.0f, 1.0f);

        bool IsBlank(const std::string& value)
        {
            return std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
        }

        std::string FirstNonEmpty(const std::string& value, const std::string& fallback)
        {
            return IsBlank(value) ? fallback : value;
        }

        std::string Trim(const std::string& value)
        {
            size_t begin = value.find_first_not_of(" \t\r\n");
            if (begin == std::string::npos)
            {
                return "";
            }

            size_t end = value.find_last_not_of(" \t\r\n");
            return value.substr(begin, end - begin + 1);
        }

        bool StartsWithIgnoreCase(const std::string& value, const std::string& prefix)
        {
            return value.size() >= prefix.size() &&
                   std::equal(prefix.begin(), prefix.end(), value.begin(), [](unsigned char x, unsigned char y) { return std::tolower(x) == std::tolower(y); });
        }

        bool ShouldAutoClearStatus(const std::string& value)
        {
            if (IsBlank(value) || value == DefaultStatus)
            {
                return false;
            }

            return !StartsWithIgnoreCase(value, "Resolving linked map") &&
                   !StartsWithIgnoreCase(value, "Checking linked map") &&
                   !StartsWithIgnoreCase(value, "Downloading linked map");
        }

        std::string FormatViewerStatus(int viewerCount)
        {
            int safeCount = std::max(0, viewerCount);
            return safeCount == 1 ? "1 viewer" : fmt::format("{:d} viewers", safeCount);
        }

        std::string EscapeRichText(const std::string& value)
        {
            std::string escaped;
            escaped.reserve(value.size());
            for (char c : value)
            {
                escaped += c;
                if (c == '<')
                {
                    escaped += "⁠";
                }
            }

            return escaped;
        }

        std::string Truncate(const std::string& value, size_t maxLength)
        {
            if (value.size() <= maxLength)
            {
                return value;
            }

            return value.substr(0, maxLength - 3) + "...";
        }

        std::string BuildDisplayText(const std::string& status, const std::string& title, const std::string& detail, bool isChat)
        {
            std::string safeStatus = EscapeRichText(status);
            std::string safeTitle = EscapeRichText(FirstNonEmpty(title, isChat ? "Unknown" : "Log"));
            std::string safeDetail = EscapeRichText(detail);
            const char* nameColor = isChat ? ChatNameColor : LogNameColor;
            std::string timePrefix = safeStatus.empty() ? "" : fmt::format("<color={:s}>{:s}</color> ", TimeColor, safeStatus);
            return fmt::format("{:s}<color={:s}><b>{:s}</b></color>: {:s}", timePrefix, nameColor, safeTitle, safeDetail);
        }

        void ApplyNoGlow(UnityEngine::UI::Image* image)
        {
            auto material = BSML::Helpers::GetUINoGlowMat();
            if (image && material)
            {
                image->material = material;
            }
        }
    }

    void LiveChatFloatingViewController::ctor()
    {
        INVOKE_CTOR();
        _status = DefaultStatus;
    }

    void LiveChatFloatingViewController::Construct(Core::Configuration::SettingsService* settings,
                                                   Services::LudusSessionService* ludusSession,
                                                   Services::LiveChatLinkService* linkService)
    {
        _settings = settings;
        _ludusSession = ludusSession;
        _linkService = linkService;
        _statusChangedToken = _linkService->StatusChanged.Add([this](const std::string& value) { SetStatus(value); });
        _resolvedTextChangedToken = _linkService->ResolvedTextChanged.Add([this] { RebuildMessages(); });
    }

    void LiveChatFloatingViewController::OnDestroy()
    {
        if (_linkService)
        {
            _linkService->StatusChanged.Remove(_statusChangedToken);
            _linkService->ResolvedTextChanged.Remove(_resolvedTextChangedToken);
        }

        HMUI::ViewController::OnDestroy();
    }

    StringW LiveChatFloatingViewController::get_chatDraft()
    {
        return _chatDraft;
    }

    void LiveChatFloatingViewController::set_chatDraft(StringW value)
    {
        _chatDraft = value ? static_cast<std::string>(value) : "";
    }

    void LiveChatFloatingViewController::SetMessages(const std::vector<Domain::LiveChatEntry>& messages)
    {
        _currentMessages = messages;
        RebuildMessages();
    }

    void LiveChatFloatingViewController::SetViewerCount(int viewerCount)
    {
        std::string nextStatus = viewerCount < 0 ? "" : FormatViewerStatus(viewerCount);
        if (_viewerStatus == nextStatus)
        {
            return;
        }

        _viewerStatus = nextStatus;
        RenderMessageRows();
    }

    void LiveChatFloatingViewController::RefreshLayoutSettings()
    {
        float nextTextScale = CurrentTextScale();
        if (std::abs(_appliedTextScale - nextTextScale) < 0.001f)
        {
            return;
        }

        _appliedTextScale = nextTextScale;
        RebuildMessages();
    }

    void LiveChatFloatingViewController::RebuildMessages()
    {
        _visibleRows.clear();
        size_t start = _currentMessages.size() > VisibleMessageCount ? _currentMessages.size() - VisibleMessageCount : 0;
        for (size_t i = start; i < _currentMessages.size(); i++)
        {
            const Domain::LiveChatEntry& entry = _currentMessages[i];
            LiveChatFloatingRow row;
            row.isChat = entry.IsChat();
            row.linkTarget = _linkService->FirstLink(entry.text);
            row.title = row.isChat ? FirstNonEmpty(_linkService->DisplaySenderName(entry), "Unknown") : "Log";
            row.detail = Truncate(_linkService->DisplayText(entry), 180);
            row.status = entry.DisplayTime();
            row.displayText = BuildDisplayText(row.status, row.title, row.detail, row.isChat);
            _visibleRows.push_back(std::move(row));
        }

        RenderMessageRows();
    }

    void LiveChatFloatingViewController::SetStatus(const std::string& value)
    {
        SetStatus(value, ShouldAutoClearStatus(value));
    }

    void LiveChatFloatingViewController::ResumeStatusAutoClear()
    {
        StartStatusAutoClearIfReady();
    }

    void LiveChatFloatingViewController::SetStatus(const std::string& value, bool autoClear)
    {
        _statusVersion++;
        ApplyStatus(value.empty() ? DefaultStatus : value);
        _statusAutoClear = autoClear && ShouldAutoClearStatus(_status);

        if (_statusClearCoroutine)
        {
            StopCoroutine(_statusClearCoroutine);
            _statusClearCoroutine = nullptr;
        }

        StartStatusAutoClearIfReady();
    }

    void LiveChatFloatingViewController::ApplyStatus(const std::string& value)
    {
        _status = value;
        RenderMessageRows();
    }

    void LiveChatFloatingViewController::StartStatusAutoClearIfReady()
    {
        if (!_statusAutoClear || _statusClearCoroutine || !gameObject->activeInHierarchy)
        {
            return;
        }

        _statusClearCoroutine = StartCoroutine(custom_types::Helpers::CoroutineHelper::New(ClearStatusAfterDelay(_statusVersion)));
    }

    custom_types::Helpers::Coroutine LiveChatFloatingViewController::ClearStatusAfterDelay(int statusVersion)
    {
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(WaitForSeconds::New_ctor(StatusAutoClearSeconds));
        if (_statusVersion == statusVersion)
        {
            _statusVersion++;
            _statusAutoClear = false;
            ApplyStatus(DefaultStatus);
        }

        _statusClearCoroutine = nullptr;
        co_return;
    }

    void LiveChatFloatingViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            _parser = BSML::parse_and_construct(IncludedAssets::LiveChatFloatingViewController_bsml, transform, this);
            Parsed();
        }
    }

    void LiveChatFloatingViewController::Parsed()
    {
        auto rectTransform = transform.try_cast<RectTransform>().value_or(nullptr);
        if (rectTransform)
        {
            rectTransform->pivot = {0.5f, 0.0f};
            rectTransform->sizeDelta = {ChatWidth, ChatHeight};
        }

        PositionKeyboard();
        EnsureKeyboardDismissZones();
        EnsureChatButton();
        RenderMessageRows();
    }

    void LiveChatFloatingViewController::ChatEntered(StringW value)
    {
        SendChatValue(value ? static_cast<std::string>(value) : "");
    }

    void LiveChatFloatingViewController::SendChatValue(const std::string& value)
    {
        std::string message = Trim(value);
        if (message.empty())
        {
            _chatDraft = "";
            SetStatus("Enter a message first.");
            return;
        }

        if (_ludusSession->SendChatMessage(message))
        {
            _chatDraft = "";
            SetStatus(DefaultStatus, false);
        }
        else
        {
            SetStatus("Live chat is not connected.");
        }
    }

    void LiveChatFloatingViewController::OpenLink(const Services::LiveChatLinkTarget& target)
    {
        SafePtr<Services::LiveChatLinkService> linkService(_linkService);
        Services::LiveChatLinkTarget copy = target;
        Utils::Async::Run([linkService, copy] {
            try
            {
                linkService->Open(copy, CancellationToken());
            }
            catch (const std::exception& ex)
            {
                WARN("Live chat link open failed: {:s}", ex.what());
            }
        });
    }

    float LiveChatFloatingViewController::CurrentTextScale()
    {
        float scale = _settings ? _settings->LiveChatOverlayTextScale() : 1.25f;
        return std::clamp(scale, 0.9f, 1.8f);
    }

    bool LiveChatFloatingViewController::HasStatusLine()
    {
        return !IsBlank(_status) && _status != DefaultStatus;
    }

    void LiveChatFloatingViewController::RenderMessageRows()
    {
        if (!transform)
        {
            return;
        }

        EnsureChatButton();
        ClearMessageObjects();
        AddViewerStatusLine();
        AddStatusLine();

        if (_visibleRows.empty())
        {
            return;
        }

        float y = HasStatusLine() ? FooterWithStatusReserve : FooterReserve;
        for (int i = static_cast<int>(_visibleRows.size()) - 1; i >= 0; i--)
        {
            float rowHeight = RowHeightFor(_visibleRows[i]);
            if (y + rowHeight > ChatHeight - TopPadding)
            {
                break;
            }

            AddMessageRow(_visibleRows[i], y, rowHeight);
            y += rowHeight;
        }
    }

    float LiveChatFloatingViewController::RowHeightFor(const LiveChatFloatingRow& row)
    {
        float minimumHeight = MessageRowMinHeight * CurrentTextScale();
        float measuredHeight = MeasureMessageTextHeight(row.displayText) + MessageTextVerticalPadding;
        return measuredHeight < minimumHeight ? minimumHeight : measuredHeight;
    }

    void LiveChatFloatingViewController::EnsureChatButton()
    {
        if (_chatButtonObject || !transform)
        {
            return;
        }

        _chatButtonObject = GameObject::New_ctor("Live Chat Open Button");
        auto rectTransform = _chatButtonObject->AddComponent<RectTransform*>();
        rectTransform->SetParent(transform, false);
        rectTransform->SetAsLastSibling();
        rectTransform->anchorMin = {0.5f, 0.0f};
        rectTransform->anchorMax = {0.5f, 0.0f};
        rectTransform->pivot = {0.5f, 0.0f};
        rectTransform->sizeDelta = {45.0f, 7.5f};
        rectTransform->anchoredPosition = {(ChatWidth * 0.5f) - 24.5f, 1.5f};
        rectTransform->localScale = Vector3::get_one();

        auto background = _chatButtonObject->AddComponent<UnityEngine::UI::Image*>();
        background->color = {0.0f, 0.0f, 0.0f, 0.22f};
        background->raycastTarget = true;
        ApplyNoGlow(background);

        auto button = _chatButtonObject->AddComponent<UnityEngine::UI::Button*>();
        button->transition = UnityEngine::UI::Selectable::Transition::None;
        button->targetGraphic = background;
        button->onClick->AddListener(custom_types::MakeDelegate<Events::UnityAction*>(classof(Events::UnityAction*), (std::function<void()>)[this]() { OpenKeyboard(); }));

        AddText(_chatButtonObject->transform, "Send Message", {0.8f, 0.92f, 1.0f, 0.92f}, 2.45f * CurrentTextScale(), {0.0f, -0.65f}, {45.0f, 7.5f}, TMPro::TextAlignmentOptions::Center);
    }

    void LiveChatFloatingViewController::OpenKeyboard()
    {
        PositionKeyboard();
        EnsureKeyboardDismissZones();
        if (_parser && _parser->parserParams)
        {
            _parser->parserParams->EmitEvent("open-chat-keyboard");
        }

        StartCoroutine(custom_types::Helpers::CoroutineHelper::New(PositionKeyboardNextFrame()));
    }

    custom_types::Helpers::Coroutine LiveChatFloatingViewController::PositionKeyboardNextFrame()
    {
        co_yield nullptr;
        PositionKeyboard();
        EnsureKeyboardDismissZones();
        co_return;
    }

    void LiveChatFloatingViewController::PositionKeyboard()
    {
        if (!chatKeyboard)
        {
            return;
        }

        auto rectTransform = chatKeyboard->transform.try_cast<RectTransform>().value_or(nullptr);
        if (!rectTransform)
        {
            return;
        }

        ApplyKeyboardSizing(rectTransform.ptr());

        auto mainCamera = Camera::get_main();
        if (mainCamera)
        {
            auto cameraTransform = mainCamera->transform;
            Vector3 position = Vector3::op_Addition(
                Vector3::op_Addition(cameraTransform->position, Vector3::op_Multiply(cameraTransform->forward, KeyboardDistance)),
                Vector3::op_Multiply(cameraTransform->up, KeyboardVerticalOffset));
            rectTransform->position = position;
            rectTransform->rotation = Quaternion::LookRotation(Vector3::op_Subtraction(position, cameraTransform->position), cameraTransform->up);
            rectTransform->localScale = Vector3::get_one();
            return;
        }

        rectTransform->anchorMin = {0.5f, 0.0f};
        rectTransform->anchorMax = {0.5f, 0.0f};
        rectTransform->pivot = {0.5f, 0.5f};
        rectTransform->anchoredPosition = {0.0f, -58.0f};
        rectTransform->localScale = Vector3::get_one();
    }

    void LiveChatFloatingViewController::ApplyKeyboardSizing(RectTransform* rectTransform)
    {
        rectTransform->sizeDelta = {KeyboardWidth, KeyboardHeight};
        ApplyKeyboardBackgroundSizing(rectTransform);

        auto keyboardParent = rectTransform->Find("KeyboardParent");
        if (!keyboardParent)
        {
            return;
        }

        auto keyboardParentRect = keyboardParent.try_cast<RectTransform>().value_or(nullptr);
        if (keyboardParentRect)
        {
            keyboardParentRect->anchoredPosition = Vector2::get_zero();
            keyboardParentRect->sizeDelta = {KeyboardWidth, KeyboardHeight};
        }

        keyboardParent->localScale = Vector3::op_Multiply(Vector3::get_one(), KeyboardContentScale);
    }

    void LiveChatFloatingViewController::ApplyKeyboardBackgroundSizing(RectTransform* rectTransform)
    {
        auto background = rectTransform->Find("BG").try_cast<RectTransform>().value_or(nullptr);
        if (!background)
        {
            return;
        }

        background->anchorMin = {0.5f, 0.5f};
        background->anchorMax = {0.5f, 0.5f};
        background->pivot = {0.5f, 0.5f};
        background->anchoredPosition = Vector2::get_zero();
        background->sizeDelta = {KeyboardWidth, KeyboardHeight};
        background->localScale = Vector3::get_one();
    }

    void LiveChatFloatingViewController::HideKeyboard()
    {
        if (_parser && _parser->parserParams)
        {
            _parser->parserParams->EmitEvent("hide-chat-keyboard");
        }
    }

    void LiveChatFloatingViewController::EnsureKeyboardDismissZones()
    {
        if (_keyboardDismissZonesObject || !chatKeyboard)
        {
            return;
        }

        _keyboardDismissZonesObject = GameObject::New_ctor("Live Chat Keyboard Outside Dismiss Zones");
        auto root = _keyboardDismissZonesObject->AddComponent<RectTransform*>();
        root->SetParent(chatKeyboard->transform, false);
        root->SetAsFirstSibling();
        root->anchorMin = {0.5f, 0.5f};
        root->anchorMax = {0.5f, 0.5f};
        root->pivot = {0.5f, 0.5f};
        root->anchoredPosition = Vector2::get_zero();
        root->sizeDelta = Vector2::get_zero();
        root->localScale = Vector3::get_one();

        float sideWidth = (KeyboardDismissWidth - KeyboardWidth) * 0.5f;
        float verticalHeight = (KeyboardDismissHeight - KeyboardHeight) * 0.5f;
        AddKeyboardDismissZone(root, {-(KeyboardWidth * 0.5f) - (sideWidth * 0.5f), 0.0f}, {sideWidth, KeyboardDismissHeight});
        AddKeyboardDismissZone(root, {(KeyboardWidth * 0.5f) + (sideWidth * 0.5f), 0.0f}, {sideWidth, KeyboardDismissHeight});
        AddKeyboardDismissZone(root, {0.0f, (KeyboardHeight * 0.5f) + (verticalHeight * 0.5f)}, {KeyboardWidth, verticalHeight});
        AddKeyboardDismissZone(root, {0.0f, -(KeyboardHeight * 0.5f) - (verticalHeight * 0.5f)}, {KeyboardWidth, verticalHeight});
    }

    void LiveChatFloatingViewController::AddKeyboardDismissZone(Transform* parent, Vector2 anchoredPosition, Vector2 sizeDelta)
    {
        auto zoneObject = GameObject::New_ctor("Dismiss Zone");
        auto rectTransform = zoneObject->AddComponent<RectTransform*>();
        rectTransform->SetParent(parent, false);
        rectTransform->anchorMin = {0.5f, 0.5f};
        rectTransform->anchorMax = {0.5f, 0.5f};
        rectTransform->pivot = {0.5f, 0.5f};
        rectTransform->anchoredPosition = anchoredPosition;
        rectTransform->sizeDelta = sizeDelta;
        rectTransform->localScale = Vector3::get_one();

        auto image = zoneObject->AddComponent<UnityEngine::UI::Image*>();
        image->color = Color::get_clear();
        image->raycastTarget = true;
        ApplyNoGlow(image);

        auto button = zoneObject->AddComponent<UnityEngine::UI::Button*>();
        button->transition = UnityEngine::UI::Selectable::Transition::None;
        button->targetGraphic = image;
        button->onClick->AddListener(custom_types::MakeDelegate<Events::UnityAction*>(classof(Events::UnityAction*), (std::function<void()>)[this]() { HideKeyboard(); }));
    }

    void LiveChatFloatingViewController::ClearMessageObjects()
    {
        for (int i = static_cast<int>(_messageObjects.size()) - 1; i >= 0; i--)
        {
            if (_messageObjects[i])
            {
                UnityEngine::Object::Destroy(_messageObjects[i].ptr());
            }
        }

        _messageObjects.clear();
    }

    void LiveChatFloatingViewController::AddViewerStatusLine()
    {
        if (IsBlank(_viewerStatus))
        {
            return;
        }

        auto root = GameObject::New_ctor("Live Chat Viewer Count");
        _messageObjects.push_back(root);

        auto rectTransform = root->AddComponent<RectTransform*>();
        rectTransform->SetParent(transform, false);
        rectTransform->anchorMin = {0.5f, 0.0f};
        rectTransform->anchorMax = {0.5f, 0.0f};
        rectTransform->pivot = {0.5f, 0.0f};
        rectTransform->sizeDelta = {48.0f, 7.5f};
        rectTransform->anchoredPosition = {-35.0f, 1.5f};
        rectTransform->localScale = Vector3::get_one();

        auto background = root->AddComponent<UnityEngine::UI::Image*>();
        background->color = {0.0f, 0.0f, 0.0f, 0.12f};
        background->raycastTarget = false;
        ApplyNoGlow(background);

        AddAccent(root->transform, ChatAccent, 7.5f);
        AddText(root->transform, _viewerStatus, {0.92f, 0.94f, 0.98f, 0.82f}, 2.15f * CurrentTextScale(), {3.0f, -0.6f}, {43.0f, 7.2f}, TMPro::TextAlignmentOptions::Left);
    }

    void LiveChatFloatingViewController::AddStatusLine()
    {
        if (!HasStatusLine())
        {
            return;
        }

        auto root = GameObject::New_ctor("Live Chat Status");
        _messageObjects.push_back(root);

        auto rectTransform = root->AddComponent<RectTransform*>();
        rectTransform->SetParent(transform, false);
        rectTransform->anchorMin = {0.5f, 0.0f};
        rectTransform->anchorMax = {0.5f, 0.0f};
        rectTransform->pivot = {0.5f, 0.0f};
        rectTransform->sizeDelta = {ChatWidth - 9.0f, 7.5f};
        rectTransform->anchoredPosition = {0.0f, 9.5f};
        rectTransform->localScale = Vector3::get_one();

        auto background = root->AddComponent<UnityEngine::UI::Image*>();
        background->color = {0.0f, 0.0f, 0.0f, 0.22f};
        background->raycastTarget = false;
        ApplyNoGlow(background);

        AddAccent(root->transform, ChatAccent, 7.5f);
        AddText(root->transform, _status, {0.92f, 0.94f, 0.98f, 0.95f}, 2.3f * CurrentTextScale(), {3.0f, -0.6f}, {ChatWidth - 15.0f, 7.2f}, TMPro::TextAlignmentOptions::Left);
    }

    void LiveChatFloatingViewController::AddMessageRow(const LiveChatFloatingRow& row, float y, float height)
    {
        auto root = GameObject::New_ctor("Live Chat Message");
        _messageObjects.push_back(root);

        auto rectTransform = root->AddComponent<RectTransform*>();
        rectTransform->SetParent(transform, false);
        rectTransform->anchorMin = {0.5f, 0.0f};
        rectTransform->anchorMax = {0.5f, 0.0f};
        rectTransform->pivot = {0.5f, 0.0f};
        rectTransform->sizeDelta = {ChatWidth, height};
        rectTransform->anchoredPosition = {0.0f, y};
        rectTransform->localScale = Vector3::get_one();

        auto background = root->AddComponent<UnityEngine::UI::Image*>();
        background->color = row.HasAccent() ? ChatHighlight : ChatBackground;
        background->raycastTarget = row.linkTarget.has_value();
        ApplyNoGlow(background);

        if (row.linkTarget)
        {
            auto button = root->AddComponent<UnityEngine::UI::Button*>();
            button->transition = UnityEngine::UI::Selectable::Transition::None;
            button->targetGraphic = background;
            Services::LiveChatLinkTarget target = *row.linkTarget;
            button->onClick->AddListener(custom_types::MakeDelegate<Events::UnityAction*>(classof(Events::UnityAction*), (std::function<void()>)[this, target]() { OpenLink(target); }));
        }

        if (row.HasAccent())
        {
            AddAccent(root->transform, row.linkTarget ? LinkAccent : ChatAccent, height);
        }

        AddText(root->transform, row.displayText, MessageColor, MessageTextFontSize * CurrentTextScale(), {6.0f, -1.0f}, {MessageTextWidth, height - MessageTextVerticalPadding}, TMPro::TextAlignmentOptions::TopLeft, true);
    }

    void LiveChatFloatingViewController::AddAccent(Transform* parent, Color color, float height)
    {
        auto accent = GameObject::New_ctor("Accent");
        auto rect = accent->AddComponent<RectTransform*>();
        rect->SetParent(parent, false);

        auto image = accent->AddComponent<UnityEngine::UI::Image*>();
        image->color = color;
        image->raycastTarget = false;
        ApplyNoGlow(image);

        rect->anchorMin = {0.0f, 0.0f};
        rect->anchorMax = {0.0f, 0.0f};
        rect->pivot = {0.0f, 0.0f};
        rect->sizeDelta = {1.0f, height};
        rect->anchoredPosition = Vector2::get_zero();
        rect->localScale = Vector3::get_one();
    }

    float LiveChatFloatingViewController::MeasureMessageTextHeight(const std::string& value)
    {
        auto text = MessageMeasurementText();
        if (!text)
        {
            return MessageRowMinHeight * CurrentTextScale();
        }

        ConfigureText(text, Color::get_clear(), MessageTextFontSize * CurrentTextScale(), TMPro::TextAlignmentOptions::TopLeft, true);
        text->text = value;
        text->rectTransform->sizeDelta = {MessageTextWidth, ChatHeight};
        Vector2 preferredValues = text->GetPreferredValues(value, MessageTextWidth, std::numeric_limits<float>::infinity());
        text->text = "";

        if (std::isnan(preferredValues.y) || std::isinf(preferredValues.y) || preferredValues.y <= 0.0f)
        {
            return MessageRowMinHeight * CurrentTextScale();
        }

        return preferredValues.y;
    }

    TMPro::TextMeshProUGUI* LiveChatFloatingViewController::MessageMeasurementText()
    {
        if (_messageMeasurementText)
        {
            return _messageMeasurementText.ptr();
        }

        if (!transform)
        {
            return nullptr;
        }

        auto textObject = GameObject::New_ctor("Live Chat Message Measurement");
        _messageMeasurementText = textObject->AddComponent<TMPro::TextMeshProUGUI*>();
        _messageMeasurementText->rectTransform->SetParent(transform, false);

        auto rect = _messageMeasurementText->rectTransform;
        rect->anchorMin = {0.0f, 1.0f};
        rect->anchorMax = {0.0f, 1.0f};
        rect->pivot = {0.0f, 1.0f};
        rect->anchoredPosition = Vector2::get_zero();
        rect->sizeDelta = {MessageTextWidth, ChatHeight};
        rect->localScale = Vector3::get_one();

        return _messageMeasurementText.ptr();
    }

    TMPro::TextMeshProUGUI* LiveChatFloatingViewController::AddText(Transform* parent, const std::string& value, Color color, float fontSize, Vector2 anchoredPosition, Vector2 sizeDelta, TMPro::TextAlignmentOptions alignment, bool wordWrapping)
    {
        auto textObject = GameObject::New_ctor("Text");
        auto text = textObject->AddComponent<TMPro::TextMeshProUGUI*>();
        text->rectTransform->SetParent(parent, false);
        ConfigureText(text, color, fontSize, alignment, wordWrapping);
        text->text = value;

        auto rect = text->rectTransform;
        rect->anchorMin = {0.0f, 1.0f};
        rect->anchorMax = {0.0f, 1.0f};
        rect->pivot = {0.0f, 1.0f};
        rect->anchoredPosition = anchoredPosition;
        rect->sizeDelta = sizeDelta;
        return text;
    }

    void LiveChatFloatingViewController::ConfigureText(TMPro::TextMeshProUGUI* text, Color color, float fontSize, TMPro::TextAlignmentOptions alignment, bool wordWrapping)
    {
        text->font = ChatFont();
        text->richText = true;
        text->enableWordWrapping = wordWrapping;
        text->overflowMode = TMPro::TextOverflowModes::Ellipsis;
        text->alignment = alignment;
        text->color = color;
        text->fontSize = fontSize;
        text->lineSpacing = TextLineSpacing;
        text->raycastTarget = false;
    }

    TMPro::TMP_FontAsset* LiveChatFloatingViewController::ChatFont()
    {
        if (_chatFont)
        {
            return _chatFont;
        }

        TMPro::TMP_FontAsset* fallback = nullptr;
        TMPro::TMP_FontAsset* teko = nullptr;
        for (auto font : Resources::FindObjectsOfTypeAll<TMPro::TMP_FontAsset*>())
        {
            if (!font)
            {
                continue;
            }
            if (font->name == "Teko-Medium SDF No Glow")
            {
                _chatFont = font;
                return _chatFont;
            }
            if (!teko && font->name == "Teko-Medium SDF")
            {
                teko = font;
            }
            if (!fallback)
            {
                fallback = font;
            }
        }

        _chatFont = teko ? teko : fallback;
        return _chatFont;
    }
}
