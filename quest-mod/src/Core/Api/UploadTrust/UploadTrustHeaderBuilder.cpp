#include "Core/Api/UploadTrust/UploadTrustHeaderBuilder.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace SnoreSaber::Core::Api::UploadTrust
{
    namespace
    {
        constexpr std::array<uint32_t, 64> Sha256K = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
        };

        uint32_t RotateRight(uint32_t value, uint32_t bits)
        {
            return (value >> bits) | (value << (32 - bits));
        }

        std::array<uint8_t, 32> Sha256Bytes(const uint8_t* data, size_t size)
        {
            std::vector<uint8_t> message;
            if (size > 0)
            {
                message.assign(data, data + size);
            }
            uint64_t bitLength = static_cast<uint64_t>(message.size()) * 8;
            message.push_back(0x80);
            while ((message.size() % 64) != 56)
            {
                message.push_back(0);
            }
            for (int i = 7; i >= 0; --i)
            {
                message.push_back(static_cast<uint8_t>((bitLength >> (i * 8)) & 0xff));
            }

            uint32_t h0 = 0x6a09e667;
            uint32_t h1 = 0xbb67ae85;
            uint32_t h2 = 0x3c6ef372;
            uint32_t h3 = 0xa54ff53a;
            uint32_t h4 = 0x510e527f;
            uint32_t h5 = 0x9b05688c;
            uint32_t h6 = 0x1f83d9ab;
            uint32_t h7 = 0x5be0cd19;

            for (size_t chunk = 0; chunk < message.size(); chunk += 64)
            {
                std::array<uint32_t, 64> w{};
                for (int i = 0; i < 16; ++i)
                {
                    size_t offset = chunk + i * 4;
                    w[i] = (static_cast<uint32_t>(message[offset]) << 24) |
                           (static_cast<uint32_t>(message[offset + 1]) << 16) |
                           (static_cast<uint32_t>(message[offset + 2]) << 8) |
                           static_cast<uint32_t>(message[offset + 3]);
                }
                for (int i = 16; i < 64; ++i)
                {
                    uint32_t s0 = RotateRight(w[i - 15], 7) ^ RotateRight(w[i - 15], 18) ^ (w[i - 15] >> 3);
                    uint32_t s1 = RotateRight(w[i - 2], 17) ^ RotateRight(w[i - 2], 19) ^ (w[i - 2] >> 10);
                    w[i] = w[i - 16] + s0 + w[i - 7] + s1;
                }

                uint32_t a = h0;
                uint32_t b = h1;
                uint32_t c = h2;
                uint32_t d = h3;
                uint32_t e = h4;
                uint32_t f = h5;
                uint32_t g = h6;
                uint32_t h = h7;

                for (int i = 0; i < 64; ++i)
                {
                    uint32_t s1 = RotateRight(e, 6) ^ RotateRight(e, 11) ^ RotateRight(e, 25);
                    uint32_t ch = (e & f) ^ (~e & g);
                    uint32_t temp1 = h + s1 + ch + Sha256K[i] + w[i];
                    uint32_t s0 = RotateRight(a, 2) ^ RotateRight(a, 13) ^ RotateRight(a, 22);
                    uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                    uint32_t temp2 = s0 + maj;

                    h = g;
                    g = f;
                    f = e;
                    e = d + temp1;
                    d = c;
                    c = b;
                    b = a;
                    a = temp1 + temp2;
                }

                h0 += a;
                h1 += b;
                h2 += c;
                h3 += d;
                h4 += e;
                h5 += f;
                h6 += g;
                h7 += h;
            }

            std::array<uint8_t, 32> digest{};
            std::array<uint32_t, 8> words = {h0, h1, h2, h3, h4, h5, h6, h7};
            for (size_t i = 0; i < words.size(); ++i)
            {
                digest[i * 4] = static_cast<uint8_t>((words[i] >> 24) & 0xff);
                digest[i * 4 + 1] = static_cast<uint8_t>((words[i] >> 16) & 0xff);
                digest[i * 4 + 2] = static_cast<uint8_t>((words[i] >> 8) & 0xff);
                digest[i * 4 + 3] = static_cast<uint8_t>(words[i] & 0xff);
            }
            return digest;
        }

        std::string ToLowerHex(const uint8_t* bytes, size_t size)
        {
            std::ostringstream result;
            result << std::hex << std::setfill('0');
            for (size_t i = 0; i < size; ++i)
            {
                result << std::setw(2) << static_cast<int>(bytes[i]);
            }
            return result.str();
        }

        long CurrentEpochSeconds()
        {
            return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }

        std::string CreateNonce()
        {
            std::array<uint8_t, 16> bytes{};
            std::random_device randomDevice;
            for (auto& byte : bytes)
            {
                byte = static_cast<uint8_t>(randomDevice());
            }
            return ToLowerHex(bytes.data(), bytes.size());
        }
    }

    UploadTrustHeaders UploadTrustHeaderBuilder::BuildUploadHeaders(const std::string& sessionId, const std::string& playerId, const std::string& uploadVersionHash,
                                                                    const std::string& encryptedData, const std::vector<char>& replay,
                                                                    const UploadTrustSession& trust)
    {
        return BuildUploadHeaders(sessionId, playerId, uploadVersionHash, encryptedData, replay, trust, CurrentEpochSeconds(), CreateNonce());
    }

    UploadTrustHeaders UploadTrustHeaderBuilder::BuildUploadHeaders(const std::string& sessionId, const std::string& playerId, const std::string& uploadVersionHash,
                                                                    const std::string& encryptedData, const std::vector<char>& replay,
                                                                    const UploadTrustSession& trust, long timestamp, std::string nonce)
    {
        if (!trust.IsUploadProtocolV2())
        {
            return {};
        }

        if (uploadVersionHash != trust.uploadVersionHash)
        {
            throw std::invalid_argument("Upload version hash did not match the authenticated upload trust session.");
        }

        std::string dataSha256 = Sha256Hex(encryptedData);
        std::string replaySha256 = Sha256Hex(replay);
        std::string timestampText = std::to_string(timestamp);
        std::string canonicalString = BuildCanonicalString(
            trust.buildId,
            sessionId,
            playerId,
            uploadVersionHash,
            dataSha256,
            replaySha256,
            timestampText,
            nonce);

        UploadTrustHeaders headers;
        headers.uploadProtocol = UploadTrustSession::ProtocolHeaderValue;
        if (!trust.buildId.empty())
        {
            headers.clientBuildId = trust.buildId;
        }
        headers.uploadTimestamp = timestampText;
        headers.uploadNonce = std::move(nonce);
        headers.replaySha256 = replaySha256;
        headers.uploadVersionHash = uploadVersionHash;
        headers.uploadSignature = HmacSha256Hex(trust.buildCredential, canonicalString);
        return headers;
    }

    std::string UploadTrustHeaderBuilder::BuildCanonicalString(const std::string& buildId, const std::string& sessionId, const std::string& playerId,
                                                               const std::string& uploadVersionHash, const std::string& encryptedDataSha256,
                                                               const std::string& replaySha256, const std::string& timestamp, const std::string& nonce)
    {
        return std::string(UploadTrustSession::ProtocolHeaderValue) + "\n" +
               buildId + "\n" +
               sessionId + "\n" +
               playerId + "\n" +
               uploadVersionHash + "\n" +
               encryptedDataSha256 + "\n" +
               replaySha256 + "\n" +
               timestamp + "\n" +
               nonce;
    }

    std::string UploadTrustHeaderBuilder::Sha256Hex(const std::string& value)
    {
        auto digest = Sha256Bytes(reinterpret_cast<const uint8_t*>(value.data()), value.size());
        return ToLowerHex(digest.data(), digest.size());
    }

    std::string UploadTrustHeaderBuilder::Sha256Hex(const std::vector<char>& value)
    {
        auto digest = Sha256Bytes(reinterpret_cast<const uint8_t*>(value.data()), value.size());
        return ToLowerHex(digest.data(), digest.size());
    }

    std::string UploadTrustHeaderBuilder::HmacSha256Hex(const std::string& credential, const std::string& canonicalString)
    {
        constexpr size_t BlockSize = 64;
        std::vector<uint8_t> key(credential.begin(), credential.end());
        if (key.size() > BlockSize)
        {
            auto digest = Sha256Bytes(key.data(), key.size());
            key.assign(digest.begin(), digest.end());
        }
        key.resize(BlockSize, 0);

        std::array<uint8_t, BlockSize> outerPad{};
        std::array<uint8_t, BlockSize> innerPad{};
        for (size_t i = 0; i < BlockSize; ++i)
        {
            outerPad[i] = key[i] ^ 0x5c;
            innerPad[i] = key[i] ^ 0x36;
        }

        std::vector<uint8_t> inner(innerPad.begin(), innerPad.end());
        inner.insert(inner.end(), canonicalString.begin(), canonicalString.end());
        auto innerDigest = Sha256Bytes(inner.data(), inner.size());

        std::vector<uint8_t> outer(outerPad.begin(), outerPad.end());
        outer.insert(outer.end(), innerDigest.begin(), innerDigest.end());
        auto digest = Sha256Bytes(outer.data(), outer.size());
        return ToLowerHex(digest.data(), digest.size());
    }
}
