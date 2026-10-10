#pragma once

#include "Core/Api/UploadTrust/UploadTrustSession.hpp"

#include <optional>
#include <string>
#include <vector>

namespace SnoreSaber::Core::Api::UploadTrust
{
    struct UploadTrustHeaders
    {
        std::optional<std::string> uploadVersionHash;
        std::optional<std::string> uploadSignature;
        std::optional<std::string> replaySha256;
        std::optional<std::string> uploadNonce;
        std::optional<std::string> uploadTimestamp;
        std::optional<std::string> clientBuildId;
        std::optional<std::string> uploadProtocol;
    };

    class UploadTrustHeaderBuilder
    {
      public:
        static UploadTrustHeaders BuildUploadHeaders(const std::string& sessionId, const std::string& playerId, const std::string& uploadVersionHash,
                                                     const std::string& encryptedData, const std::vector<char>& replay, const UploadTrustSession& trust);

        static UploadTrustHeaders BuildUploadHeaders(const std::string& sessionId, const std::string& playerId, const std::string& uploadVersionHash,
                                                     const std::string& encryptedData, const std::vector<char>& replay, const UploadTrustSession& trust,
                                                     long timestamp, std::string nonce);

        static std::string BuildCanonicalString(const std::string& buildId, const std::string& sessionId, const std::string& playerId,
                                                const std::string& uploadVersionHash, const std::string& encryptedDataSha256,
                                                const std::string& replaySha256, const std::string& timestamp, const std::string& nonce);

        static std::string Sha256Hex(const std::string& value);
        static std::string Sha256Hex(const std::vector<char>& value);
        static std::string HmacSha256Hex(const std::string& credential, const std::string& canonicalString);
    };
}
