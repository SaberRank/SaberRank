
#pragma once
#include <beatsaber-hook/shared/utils/typedefs.h>
#include <functional>
#include <string>
namespace SnoreSaber::Services::PlayerService
{
    enum LoginStatus
    {
        None = 0,
        InProgress = 1,
        Error = 2,
        Success = 3,
    };

    void AuthenticateUser(std::function<void(LoginStatus)> finished);
    void UpdatePlayerInfo();
    void OnSoftRestart();
    std::string GetLocalPlayerId();
    StringW GetLocalPlayerName();

} // namespace SnoreSaber::Services::PlayerService
