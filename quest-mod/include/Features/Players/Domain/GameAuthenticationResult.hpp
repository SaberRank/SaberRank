#pragma once

#include "Core/Api/SnoreSaberApiError.hpp"
#include "Features/Players/Domain/GameSession.hpp"

#include <optional>
#include <string>
#include <utility>

namespace SnoreSaber::Data
{
    enum class GameSessionStatus
    {
        None,
        InProgress,
        Success,
        Error,
    };

    struct GameAuthenticationResult
    {
        GameSessionStatus status = GameSessionStatus::None;
        std::optional<GameSession> session;
        std::string message;
        std::optional<SnoreSaber::Core::Api::SnoreSaberApiError> error;

        static GameAuthenticationResult Success(GameSession session)
        {
            GameAuthenticationResult result;
            result.status = GameSessionStatus::Success;
            result.session = std::move(session);
            result.message = "Authenticated";
            return result;
        }

        static GameAuthenticationResult Failure(std::string message, std::optional<SnoreSaber::Core::Api::SnoreSaberApiError> error = std::nullopt)
        {
            GameAuthenticationResult result;
            result.status = GameSessionStatus::Error;
            result.message = std::move(message);
            result.error = std::move(error);
            return result;
        }
    };
}
