#pragma once

#include "Features/Live/Transport/WebSocketFraming.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace SnoreSaber::Features::Live::Transport
{
    enum class WebSocketReceiveStatus
    {
        Message,
        Closed,
        Canceled,
        Failed,
    };

    // blocking rfc6455 client over a curl connect-only socket. curl provides the
    // tcp + tls layer (shared ca-blob hardening from WebUtils); framing is ours.
    // one thread may send while another receives; the short non-blocking curl
    // calls are serialized behind _ioLock, the waits are not.
    class CurlWebSocketConnection
    {
      public:
        ~CurlWebSocketConnection();
        CurlWebSocketConnection() = default;
        CurlWebSocketConnection(const CurlWebSocketConnection&) = delete;
        CurlWebSocketConnection& operator=(const CurlWebSocketConnection&) = delete;

        // blocking: tls connect + http upgrade handshake; call off the main thread
        bool Connect(const std::string& url, long timeoutMs, std::string& error);
        // serialized by the transport send worker; error stays empty when the socket was closed on purpose
        bool SendBinary(const uint8_t* data, size_t size, std::string& error);
        // blocking; receive-thread only. handles ping/pong/close and fragmented messages
        WebSocketReceiveStatus ReceiveMessage(std::vector<uint8_t>& message, std::string& error);
        // any thread: abortive close, unblocks pending send/receive within one poll interval
        void RequestClose();
        bool IsOpen() const;

      private:
        enum class ReadStatus
        {
            Data,
            NoData,
            PeerClosed,
            Error,
        };

        bool WaitForSocket(bool forRead, int timeoutMs);
        ReadStatus ReadChunk(std::string& error);
        bool SendRaw(const uint8_t* data, size_t size, std::string& error);

        void* _curl = nullptr;
        int _socket = -1;
        // _ioLock guards individual curl calls against the receive side;
        // _sendLock keeps whole frames contiguous when a pong races a data frame
        std::mutex _ioLock;
        std::mutex _sendLock;
        std::atomic<bool> _closed{false};
        std::atomic<bool> _open{false};
        bool _sentClose = false;
        std::vector<uint8_t> _receiveBuffer;
        std::vector<uint8_t> _messageBuffer;
        bool _messageStarted = false;
    };
}
