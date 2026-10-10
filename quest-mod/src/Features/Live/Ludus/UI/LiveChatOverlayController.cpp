#include "Features/Live/Ludus/UI/LiveChatOverlayController.hpp"

#include "logging.hpp"

#include <UnityEngine/Canvas.hpp>
#include <UnityEngine/Color.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Material.hpp>
#include <UnityEngine/Quaternion.hpp>
#include <UnityEngine/Renderer.hpp>
#include <UnityEngine/SceneManagement/Scene.hpp>
#include <UnityEngine/SceneManagement/SceneManager.hpp>
#include <UnityEngine/Shader.hpp>
#include <UnityEngine/Transform.hpp>
#include <UnityEngine/UI/Image.hpp>
#include <bsml/shared/Helpers/getters.hpp>

#include <algorithm>

using namespace UnityEngine;

DEFINE_TYPE(SnoreSaber::Features::Live::Ludus::UI, LiveChatOverlayController);

namespace SnoreSaber::Features::Live::Ludus::UI
{
    void LiveChatOverlayController::ctor(Core::Configuration::SettingsService* settings,
                                         Services::LudusSessionService* ludusSession,
                                         Compete::Services::CompeteGameplayState* competeGameplayState,
                                         LiveChatFloatingViewController* viewController)
    {
        INVOKE_CTOR();
        _settings = settings;
        _ludusSession = ludusSession;
        _competeGameplayState = competeGameplayState;
        _viewController = viewController;
    }

    void LiveChatOverlayController::Initialize()
    {
        _screen = BSML::FloatingScreen::CreateFloatingScreen(
            {LiveChatFloatingViewController::ChatWidth, LiveChatFloatingViewController::ChatHeight},
            true, {0.0f, 3.75f, 2.5f}, Quaternion::Euler(325.0f, 0.0f, 0.0f));
        _screen->gameObject->name = "SnoreSaber Live Chat Overlay";
        UnityEngine::Object::DontDestroyOnLoad(_screen->gameObject);

        auto canvas = _screen->GetComponent<Canvas*>();
        if (canvas)
        {
            canvas->sortingOrder = 33;
        }

        _baseScale = _screen->transform->localScale;
        StyleFloatingScreen();
        ApplyOverlayScale(true);
        _screen->gameObject->SetActive(false);

        _chatMessagesChangedToken = _ludusSession->ChatMessagesChanged.Add([this](const std::vector<Domain::LiveChatEntry>& messages) { OnChatMessagesChanged(messages); });
        _statusChangedToken = _ludusSession->StatusChanged.Add([this](const std::string& status) { OnStatusChanged(status); });
        _viewerListUpdatedToken = _ludusSession->ViewerListUpdated.Add([this](const std::vector<::SnoreSaber::Live::V1::LiveRoomViewerState>& viewers) { StoreViewerCount(static_cast<int>(viewers.size())); });
        _playerFollowRequestedToken = _ludusSession->PlayerFollowRequested.Add([this](int) { StoreViewerCount(_ludusSession->CurrentViewerCount()); });

        StoreMessages(_ludusSession->CurrentChatMessages());
        StoreViewerCount(_ludusSession->CurrentViewerCount());

        INFO("Live chat overlay initialized. enabled={} connected={}", _settings->LiveChatOverlayEnabled(), _ludusSession->IsConnectedToLudus());
        ApplyVisibility(false);
    }

    void LiveChatOverlayController::Tick()
    {
        ApplyOverlayScale(false);
        ApplyVisibility(true);
        if (!_visible)
        {
            return;
        }

        _viewController->RefreshLayoutSettings();
        StoreViewerCount(_ludusSession->CurrentViewerCount());
        FlushViewStateIfVisible();
    }

    void LiveChatOverlayController::Dispose()
    {
        _ludusSession->ChatMessagesChanged.Remove(_chatMessagesChangedToken);
        _ludusSession->StatusChanged.Remove(_statusChangedToken);
        _ludusSession->ViewerListUpdated.Remove(_viewerListUpdatedToken);
        _ludusSession->PlayerFollowRequested.Remove(_playerFollowRequestedToken);

        if (_screen)
        {
            UnityEngine::Object::Destroy(_screen->gameObject);
            _screen = nullptr;
        }
    }

    void LiveChatOverlayController::OnChatMessagesChanged(const std::vector<Domain::LiveChatEntry>& messages)
    {
        StoreMessages(messages);
        ApplyVisibility(true);
    }

    void LiveChatOverlayController::OnStatusChanged(const std::string& status)
    {
        if (_pendingStatus.has_value() && *_pendingStatus == status)
        {
            return;
        }

        _pendingStatus = status;
        _statusDirty = true;
        FlushViewStateIfVisible();
    }

    void LiveChatOverlayController::StoreMessages(const std::vector<Domain::LiveChatEntry>& messages)
    {
        _pendingMessages = messages;
        _hasChatMessages = std::any_of(messages.begin(), messages.end(), [](const Domain::LiveChatEntry& entry) { return entry.IsChat(); });
        _messagesDirty = true;
        FlushViewStateIfVisible();
    }

    void LiveChatOverlayController::StoreViewerCount(int viewerCount)
    {
        int nextViewerCount = _ludusSession->IsInPublicPresence() ? viewerCount : -1;
        if (_pendingViewerCount == nextViewerCount)
        {
            return;
        }

        _pendingViewerCount = nextViewerCount;
        _viewerCountDirty = true;
        FlushViewStateIfVisible();
    }

    void LiveChatOverlayController::FlushViewStateIfVisible()
    {
        if (!_visible)
        {
            return;
        }

        if (_messagesDirty)
        {
            _messagesDirty = false;
            _viewController->SetMessages(_pendingMessages);
        }

        if (_statusDirty)
        {
            _statusDirty = false;
            _viewController->SetStatus(_pendingStatus.value_or(""));
        }

        if (_viewerCountDirty)
        {
            _viewerCountDirty = false;
            _viewController->SetViewerCount(_pendingViewerCount);
        }
    }

    void LiveChatOverlayController::ApplyOverlayScale(bool force)
    {
        float nextScale = std::clamp(_settings->LiveChatOverlayScale(), 0.85f, 1.75f);
        if (!force && std::abs(_appliedOverlayScale - nextScale) < 0.001f)
        {
            return;
        }

        _appliedOverlayScale = nextScale;
        if (_screen)
        {
            _screen->transform->localScale = Vector3::op_Multiply(_baseScale, nextScale);
        }
    }

    void LiveChatOverlayController::ApplyVisibility(bool animated)
    {
        bool shouldShow = ShouldShowOverlay();
        if (_visible == shouldShow)
        {
            if (_screen && _screen->gameObject->activeSelf != shouldShow)
            {
                _screen->gameObject->SetActive(shouldShow);
            }

            return;
        }

        _visible = shouldShow;
        if (!_screen)
        {
            return;
        }

        if (shouldShow)
        {
            _screen->gameObject->SetActive(true);
            FlushViewStateIfVisible();
            _screen->SetRootViewController(_viewController, animated ? HMUI::ViewController::AnimationType::In : HMUI::ViewController::AnimationType::None);
            _viewController->ResumeStatusAutoClear();
            INFO("Live chat overlay shown. connected={}", _ludusSession->IsConnectedToLudus());
        }
        else
        {
            _screen->SetRootViewController(nullptr, animated ? HMUI::ViewController::AnimationType::Out : HMUI::ViewController::AnimationType::None);
            _screen->gameObject->SetActive(false);
            INFO("Live chat overlay hidden.");
        }
    }

    bool LiveChatOverlayController::ShouldShowOverlay()
    {
        if (!_settings->LiveChatOverlayEnabled())
        {
            return false;
        }

        if (_competeGameplayState->IsLiveGameplayActive())
        {
            return false;
        }

        bool gameplaySceneActive = IsGameplaySceneActive();
        if (_ludusSession->IsInTournamentRoom())
        {
            return !gameplaySceneActive;
        }

        if (!_hasChatMessages)
        {
            return false;
        }

        return !gameplaySceneActive || _settings->LiveChatOverlayGameplayEnabled();
    }

    bool LiveChatOverlayController::IsGameplaySceneActive()
    {
        int sceneCount = SceneManagement::SceneManager::get_sceneCount();
        for (int i = 0; i < sceneCount; i++)
        {
            auto scene = SceneManagement::SceneManager::GetSceneAt(i);
            if (scene.get_isLoaded() && scene.get_name() == "GameCore")
            {
                return true;
            }
        }

        return SceneManagement::SceneManager::GetActiveScene().get_name() == "GameCore";
    }

    void LiveChatOverlayController::StyleFloatingScreen()
    {
        auto screenImage = _screen->GetComponent<UnityEngine::UI::Image*>();
        if (screenImage)
        {
            screenImage->material = CreateNoGlowMaterial(Color::get_white());
            screenImage->color = {0.0f, 0.0f, 0.0f, 0.5f};
            screenImage->raycastTarget = false;
        }

        auto handle = _screen->handle;
        if (handle)
        {
            handle->transform->localScale = {8.0f, LiveChatFloatingViewController::ChatHeight, 0.0f};
            handle->transform->localPosition = {-65.0f, 0.0f, 0.0f};
            handle->transform->localRotation = Quaternion::get_identity();

            auto handleRenderer = handle->GetComponent<Renderer*>();
            if (handleRenderer)
            {
                handleRenderer->material = CreateNoGlowMaterial(Color::get_clear());
            }
        }
    }

    Material* LiveChatOverlayController::CreateNoGlowMaterial(Color color)
    {
        auto noGlow = BSML::Helpers::GetUINoGlowMat();
        auto material = noGlow ? UnityEngine::Object::Instantiate(noGlow) : Material::New_ctor(Shader::Find("UI/Default"));
        material->color = color;
        return material;
    }
}
