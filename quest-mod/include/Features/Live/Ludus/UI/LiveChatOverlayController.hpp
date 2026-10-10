#pragma once

#include "Core/Configuration/SettingsService.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayState.hpp"
#include "Features/Live/Ludus/Domain/LiveChatEntry.hpp"
#include "Features/Live/Ludus/Services/LudusSessionService.hpp"
#include "Features/Live/Ludus/UI/LiveChatFloatingViewController.hpp"

#include <System/IDisposable.hpp>
#include <UnityEngine/Vector3.hpp>
#include <Zenject/IInitializable.hpp>
#include <Zenject/ITickable.hpp>
#include <bsml/shared/BSML/FloatingScreen/FloatingScreen.hpp>
#include <custom-types/shared/macros.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

DECLARE_CLASS_CODEGEN_INTERFACES(
    SnoreSaber::Features::Live::Ludus::UI,
    LiveChatOverlayController,
    System::Object,
    Zenject::IInitializable*,
    Zenject::ITickable*,
    System::IDisposable*) {

    DECLARE_INSTANCE_FIELD_PRIVATE(Core::Configuration::SettingsService*, _settings);
    DECLARE_INSTANCE_FIELD_PRIVATE(Services::LudusSessionService*, _ludusSession);
    DECLARE_INSTANCE_FIELD_PRIVATE(Compete::Services::CompeteGameplayState*, _competeGameplayState);
    DECLARE_INSTANCE_FIELD_PRIVATE(LiveChatFloatingViewController*, _viewController);
    DECLARE_INSTANCE_FIELD_PRIVATE(BSML::FloatingScreen*, _screen);

    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Tick, &::Zenject::ITickable::Tick);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);

    DECLARE_CTOR(ctor,
                 Core::Configuration::SettingsService* settings,
                 Services::LudusSessionService* ludusSession,
                 Compete::Services::CompeteGameplayState* competeGameplayState,
                 LiveChatFloatingViewController* viewController);

  private:
    UnityEngine::Vector3 _baseScale = {1.0f, 1.0f, 1.0f};
    float _appliedOverlayScale = -1.0f;
    std::vector<Domain::LiveChatEntry> _pendingMessages;
    std::optional<std::string> _pendingStatus;
    int _pendingViewerCount = INT_MIN;
    bool _hasChatMessages = false;
    bool _visible = false;
    bool _messagesDirty = false;
    bool _statusDirty = false;
    bool _viewerCountDirty = false;
    uint64_t _chatMessagesChangedToken = 0;
    uint64_t _statusChangedToken = 0;
    uint64_t _viewerListUpdatedToken = 0;
    uint64_t _playerFollowRequestedToken = 0;

    void OnChatMessagesChanged(const std::vector<Domain::LiveChatEntry>& messages);
    void OnStatusChanged(const std::string& status);
    void StoreMessages(const std::vector<Domain::LiveChatEntry>& messages);
    void StoreViewerCount(int viewerCount);
    void FlushViewStateIfVisible();
    void ApplyOverlayScale(bool force);
    void ApplyVisibility(bool animated);
    bool ShouldShowOverlay();
    bool IsGameplaySceneActive();
    void StyleFloatingScreen();
    UnityEngine::Material* CreateNoGlowMaterial(UnityEngine::Color color);
};
