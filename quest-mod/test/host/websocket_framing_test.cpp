// host-side checks for the rfc6455 client framing used by the ludus transport

#include "Features/Live/Transport/WebSocketFraming.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace ScoreSaber::Features::Live::Transport;

namespace
{
    [[noreturn]] void Fail(const std::string& message)
    {
        std::fprintf(stderr, "FAIL: %s\n", message.c_str());
        std::exit(1);
    }

    void Require(bool condition, const std::string& message)
    {
        if (!condition)
        {
            Fail(message);
        }
    }

    std::vector<uint8_t> Bytes(const std::string& text)
    {
        return std::vector<uint8_t>(text.begin(), text.end());
    }

    void TestAcceptKey()
    {
        // golden pair straight from rfc 6455 section 1.3
        Require(WebSocketFraming::ComputeAcceptKey("dGhlIHNhbXBsZSBub25jZQ==") == "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=",
                "rfc 6455 accept key");

        auto first = WebSocketFraming::GenerateClientKey();
        auto second = WebSocketFraming::GenerateClientKey();
        Require(first.size() == 24 && first.ends_with("=="), "client key is base64 of 16 bytes");
        Require(first != second, "client keys are random");
    }

    void TestHandshakeRequest()
    {
        auto request = WebSocketFraming::BuildHandshakeRequest("ludus-1.scoresaber.com", "/v1/connect", "abc123");
        Require(request.starts_with("GET /v1/connect HTTP/1.1\r\n"), "request line");
        Require(request.find("Host: ludus-1.scoresaber.com\r\n") != std::string::npos, "host header");
        Require(request.find("Upgrade: websocket\r\n") != std::string::npos, "upgrade header");
        Require(request.find("Sec-WebSocket-Key: abc123\r\n") != std::string::npos, "key header");
        Require(request.find("Sec-WebSocket-Version: 13\r\n") != std::string::npos, "version header");
        Require(request.ends_with("\r\n\r\n"), "request terminator");
    }

    void TestHandshakeResponse()
    {
        std::string clientKey = "dGhlIHNhbXBsZSBub25jZQ==";
        std::string good = "HTTP/1.1 101 Switching Protocols\r\n"
                           "Upgrade: websocket\r\n"
                           "Connection: Upgrade\r\n"
                           "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
                           "\r\n";
        auto bytes = Bytes(good);
        auto result = WebSocketFraming::ParseHandshakeResponse(bytes.data(), bytes.size(), clientKey);
        Require(result.Complete && result.Ok, "valid 101 accepted");
        Require(result.HeaderLength == bytes.size(), "header length covers full block");

        // leftover bytes after the header must not confuse the parser
        auto withFrame = Bytes(good + "\x81\x00");
        result = WebSocketFraming::ParseHandshakeResponse(withFrame.data(), withFrame.size(), clientKey);
        Require(result.Complete && result.Ok && result.HeaderLength == bytes.size(), "trailing frame data ignored");

        auto partial = Bytes(good.substr(0, good.size() - 2));
        result = WebSocketFraming::ParseHandshakeResponse(partial.data(), partial.size(), clientKey);
        Require(!result.Complete, "partial header incomplete");

        auto forbidden = Bytes("HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n");
        result = WebSocketFraming::ParseHandshakeResponse(forbidden.data(), forbidden.size(), clientKey);
        Require(result.Complete && !result.Ok && result.Error.find("403") != std::string::npos, "non-101 rejected");

        std::string badAccept = "HTTP/1.1 101 Switching Protocols\r\n"
                                "Upgrade: websocket\r\n"
                                "Connection: Upgrade\r\n"
                                "Sec-WebSocket-Accept: bm90IHRoZSByaWdodCBrZXk=\r\n"
                                "\r\n";
        auto badBytes = Bytes(badAccept);
        result = WebSocketFraming::ParseHandshakeResponse(badBytes.data(), badBytes.size(), clientKey);
        Require(result.Complete && !result.Ok, "wrong accept key rejected");

        std::string caseFolded = "HTTP/1.1 101 Switching Protocols\r\n"
                                 "upgrade: WebSocket\r\n"
                                 "connection: keep-alive, Upgrade\r\n"
                                 "sec-websocket-accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
                                 "\r\n";
        auto foldedBytes = Bytes(caseFolded);
        result = WebSocketFraming::ParseHandshakeResponse(foldedBytes.data(), foldedBytes.size(), clientKey);
        Require(result.Complete && result.Ok, "headers matched case-insensitively");
    }

    void TestFrameGolden()
    {
        // rfc 6455 section 5.7: masked "Hello" text frame
        std::vector<uint8_t> golden = {0x81, 0x85, 0x37, 0xFA, 0x21, 0x3D, 0x7F, 0x9F, 0x4D, 0x51, 0x58};
        auto payload = Bytes("Hello");
        auto encoded = WebSocketFraming::EncodeFrame(WebSocketOpcode::Text, payload.data(), payload.size(),
                                                     {0x37, 0xFA, 0x21, 0x3D});
        Require(encoded == golden, "rfc masked hello frame");

        WebSocketFrame frame;
        size_t consumed = 0;
        auto decoded = WebSocketFraming::DecodeFrame(golden.data(), golden.size(), frame, consumed);
        Require(decoded == WebSocketDecodeResult::Frame, "masked frame decodes");
        Require(consumed == golden.size(), "masked frame consumed fully");
        Require(frame.Fin && frame.Opcode == WebSocketOpcode::Text, "masked frame header");
        Require(frame.Payload == payload, "masked frame unmasked payload");

        // rfc 6455 section 5.7: unmasked "Hello" (server-style frame)
        std::vector<uint8_t> unmasked = {0x81, 0x05, 0x48, 0x65, 0x6C, 0x6C, 0x6F};
        decoded = WebSocketFraming::DecodeFrame(unmasked.data(), unmasked.size(), frame, consumed);
        Require(decoded == WebSocketDecodeResult::Frame && frame.Payload == payload, "unmasked frame decodes");
    }

    void TestFrameLengthBoundaries()
    {
        for (size_t length : {size_t(0), size_t(125), size_t(126), size_t(65535), size_t(65536)})
        {
            std::vector<uint8_t> payload(length);
            for (size_t i = 0; i < length; i++)
            {
                payload[i] = static_cast<uint8_t>(i * 31);
            }

            auto encoded = WebSocketFraming::EncodeFrame(WebSocketOpcode::Binary, payload.data(), payload.size());
            WebSocketFrame frame;
            size_t consumed = 0;
            auto decoded = WebSocketFraming::DecodeFrame(encoded.data(), encoded.size(), frame, consumed);
            Require(decoded == WebSocketDecodeResult::Frame, "boundary frame decodes");
            Require(consumed == encoded.size(), "boundary frame consumed fully");
            Require(frame.Payload == payload, "boundary frame round-trips");

            // one byte short must ask for more data
            decoded = WebSocketFraming::DecodeFrame(encoded.data(), encoded.size() - 1, frame, consumed);
            Require(decoded == WebSocketDecodeResult::NeedMoreData, "truncated frame needs more data");
        }
    }

    void TestFragmentedAndControlFrames()
    {
        auto payload = Bytes("fragment");
        auto encoded = WebSocketFraming::EncodeFrame(WebSocketOpcode::Binary, payload.data(), payload.size());
        encoded[0] &= 0x7F; // clear fin: first frame of a fragmented message
        WebSocketFrame frame;
        size_t consumed = 0;
        auto decoded = WebSocketFraming::DecodeFrame(encoded.data(), encoded.size(), frame, consumed);
        Require(decoded == WebSocketDecodeResult::Frame && !frame.Fin, "non-fin frame decodes");

        std::vector<uint8_t> continuation = {0x80, 0x03, 0x61, 0x62, 0x63};
        decoded = WebSocketFraming::DecodeFrame(continuation.data(), continuation.size(), frame, consumed);
        Require(decoded == WebSocketDecodeResult::Frame && frame.Opcode == WebSocketOpcode::Continuation && frame.Fin,
                "continuation frame decodes");

        std::vector<uint8_t> ping = {0x89, 0x02, 0x68, 0x69};
        decoded = WebSocketFraming::DecodeFrame(ping.data(), ping.size(), frame, consumed);
        Require(decoded == WebSocketDecodeResult::Frame && frame.Opcode == WebSocketOpcode::Ping, "ping decodes");
    }

    void TestProtocolErrors()
    {
        WebSocketFrame frame;
        size_t consumed = 0;

        std::vector<uint8_t> reservedBits = {0xC1, 0x00};
        Require(WebSocketFraming::DecodeFrame(reservedBits.data(), reservedBits.size(), frame, consumed) ==
                    WebSocketDecodeResult::ProtocolError,
                "rsv bits rejected");

        std::vector<uint8_t> badOpcode = {0x83, 0x00};
        Require(WebSocketFraming::DecodeFrame(badOpcode.data(), badOpcode.size(), frame, consumed) ==
                    WebSocketDecodeResult::ProtocolError,
                "reserved opcode rejected");

        std::vector<uint8_t> longControl = {0x88, 0x7E, 0x00, 0x80};
        Require(WebSocketFraming::DecodeFrame(longControl.data(), longControl.size(), frame, consumed) ==
                    WebSocketDecodeResult::ProtocolError,
                "oversized control frame rejected");

        std::vector<uint8_t> fragmentedControl = {0x09, 0x00};
        Require(WebSocketFraming::DecodeFrame(fragmentedControl.data(), fragmentedControl.size(), frame, consumed) ==
                    WebSocketDecodeResult::ProtocolError,
                "fragmented control frame rejected");

        // 64-bit length far past the sanity cap
        std::vector<uint8_t> huge = {0x82, 0x7F, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
        Require(WebSocketFraming::DecodeFrame(huge.data(), huge.size(), frame, consumed) ==
                    WebSocketDecodeResult::ProtocolError,
                "oversized payload length rejected");

        std::vector<uint8_t> tiny = {0x82};
        Require(WebSocketFraming::DecodeFrame(tiny.data(), tiny.size(), frame, consumed) ==
                    WebSocketDecodeResult::NeedMoreData,
                "single byte needs more data");
    }
} // namespace

int main()
{
    TestAcceptKey();
    TestHandshakeRequest();
    TestHandshakeResponse();
    TestFrameGolden();
    TestFrameLengthBoundaries();
    TestFragmentedAndControlFrames();
    TestProtocolErrors();
    std::printf("websocket framing tests passed\n");
    return 0;
}
