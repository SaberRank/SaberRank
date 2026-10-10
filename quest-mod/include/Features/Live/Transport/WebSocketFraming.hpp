#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// minimal rfc6455 client framing. quest-only shim: pc uses .net's ClientWebSocket,
// but the bundled libcurl is built without websocket support, so the upgrade
// handshake and frame codec live here (pure c++, host-testable) and the socket
// io rides curl's connect-only tls layer in CurlWebSocketConnection.
namespace SnoreSaber::Features::Live::Transport
{
    enum class WebSocketOpcode : uint8_t
    {
        Continuation = 0x0,
        Text = 0x1,
        Binary = 0x2,
        Close = 0x8,
        Ping = 0x9,
        Pong = 0xA,
    };

    struct WebSocketFrame
    {
        bool Fin = true;
        WebSocketOpcode Opcode = WebSocketOpcode::Binary;
        std::vector<uint8_t> Payload;
    };

    enum class WebSocketDecodeResult
    {
        NeedMoreData,
        Frame,
        ProtocolError,
    };

    namespace WebSocketFraming
    {
        // sanity cap so a corrupt length prefix cannot ask us to buffer gigabytes
        inline constexpr size_t MaxPayloadBytes = 32 * 1024 * 1024;

        std::string GenerateClientKey();
        std::string ComputeAcceptKey(const std::string& clientKey);
        std::string BuildHandshakeRequest(const std::string& hostHeader, const std::string& path, const std::string& clientKey);

        struct HandshakeResult
        {
            bool Complete = false;
            bool Ok = false;
            std::string Error;
            // bytes consumed by the response header block when Complete
            size_t HeaderLength = 0;
        };
        HandshakeResult ParseHandshakeResponse(const uint8_t* data, size_t size, const std::string& clientKey);

        std::vector<uint8_t> EncodeFrame(WebSocketOpcode opcode, const uint8_t* payload, size_t size, const std::array<uint8_t, 4>& maskKey);
        // client frames are always masked; this overload picks a random mask key
        std::vector<uint8_t> EncodeFrame(WebSocketOpcode opcode, const uint8_t* payload, size_t size);
        WebSocketDecodeResult DecodeFrame(const uint8_t* data, size_t size, WebSocketFrame& frame, size_t& consumed);
    }
}
