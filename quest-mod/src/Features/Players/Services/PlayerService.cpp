#include "Features/Players/Services/PlayerService.hpp"

#include "Features/Players/Services/GameSessionService.hpp"

#include <bsml/shared/Helpers/getters.hpp>
#include <utility>

namespace SnoreSaber::Services::PlayerService
{
    namespace
    {
        SnoreSaber::Features::Players::Services::GameSessionService* GetGameSessionService()
        {
            return BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Features::Players::Services::GameSessionService*>();
        }
    }

    void AuthenticateUser(std::function<void(LoginStatus)> finished)
    {
        auto gameSessionService = GetGameSessionService();
        if (gameSessionService)
        {
            gameSessionService->EnsureAuthenticated(false, std::move(finished));
        }
        else if (finished)
        {
            finished(LoginStatus::Error);
        }
    }

    void UpdatePlayerInfo()
    {
        auto gameSessionService = GetGameSessionService();
        if (gameSessionService)
        {
            gameSessionService->UpdatePlayerInfo();
        }
    }

    void OnSoftRestart()
    {
        auto gameSessionService = GetGameSessionService();
        if (gameSessionService)
        {
            gameSessionService->OnSoftRestart();
        }
    }

    std::string GetLocalPlayerId()
    {
        auto gameSessionService = GetGameSessionService();
        return gameSessionService ? gameSessionService->GetLocalPlayerId() : "";
    }

    StringW GetLocalPlayerName()
    {
        auto gameSessionService = GetGameSessionService();
        return gameSessionService ? gameSessionService->GetLocalPlayerName() : "";
    }

} // namespace SnoreSaber::Services::PlayerService
