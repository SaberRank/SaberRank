#include "Core/Api/UploadTrust/UploadTrustClient.hpp"

#include "Core/SnoreSaberRuntimeInfo.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <utility>

namespace SnoreSaber::Core::Api::UploadTrust
{
    namespace
    {
        std::string ToLower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
            return value;
        }

        bool EqualsIgnoreCase(const std::string& lhs, const std::string& rhs)
        {
            return ToLower(lhs) == ToLower(rhs);
        }
    }

    UploadTrustClient::UploadTrustClient()
        : UploadTrustClient(UploadTrustBuildMetadata::FromBuildConfig())
    {
    }

    UploadTrustClient::UploadTrustClient(UploadTrustBuildMetadata buildMetadata)
        : _buildMetadata(std::move(buildMetadata)), _uploadVersionHash(SnoreSaber::Core::SnoreSaberRuntimeInfo::BuildUploadVersionHash())
    {
    }

    void UploadTrustClient::ApplyAuthMetadata(Generated::GameAuthenticateRequest& request) const
    {
        if (!_buildMetadata.IsOfficial() && !_buildMetadata.IsDevelopment())
        {
            return;
        }

        request.ClientKind = _buildMetadata.IsOfficial() ? "official" : "development";
        request.UploadProtocolVersion = UploadTrustSession::ProtocolVersion;
        if (_buildMetadata.IsOfficial())
        {
            request.ClientBuildId = _buildMetadata.buildId;
        }
        request.PluginVersion = SnoreSaber::Core::SnoreSaberRuntimeInfo::PluginVersion();
        request.GameVersion = SnoreSaber::Core::SnoreSaberRuntimeInfo::GameVersion();
        request.UploadVersionHash = _uploadVersionHash;
        request.ArtifactSha256 = _buildMetadata.artifactSha256;
        request.DevUploadToken = _buildMetadata.developmentUploadToken;
    }

    std::optional<UploadTrustSession> UploadTrustClient::CreateSession(const Generated::GameAuthenticateResponse& response) const
    {
        if (!_buildMetadata.IsOfficial() && !_buildMetadata.IsDevelopment())
        {
            return std::nullopt;
        }

        if (!std::isfinite(response.UploadProtocolVersion) || response.UploadProtocolVersion != static_cast<double>(UploadTrustSession::ProtocolVersion))
        {
            return std::nullopt;
        }

        if (_buildMetadata.IsOfficial() && response.BuildId.empty())
        {
            return std::nullopt;
        }

        if (response.UploadVersionHash.empty())
        {
            return std::nullopt;
        }

        bool trustedOfficial = _buildMetadata.IsOfficial() &&
                               EqualsIgnoreCase(response.ClientTrust, "official") &&
                               response.BuildId == _buildMetadata.buildId;
        bool trustedDevelopment = _buildMetadata.IsDevelopment() &&
                                  EqualsIgnoreCase(response.ClientTrust, "development");

        if (!trustedOfficial && !trustedDevelopment)
        {
            return std::nullopt;
        }

        if (!EqualsIgnoreCase(response.UploadVersionHash, _uploadVersionHash))
        {
            return std::nullopt;
        }

        return UploadTrustSession {
            response.BuildId,
            _buildMetadata.IsOfficial() ? _buildMetadata.credential : _buildMetadata.developmentUploadToken,
            response.UploadVersionHash,
            _buildMetadata.IsOfficial(),
        };
    }
}
