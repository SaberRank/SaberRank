#include "Features/Live/Transport/CurlWebSocketConnection.hpp"

#include "Utils/WebUtils.hpp"
#include "logging.hpp"

#include <libcurl/shared/curl.h>
#include <libcurl/shared/easy.h>

#include <sys/select.h>

#include <chrono>
#include <cstring>

namespace SnoreSaber::Features::Live::Transport
{
    namespace
    {
        constexpr int PollIntervalMs = 250;
        constexpr size_t ReadChunkBytes = 16 * 1024;

        int64_t NowMs()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        }
    }

    CurlWebSocketConnection::~CurlWebSocketConnection()
    {
        if (_curl)
        {
            curl_easy_cleanup(static_cast<CURL*>(_curl));
        }
    }

    bool CurlWebSocketConnection::Connect(const std::string& url, long timeoutMs, std::string& error)
    {
        bool secure;
        std::string rest;
        if (url.starts_with("wss://"))
        {
            secure = true;
            rest = url.substr(6);
        }
        else if (url.starts_with("ws://"))
        {
            secure = false;
            rest = url.substr(5);
        }
        else
        {
            error = "unsupported websocket url: " + url;
            return false;
        }

        size_t slash = rest.find('/');
        std::string hostPort = slash == std::string::npos ? rest : rest.substr(0, slash);
        std::string path = slash == std::string::npos ? "/" : rest.substr(slash);
        if (hostPort.empty())
        {
            error = "websocket url has no host: " + url;
            return false;
        }

        auto* curl = curl_easy_init();
        if (!curl)
        {
            error = "curl_easy_init failed";
            return false;
        }
        _curl = curl;

        // curl only dials; the upgrade request goes over the raw (tls) socket below
        std::string connectUrl = (secure ? "https://" : "http://") + hostPort;
        curl_easy_setopt(curl, CURLOPT_URL, connectUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeoutMs);
        WebUtils::ApplyTransportOptions(curl);

        CURLcode result = curl_easy_perform(curl);
        if (result != CURLE_OK)
        {
            error = curl_easy_strerror(result);
            return false;
        }

        curl_socket_t socket = CURL_SOCKET_BAD;
        result = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &socket);
        if (result != CURLE_OK || socket == CURL_SOCKET_BAD)
        {
            error = "failed to resolve websocket socket handle";
            return false;
        }
        _socket = static_cast<int>(socket);

        std::string clientKey = WebSocketFraming::GenerateClientKey();
        std::string request = WebSocketFraming::BuildHandshakeRequest(hostPort, path, clientKey);
        if (!SendRaw(reinterpret_cast<const uint8_t*>(request.data()), request.size(), error))
        {
            if (error.empty())
            {
                error = "websocket handshake canceled";
            }
            return false;
        }

        int64_t deadline = NowMs() + timeoutMs;
        while (true)
        {
            auto handshake = WebSocketFraming::ParseHandshakeResponse(_receiveBuffer.data(), _receiveBuffer.size(), clientKey);
            if (handshake.Complete)
            {
                if (!handshake.Ok)
                {
                    error = handshake.Error;
                    return false;
                }
                // whatever followed the header block is already frame data
                _receiveBuffer.erase(_receiveBuffer.begin(), _receiveBuffer.begin() + handshake.HeaderLength);
                break;
            }

            if (_closed.load())
            {
                error = "websocket handshake canceled";
                return false;
            }
            if (NowMs() > deadline)
            {
                error = "websocket handshake timed out";
                return false;
            }

            WaitForSocket(true, PollIntervalMs);
            ReadStatus read = ReadChunk(error);
            if (read == ReadStatus::PeerClosed)
            {
                error = "connection closed during websocket handshake";
                return false;
            }
            if (read == ReadStatus::Error)
            {
                return false;
            }
        }

        _open.store(true);
        return true;
    }

    bool CurlWebSocketConnection::SendBinary(const uint8_t* data, size_t size, std::string& error)
    {
        if (!IsOpen())
        {
            error = _closed.load() ? "" : "socket is not open";
            return false;
        }

        auto frame = WebSocketFraming::EncodeFrame(WebSocketOpcode::Binary, data, size);
        return SendRaw(frame.data(), frame.size(), error);
    }

    WebSocketReceiveStatus CurlWebSocketConnection::ReceiveMessage(std::vector<uint8_t>& message, std::string& error)
    {
        while (true)
        {
            if (_closed.load())
            {
                return WebSocketReceiveStatus::Canceled;
            }

            WebSocketFrame frame;
            size_t consumed = 0;
            auto decoded = WebSocketFraming::DecodeFrame(_receiveBuffer.data(), _receiveBuffer.size(), frame, consumed);
            if (decoded == WebSocketDecodeResult::ProtocolError)
            {
                error = "websocket protocol error";
                return WebSocketReceiveStatus::Failed;
            }

            if (decoded == WebSocketDecodeResult::Frame)
            {
                _receiveBuffer.erase(_receiveBuffer.begin(), _receiveBuffer.begin() + consumed);

                switch (frame.Opcode)
                {
                    case WebSocketOpcode::Ping:
                    {
                        auto pong = WebSocketFraming::EncodeFrame(WebSocketOpcode::Pong, frame.Payload.data(), frame.Payload.size());
                        std::string pongError;
                        SendRaw(pong.data(), pong.size(), pongError);
                        continue;
                    }
                    case WebSocketOpcode::Pong:
                        continue;
                    case WebSocketOpcode::Close:
                    {
                        if (!_sentClose)
                        {
                            _sentClose = true;
                            auto close = WebSocketFraming::EncodeFrame(WebSocketOpcode::Close, frame.Payload.data(), frame.Payload.size());
                            std::string closeError;
                            SendRaw(close.data(), close.size(), closeError);
                        }
                        _open.store(false);
                        return WebSocketReceiveStatus::Closed;
                    }
                    case WebSocketOpcode::Text:
                    case WebSocketOpcode::Binary:
                        if (_messageStarted)
                        {
                            error = "new websocket message before previous one finished";
                            return WebSocketReceiveStatus::Failed;
                        }
                        if (frame.Fin)
                        {
                            message = std::move(frame.Payload);
                            return WebSocketReceiveStatus::Message;
                        }
                        _messageStarted = true;
                        _messageBuffer = std::move(frame.Payload);
                        continue;
                    case WebSocketOpcode::Continuation:
                        if (!_messageStarted)
                        {
                            error = "unexpected websocket continuation frame";
                            return WebSocketReceiveStatus::Failed;
                        }
                        if (_messageBuffer.size() + frame.Payload.size() > WebSocketFraming::MaxPayloadBytes)
                        {
                            error = "websocket message too large";
                            return WebSocketReceiveStatus::Failed;
                        }
                        _messageBuffer.insert(_messageBuffer.end(), frame.Payload.begin(), frame.Payload.end());
                        if (frame.Fin)
                        {
                            _messageStarted = false;
                            message = std::move(_messageBuffer);
                            _messageBuffer.clear();
                            return WebSocketReceiveStatus::Message;
                        }
                        continue;
                }
                continue;
            }

            WaitForSocket(true, PollIntervalMs);
            ReadStatus read = ReadChunk(error);
            if (read == ReadStatus::PeerClosed)
            {
                _open.store(false);
                error = "connection closed unexpectedly";
                return WebSocketReceiveStatus::Failed;
            }
            if (read == ReadStatus::Error)
            {
                _open.store(false);
                return WebSocketReceiveStatus::Failed;
            }
        }
    }

    void CurlWebSocketConnection::RequestClose()
    {
        _closed.store(true);
    }

    bool CurlWebSocketConnection::IsOpen() const
    {
        return _open.load() && !_closed.load();
    }

    bool CurlWebSocketConnection::WaitForSocket(bool forRead, int timeoutMs)
    {
        if (_socket < 0)
        {
            return false;
        }

        fd_set set;
        FD_ZERO(&set);
        FD_SET(_socket, &set);
        timeval timeout;
        timeout.tv_sec = timeoutMs / 1000;
        timeout.tv_usec = (timeoutMs % 1000) * 1000;
        int ready = select(_socket + 1, forRead ? &set : nullptr, forRead ? nullptr : &set, nullptr, &timeout);
        return ready > 0;
    }

    CurlWebSocketConnection::ReadStatus CurlWebSocketConnection::ReadChunk(std::string& error)
    {
        uint8_t chunk[ReadChunkBytes];
        size_t received = 0;
        CURLcode result;
        {
            std::lock_guard<std::mutex> lock(_ioLock);
            result = curl_easy_recv(static_cast<CURL*>(_curl), chunk, sizeof(chunk), &received);
        }

        if (result == CURLE_AGAIN)
        {
            return ReadStatus::NoData;
        }
        if (result != CURLE_OK)
        {
            error = curl_easy_strerror(result);
            return ReadStatus::Error;
        }
        if (received == 0)
        {
            return ReadStatus::PeerClosed;
        }

        if (_receiveBuffer.size() + received > WebSocketFraming::MaxPayloadBytes + ReadChunkBytes)
        {
            error = "websocket receive buffer overflow";
            return ReadStatus::Error;
        }
        _receiveBuffer.insert(_receiveBuffer.end(), chunk, chunk + received);
        return ReadStatus::Data;
    }

    bool CurlWebSocketConnection::SendRaw(const uint8_t* data, size_t size, std::string& error)
    {
        std::lock_guard<std::mutex> frameLock(_sendLock);
        size_t offset = 0;
        while (offset < size)
        {
            if (_closed.load())
            {
                // deliberate close: not an error the caller should report
                error = "";
                return false;
            }

            size_t sent = 0;
            CURLcode result;
            {
                std::lock_guard<std::mutex> lock(_ioLock);
                result = curl_easy_send(static_cast<CURL*>(_curl), data + offset, size - offset, &sent);
            }

            if (result == CURLE_AGAIN || (result == CURLE_OK && sent == 0))
            {
                WaitForSocket(false, PollIntervalMs);
                continue;
            }
            if (result != CURLE_OK)
            {
                _open.store(false);
                error = curl_easy_strerror(result);
                return false;
            }

            offset += sent;
        }
        return true;
    }
}
