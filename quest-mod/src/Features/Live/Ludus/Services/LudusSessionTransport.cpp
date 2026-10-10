#include "Features/Live/Ludus/Services/LudusSessionTransport.hpp"

#include "logging.hpp"

namespace SnoreSaber::Features::Live::Ludus::Services
{
    LudusSessionTransport::LudusSessionTransport(LudusMainThreadQueue* mainThread) : _mainThread(mainThread)
    {
        _sendWorker = std::thread([this] { SendWorkerLoop(); });
    }

    LudusSessionTransport::~LudusSessionTransport()
    {
        DisposeSocket();
        {
            std::lock_guard<std::mutex> lock(_sendLock);
            _shutdown = true;
        }
        _sendSignal.notify_all();
        if (_sendWorker.joinable())
        {
            _sendWorker.join();
        }
        if (_receiveThread.joinable())
        {
            _receiveThread.join();
        }
    }

    bool LudusSessionTransport::IsOpen()
    {
        auto socket = CurrentSocket();
        return socket && socket->IsOpen();
    }

    void LudusSessionTransport::Prepare()
    {
        DisposeSocket();
        // the closed socket unblocks its receive loop within one poll interval
        if (_receiveThread.joinable())
        {
            _receiveThread.join();
        }

        std::lock_guard<std::mutex> lock(_socketLock);
        _socket = std::make_shared<Transport::CurlWebSocketConnection>();
    }

    bool LudusSessionTransport::Connect(const std::string& url, long timeoutMs, std::string& error)
    {
        auto socket = CurrentSocket();
        if (!socket)
        {
            error = "socket is not prepared";
            return false;
        }
        return socket->Connect(url, timeoutMs, error);
    }

    void LudusSessionTransport::StartReceiveLoop()
    {
        auto socket = CurrentSocket();
        if (!socket)
        {
            return;
        }
        if (_receiveThread.joinable())
        {
            _receiveThread.join();
        }
        _receiveThread = std::thread([this, socket] { ReceiveLoop(socket); });
    }

    void LudusSessionTransport::Send(std::vector<uint8_t> bytes)
    {
        if (bytes.empty())
        {
            return;
        }
        QueueSend([bytes = std::move(bytes)] { return bytes; });
    }

    bool LudusSessionTransport::SendDeferred(std::function<std::vector<uint8_t>()> bytesFactory)
    {
        if (!bytesFactory)
        {
            return false;
        }
        QueueSend(std::move(bytesFactory));
        return true;
    }

    void LudusSessionTransport::DisposeSocket()
    {
        std::shared_ptr<Transport::CurlWebSocketConnection> socket;
        {
            std::lock_guard<std::mutex> lock(_socketLock);
            socket = std::move(_socket);
            _socket = nullptr;
        }
        if (socket)
        {
            socket->RequestClose();
        }
    }

    std::shared_ptr<Transport::CurlWebSocketConnection> LudusSessionTransport::CurrentSocket()
    {
        std::lock_guard<std::mutex> lock(_socketLock);
        return _socket;
    }

    void LudusSessionTransport::QueueSend(std::function<std::vector<uint8_t>()> bytesFactory)
    {
        PendingSend pending{CurrentSocket(), std::move(bytesFactory)};
        {
            std::lock_guard<std::mutex> lock(_sendLock);
            _pendingSends.push_back(std::move(pending));
        }
        _sendSignal.notify_one();
    }

    void LudusSessionTransport::SendWorkerLoop()
    {
        while (true)
        {
            PendingSend pending;
            {
                std::unique_lock<std::mutex> lock(_sendLock);
                _sendSignal.wait(lock, [this] { return _shutdown || !_pendingSends.empty(); });
                if (_shutdown)
                {
                    return;
                }
                pending = std::move(_pendingSends.front());
                _pendingSends.pop_front();
            }

            // sends captured against a socket that got replaced are dropped, like pc
            if (!pending.Socket || pending.Socket != CurrentSocket())
            {
                continue;
            }

            if (!pending.Socket->IsOpen())
            {
                EnqueueOnMain([this] {
                    if (ReconnectRequested)
                    {
                        ReconnectRequested("socket is not open");
                    }
                });
                continue;
            }

            std::vector<uint8_t> bytes;
            try
            {
                bytes = pending.BytesFactory();
            }
            catch (const std::exception& e)
            {
                std::string message = e.what();
                EnqueueOnMain([this, message] {
                    if (SendFailed)
                    {
                        SendFailed(message);
                    }
                });
                continue;
            }
            catch (...)
            {
                EnqueueOnMain([this] {
                    if (SendFailed)
                    {
                        SendFailed("failed to encode ludus frame");
                    }
                });
                continue;
            }

            if (bytes.empty())
            {
                continue;
            }

            std::string error;
            if (!pending.Socket->SendBinary(bytes.data(), bytes.size(), error) && !error.empty())
            {
                EnqueueOnMain([this, error] {
                    if (SendFailed)
                    {
                        SendFailed(error);
                    }
                    if (ReconnectRequested)
                    {
                        ReconnectRequested(error);
                    }
                });
            }
        }
    }

    void LudusSessionTransport::ReceiveLoop(std::shared_ptr<Transport::CurlWebSocketConnection> socket)
    {
        while (socket->IsOpen())
        {
            std::vector<uint8_t> message;
            std::string error;
            auto status = socket->ReceiveMessage(message, error);
            if (status == Transport::WebSocketReceiveStatus::Message)
            {
                EnqueueOnMain([this, bytes = std::move(message)] {
                    if (MessageReceived)
                    {
                        MessageReceived(bytes);
                    }
                });
                continue;
            }

            if (status == Transport::WebSocketReceiveStatus::Failed)
            {
                EnqueueOnMain([this, error] {
                    if (ReceiveFailed)
                    {
                        ReceiveFailed(error);
                    }
                });
            }
            break;
        }

        EnqueueOnMain([this, socket] {
            if (CurrentSocket() == socket && Disconnected)
            {
                Disconnected();
            }
        });
    }

    void LudusSessionTransport::EnqueueOnMain(std::function<void()> action)
    {
        _mainThread->Enqueue(std::move(action));
    }
}
