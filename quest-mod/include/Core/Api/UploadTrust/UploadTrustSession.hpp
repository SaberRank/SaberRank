#pragma once

#include <string>

namespace SnoreSaber::Core::Api::UploadTrust
{
    struct UploadTrustSession
    {
        static constexpr const char* ProtocolHeaderValue = "snoresaber-upload-v2";
        static constexpr int ProtocolVersion = 2;

        std::string buildId;
        std::string buildCredential;
        std::string uploadVersionHash;
        bool requiresBuildId = false;

        bool IsUploadProtocolV2() const;
    };
}
