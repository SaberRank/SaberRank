#pragma once

#include "Features/Players/Services/PlayerService.hpp"
#include "Features/Players/Domain/GameSession.hpp"
#include "Features/Players/Domain/LocalPlayerInfo.hpp"
#include "Features/Players/Domain/Player.hpp"

#include <System/zzzz__Object_def.hpp>
#include <chrono>
#include <cstdint>
#include <custom-types/shared/macros.hpp>
#include <functional>
#include <lapiz/shared/macros.hpp>
#include <optional>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Players::Services, GameSessionService, System::Object) {
    DECLARE_CTOR(ctor);

  public:
    using LoginStatus = SnoreSaber::Services::PlayerService::LoginStatus;
    using LoginCallback = std::function<void(LoginStatus)>;

    LoginStatus GetStatus() const;
    std::string GetLocalPlayerId() const;
    StringW GetLocalPlayerName() const;
    std::string GetPlayerKey() const;
    std::optional<SnoreSaber::Data::GameSession> GetGameSession() const;
    std::optional<SnoreSaber::Data::LocalPlayerInfo> GetLocalPlayerInfo() const;
    bool HasAuthenticatedSession() const;
    std::chrono::system_clock::time_point LastAuthenticatedAtUtc() const;
    // PC multicast LoginStatusChanged; handlers invoked on the main thread
    uint64_t AddLoginStatusChangedHandler(std::function<void(LoginStatus, const std::string&)> handler);
    void RemoveLoginStatusChangedHandler(uint64_t token);
    void EnsureAuthenticated(bool forceRefresh, LoginCallback finished);
    bool RefreshGameSession();
    bool RefreshUploadTrust();
    void GetPlayerInfo(std::string playerId, bool full, std::function<void(std::optional<SnoreSaber::Data::Player>)> finished);
    void UpdatePlayerInfo();
    void UpdatePlayerInfoThread();
    void OnSoftRestart();
};
