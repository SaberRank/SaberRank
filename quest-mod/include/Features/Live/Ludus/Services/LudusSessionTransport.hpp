#pragma once

#include "Features/Live/Ludus/Services/LudusMainThreadQueue.hpp"
#include "Features/Live/Transport/CurlWebSocketConnection.hpp"

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace SnoreSaber::Features::Live::Ludus::Services
{
    // pc's ClientWebSocket transport rebuilt on a curl connect-only socket:
    // a receive thread plus one worker that serializes sends (replay streaming
    // can queue many chunks). all callbacks are marshalled through the ludus
    // main thread queue, mirroring the pc task-chain semantics.
    class LudusSessionTransport
    {
      public:
        explicit LudusSessionTransport(LudusMainThreadQueue* mainThread);
        ~LudusSessionTransport();
        LudusSessionTransport(const LudusSessionTransport&) = delete;
        LudusSessionTransport& operator=(const LudusSessionTransport&) = delete;

        std::function<void(const std::vector<uint8_t>&)> MessageReceived;
        std::function<void(std::string)> ReceiveFailed;
        std::function<void(std::string)> SendFailed;
        std::function<void(std::string)> ReconnectRequested;
        std::function<void()> Disconnected;

        bool IsOpen();
        void Prepare();
        // blocking connect + upgrade handshake; call off the main thread
        bool Connect(const std::string& url, long timeoutMs, std::string& error);
        void StartReceiveLoop();
        void Send(std::vector<uint8_t> bytes);
        // defers the (potentially expensive) encode onto the send worker thread
        bool SendDeferred(std::function<std::vector<uint8_t>()> bytesFactory);
        void DisposeSocket();

      private:
        struct PendingSend
        {
            std::shared_ptr<Transport::CurlWebSocketConnection> Socket;
            std::function<std::vector<uint8_t>()> BytesFactory;
        };

        std::shared_ptr<Transport::CurlWebSocketConnection> CurrentSocket();
        void QueueSend(std::function<std::vector<uint8_t>()> bytesFactory);
        void SendWorkerLoop();
        void ReceiveLoop(std::shared_ptr<Transport::CurlWebSocketConnection> socket);
        void EnqueueOnMain(std::function<void()> action);

        LudusMainThreadQueue* _mainThread;
        std::mutex _socketLock;
        std::shared_ptr<Transport::CurlWebSocketConnection> _socket;
        std::thread _receiveThread;

        std::mutex _sendLock;
        std::condition_variable _sendSignal;
        std::deque<PendingSend> _pendingSends;
        bool _shutdown = false;
        std::thread _sendWorker;
    };
}
