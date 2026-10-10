#include "Features/Live/Transport/WebSocketFraming.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <random>
#include <string_view>

namespace SnoreSaber::Features::Live::Transport::WebSocketFraming
{
    namespace
    {
        // tiny sha1 (rfc 3174); only used to derive the handshake accept key
        struct Sha1
        {
            uint32_t State[5] = {0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};
            uint64_t TotalBytes = 0;
            uint8_t Block[64] = {};
            size_t BlockSize = 0;

            static uint32_t RotateLeft(uint32_t value, int bits)
            {
                return (value << bits) | (value >> (32 - bits));
            }

            void ProcessBlock()
            {
                uint32_t words[80];
                for (int i = 0; i < 16; i++)
                {
                    words[i] = (static_cast<uint32_t>(Block[i * 4]) << 24) |
                               (static_cast<uint32_t>(Block[i * 4 + 1]) << 16) |
                               (static_cast<uint32_t>(Block[i * 4 + 2]) << 8) |
                               static_cast<uint32_t>(Block[i * 4 + 3]);
                }
                for (int i = 16; i < 80; i++)
                {
                    words[i] = RotateLeft(words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16], 1);
                }

                uint32_t a = State[0], b = State[1], c = State[2], d = State[3], e = State[4];
                for (int i = 0; i < 80; i++)
                {
                    uint32_t f;
                    uint32_t k;
                    if (i < 20)
                    {
                        f = (b & c) | (~b & d);
                        k = 0x5A827999u;
                    }
                    else if (i < 40)
                    {
                        f = b ^ c ^ d;
                        k = 0x6ED9EBA1u;
                    }
                    else if (i < 60)
                    {
                        f = (b & c) | (b & d) | (c & d);
                        k = 0x8F1BBCDCu;
                    }
                    else
                    {
                        f = b ^ c ^ d;
                        k = 0xCA62C1D6u;
                    }

                    uint32_t temp = RotateLeft(a, 5) + f + e + k + words[i];
                    e = d;
                    d = c;
                    c = RotateLeft(b, 30);
                    b = a;
                    a = temp;
                }

                State[0] += a;
                State[1] += b;
                State[2] += c;
                State[3] += d;
                State[4] += e;
                BlockSize = 0;
            }

            void Update(const uint8_t* data, size_t size)
            {
                TotalBytes += size;
                for (size_t i = 0; i < size; i++)
                {
                    Block[BlockSize++] = data[i];
                    if (BlockSize == 64)
                    {
                        ProcessBlock();
                    }
                }
            }

            std::array<uint8_t, 20> Finish()
            {
                uint64_t totalBits = TotalBytes * 8;
                uint8_t padding = 0x80;
                Update(&padding, 1);
                uint8_t zero = 0;
                while (BlockSize != 56)
                {
                    Update(&zero, 1);
                }

                uint8_t lengthBytes[8];
                for (int i = 0; i < 8; i++)
                {
                    lengthBytes[i] = static_cast<uint8_t>(totalBits >> ((7 - i) * 8));
                }
                Update(lengthBytes, 8);

                std::array<uint8_t, 20> digest;
                for (int i = 0; i < 5; i++)
                {
                    digest[i * 4] = static_cast<uint8_t>(State[i] >> 24);
                    digest[i * 4 + 1] = static_cast<uint8_t>(State[i] >> 16);
                    digest[i * 4 + 2] = static_cast<uint8_t>(State[i] >> 8);
                    digest[i * 4 + 3] = static_cast<uint8_t>(State[i]);
                }
                return digest;
            }
        };

        std::string Base64Encode(const uint8_t* data, size_t size)
        {
            static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string encoded;
            encoded.reserve(((size + 2) / 3) * 4);
            for (size_t i = 0; i < size; i += 3)
            {
                uint32_t chunk = static_cast<uint32_t>(data[i]) << 16;
                size_t remaining = size - i;
                if (remaining > 1)
                {
                    chunk |= static_cast<uint32_t>(data[i + 1]) << 8;
                }
                if (remaining > 2)
                {
                    chunk |= static_cast<uint32_t>(data[i + 2]);
                }

                encoded.push_back(alphabet[(chunk >> 18) & 0x3F]);
                encoded.push_back(alphabet[(chunk >> 12) & 0x3F]);
                encoded.push_back(remaining > 1 ? alphabet[(chunk >> 6) & 0x3F] : '=');
                encoded.push_back(remaining > 2 ? alphabet[chunk & 0x3F] : '=');
            }
            return encoded;
        }

        std::mt19937_64& Rng()
        {
            thread_local std::mt19937_64 rng = [] {
                std::random_device device;
                std::seed_seq seed{device(), device(), device(), device()};
                return std::mt19937_64(seed);
            }();
            return rng;
        }

        std::string ToLower(std::string_view value)
        {
            std::string lowered(value);
            std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            return lowered;
        }

        std::string_view Trim(std::string_view value)
        {
            while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
            {
                value.remove_prefix(1);
            }
            while (!value.empty() && (value.back() == ' ' || value.back() == '\t'))
            {
                value.remove_suffix(1);
            }
            return value;
        }

        bool IsValidOpcode(uint8_t opcode)
        {
            switch (static_cast<WebSocketOpcode>(opcode))
            {
                case WebSocketOpcode::Continuation:
                case WebSocketOpcode::Text:
                case WebSocketOpcode::Binary:
                case WebSocketOpcode::Close:
                case WebSocketOpcode::Ping:
                case WebSocketOpcode::Pong:
                    return true;
                default:
                    return false;
            }
        }
    }

    std::string GenerateClientKey()
    {
        std::array<uint8_t, 16> nonce;
        for (size_t i = 0; i < nonce.size(); i += 8)
        {
            uint64_t chunk = Rng()();
            std::memcpy(nonce.data() + i, &chunk, 8);
        }
        return Base64Encode(nonce.data(), nonce.size());
    }

    std::string ComputeAcceptKey(const std::string& clientKey)
    {
        static constexpr std::string_view handshakeGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        Sha1 sha;
        sha.Update(reinterpret_cast<const uint8_t*>(clientKey.data()), clientKey.size());
        sha.Update(reinterpret_cast<const uint8_t*>(handshakeGuid.data()), handshakeGuid.size());
        auto digest = sha.Finish();
        return Base64Encode(digest.data(), digest.size());
    }

    std::string BuildHandshakeRequest(const std::string& hostHeader, const std::string& path, const std::string& clientKey)
    {
        std::string request;
        request += "GET " + path + " HTTP/1.1\r\n";
        request += "Host: " + hostHeader + "\r\n";
        request += "Upgrade: websocket\r\n";
        request += "Connection: Upgrade\r\n";
        request += "Sec-WebSocket-Key: " + clientKey + "\r\n";
        request += "Sec-WebSocket-Version: 13\r\n";
        request += "\r\n";
        return request;
    }

    HandshakeResult ParseHandshakeResponse(const uint8_t* data, size_t size, const std::string& clientKey)
    {
        HandshakeResult result;
        std::string_view text(reinterpret_cast<const char*>(data), size);
        size_t headerEnd = text.find("\r\n\r\n");
        if (headerEnd == std::string_view::npos)
        {
            return result;
        }

        result.Complete = true;
        result.HeaderLength = headerEnd + 4;

        std::string_view header = text.substr(0, headerEnd);
        size_t statusLineEnd = header.find("\r\n");
        std::string_view statusLine = statusLineEnd == std::string_view::npos ? header : header.substr(0, statusLineEnd);
        if (!statusLine.starts_with("HTTP/"))
        {
            result.Error = "handshake response is not http";
            return result;
        }

        size_t statusStart = statusLine.find(' ');
        if (statusStart == std::string_view::npos || statusLine.size() < statusStart + 4)
        {
            result.Error = "handshake response has no status code";
            return result;
        }

        std::string_view status = statusLine.substr(statusStart + 1, 3);
        if (status != "101")
        {
            result.Error = "handshake failed with status " + std::string(status);
            return result;
        }

        std::string upgrade;
        std::string connection;
        std::string accept;
        size_t lineStart = statusLineEnd == std::string_view::npos ? header.size() : statusLineEnd + 2;
        while (lineStart < header.size())
        {
            size_t lineEnd = header.find("\r\n", lineStart);
            std::string_view line = header.substr(lineStart, lineEnd == std::string_view::npos ? std::string_view::npos : lineEnd - lineStart);
            lineStart = lineEnd == std::string_view::npos ? header.size() : lineEnd + 2;

            size_t split = line.find(':');
            if (split == std::string_view::npos)
            {
                continue;
            }

            std::string name = ToLower(Trim(line.substr(0, split)));
            std::string_view value = Trim(line.substr(split + 1));
            if (name == "upgrade")
            {
                upgrade = ToLower(value);
            }
            else if (name == "connection")
            {
                connection = ToLower(value);
            }
            else if (name == "sec-websocket-accept")
            {
                accept = std::string(value);
            }
        }

        if (upgrade != "websocket")
        {
            result.Error = "handshake missing websocket upgrade header";
            return result;
        }
        if (connection.find("upgrade") == std::string::npos)
        {
            result.Error = "handshake missing connection upgrade header";
            return result;
        }
        if (accept != ComputeAcceptKey(clientKey))
        {
            result.Error = "handshake accept key mismatch";
            return result;
        }

        result.Ok = true;
        return result;
    }

    std::vector<uint8_t> EncodeFrame(WebSocketOpcode opcode, const uint8_t* payload, size_t size, const std::array<uint8_t, 4>& maskKey)
    {
        std::vector<uint8_t> frame;
        frame.reserve(size + 14);
        frame.push_back(0x80 | static_cast<uint8_t>(opcode));
        if (size <= 125)
        {
            frame.push_back(0x80 | static_cast<uint8_t>(size));
        }
        else if (size <= 0xFFFF)
        {
            frame.push_back(0x80 | 126);
            frame.push_back(static_cast<uint8_t>(size >> 8));
            frame.push_back(static_cast<uint8_t>(size));
        }
        else
        {
            frame.push_back(0x80 | 127);
            for (int i = 7; i >= 0; i--)
            {
                frame.push_back(static_cast<uint8_t>(static_cast<uint64_t>(size) >> (i * 8)));
            }
        }

        frame.insert(frame.end(), maskKey.begin(), maskKey.end());
        for (size_t i = 0; i < size; i++)
        {
            frame.push_back(payload[i] ^ maskKey[i & 3]);
        }
        return frame;
    }

    std::vector<uint8_t> EncodeFrame(WebSocketOpcode opcode, const uint8_t* payload, size_t size)
    {
        std::array<uint8_t, 4> maskKey;
        uint32_t random = static_cast<uint32_t>(Rng()());
        std::memcpy(maskKey.data(), &random, 4);
        return EncodeFrame(opcode, payload, size, maskKey);
    }

    WebSocketDecodeResult DecodeFrame(const uint8_t* data, size_t size, WebSocketFrame& frame, size_t& consumed)
    {
        consumed = 0;
        if (size < 2)
        {
            return WebSocketDecodeResult::NeedMoreData;
        }

        uint8_t first = data[0];
        uint8_t second = data[1];
        // no extensions are negotiated, so any rsv bit means a broken peer
        if ((first & 0x70) != 0)
        {
            return WebSocketDecodeResult::ProtocolError;
        }
        if (!IsValidOpcode(first & 0x0F))
        {
            return WebSocketDecodeResult::ProtocolError;
        }

        bool fin = (first & 0x80) != 0;
        auto opcode = static_cast<WebSocketOpcode>(first & 0x0F);
        bool isControl = (first & 0x08) != 0;
        bool masked = (second & 0x80) != 0;
        uint64_t length = second & 0x7F;
        size_t offset = 2;

        if (length == 126)
        {
            if (size < 4)
            {
                return WebSocketDecodeResult::NeedMoreData;
            }
            length = (static_cast<uint64_t>(data[2]) << 8) | data[3];
            offset = 4;
        }
        else if (length == 127)
        {
            if (size < 10)
            {
                return WebSocketDecodeResult::NeedMoreData;
            }
            length = 0;
            for (int i = 0; i < 8; i++)
            {
                length = (length << 8) | data[2 + i];
            }
            offset = 10;
        }

        if (isControl && (length > 125 || !fin))
        {
            return WebSocketDecodeResult::ProtocolError;
        }
        if (length > MaxPayloadBytes)
        {
            return WebSocketDecodeResult::ProtocolError;
        }

        std::array<uint8_t, 4> maskKey = {};
        if (masked)
        {
            if (size < offset + 4)
            {
                return WebSocketDecodeResult::NeedMoreData;
            }
            std::memcpy(maskKey.data(), data + offset, 4);
            offset += 4;
        }

        if (size - offset < length)
        {
            return WebSocketDecodeResult::NeedMoreData;
        }

        frame.Fin = fin;
        frame.Opcode = opcode;
        frame.Payload.assign(data + offset, data + offset + length);
        if (masked)
        {
            for (size_t i = 0; i < frame.Payload.size(); i++)
            {
                frame.Payload[i] ^= maskKey[i & 3];
            }
        }

        consumed = offset + static_cast<size_t>(length);
        return WebSocketDecodeResult::Frame;
    }
}
