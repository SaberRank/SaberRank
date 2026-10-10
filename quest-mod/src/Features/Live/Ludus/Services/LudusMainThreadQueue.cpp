#include "Features/Live/Ludus/Services/LudusMainThreadQueue.hpp"

#include "logging.hpp"

namespace SnoreSaber::Features::Live::Ludus::Services
{
    void LudusMainThreadQueue::Enqueue(std::function<void()> action)
    {
        std::lock_guard<std::mutex> lock(_lock);
        _actions.push(std::move(action));
    }

    size_t LudusMainThreadQueue::Drain(size_t maxActions)
    {
        size_t processed = 0;
        while (true)
        {
            std::function<void()> action;
            {
                std::lock_guard<std::mutex> lock(_lock);
                if (_actions.empty() || processed >= maxActions)
                {
                    return _actions.size();
                }

                action = std::move(_actions.front());
                _actions.pop();
            }

            try
            {
                action();
            }
            catch (const std::exception& e)
            {
                ERROR("Ludus main thread action failed: {}", e.what());
            }
            catch (...)
            {
                ERROR("Ludus main thread action failed");
            }
            processed++;
        }
    }
}
