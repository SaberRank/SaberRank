#pragma once

#include <string>

namespace SnoreSaber::Core::Api::UploadTrust
{
    struct UploadTrustBuildMetadata
    {
        std::string buildId;
        std::string credential;
        std::string artifactSha256;
        std::string developmentUploadToken;
        std::string developmentAuthNonce;
        std::string developmentPlayerId;
        std::string developmentPlayerName;

        bool IsOfficial() const;
        bool IsDevelopment() const;
        bool HasDevelopmentAuth() const;

        static UploadTrustBuildMetadata FromBuildConfig();
    };
}
