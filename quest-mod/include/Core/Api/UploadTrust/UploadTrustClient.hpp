#pragma once

#include "Core/Api/Generated/SnoreSaberApiGeneratedClient.hpp"
#include "Core/Api/UploadTrust/UploadTrustBuildMetadata.hpp"
#include "Core/Api/UploadTrust/UploadTrustSession.hpp"

#include <optional>
#include <string>

namespace SnoreSaber::Core::Api::UploadTrust
{
    class UploadTrustClient
    {
      public:
        UploadTrustClient();
        explicit UploadTrustClient(UploadTrustBuildMetadata buildMetadata);

        void ApplyAuthMetadata(Generated::GameAuthenticateRequest& request) const;
        std::optional<UploadTrustSession> CreateSession(const Generated::GameAuthenticateResponse& response) const;

      private:
        UploadTrustBuildMetadata _buildMetadata;
        std::string _uploadVersionHash;
    };
}
