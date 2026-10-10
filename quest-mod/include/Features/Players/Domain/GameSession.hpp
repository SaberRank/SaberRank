#pragma once

#include "Core/Api/UploadTrust/UploadTrustSession.hpp"

#include <optional>
#include <string>

namespace SnoreSaber::Data
{
    struct GameSession
    {
        std::string playerId;
        std::string playerName;
        std::string sessionId;
        std::string sessionKey;
        std::optional<SnoreSaber::Core::Api::UploadTrust::UploadTrustSession> uploadTrust;

        bool IsAuthenticated() const;
        bool UsesUploadProtocolV2() const;
    };
}
