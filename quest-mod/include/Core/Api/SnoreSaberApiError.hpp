#pragma once

#include <string>
#include <utility>

namespace SnoreSaber::Core::Api
{
    struct SnoreSaberApiError
    {
        long statusCode = 0;
        bool networkError = false;
        std::string code;
        std::string message;
        std::string rawBody;

        static SnoreSaberApiError FromMessage(std::string message)
        {
            SnoreSaberApiError error;
            error.message = std::move(message);
            return error;
        }
    };
}
