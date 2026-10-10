#include "Features/Players/Services/PlayerProfileService.hpp"

#include "Core/Api/SnoreSaberApiClient.hpp"
#include "Utils/AsyncUtils.hpp"
#include "Utils/SafePtr.hpp"
#include "logging.hpp"

#include <utility>

DEFINE_TYPE(SnoreSaber::Features::Players::Services, PlayerProfileService);

namespace SnoreSaber::Features::Players::Services
{
    void PlayerProfileService::ctor()
    {
        INVOKE_CTOR();
    }

    std::optional<SnoreSaber::Data::Player> PlayerProfileService::GetPlayerInfo(const std::string& playerId, bool full)
    {
        if (playerId.empty())
        {
            return std::nullopt;
        }

        try
        {
            Core::Api::SnoreSaberApiClient apiClient;
            return apiClient.GetPlayerProfile(playerId, full);
        }
        catch (const std::exception& exception)
        {
            ERROR("Failed to load SnoreSaber player profile: {:s}", exception.what());
            return std::nullopt;
        }
    }

    void PlayerProfileService::GetPlayerInfoAsync(std::string playerId, bool full, std::function<void(std::optional<SnoreSaber::Data::Player>)> finished)
    {
        if (!finished)
            return;

        SafePtr<PlayerProfileService> self(this);
        SnoreSaber::Utils::Async::RunThenMain(
            [self, playerId = std::move(playerId), full] {
                return self->GetPlayerInfo(playerId, full);
            },
            [finished = std::move(finished)](std::optional<SnoreSaber::Data::Player> player) {
                finished(player);
            });
    }
}
