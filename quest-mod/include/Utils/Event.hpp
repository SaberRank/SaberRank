#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <utility>
#include <vector>

namespace SnoreSaber::Utils
{
    // minimal stand-in for C# multicast events; token-based removal, invoke copies
    // the handler list so subscribers can add/remove from inside a callback
    template <typename... Args>
    class Event
    {
      public:
        using Handler = std::function<void(Args...)>;

        uint64_t Add(Handler handler)
        {
            std::lock_guard lock(_lock);
            uint64_t token = _nextToken++;
            _handlers.emplace_back(token, std::move(handler));
            return token;
        }

        void Remove(uint64_t token)
        {
            std::lock_guard lock(_lock);
            std::erase_if(_handlers, [token](const auto& entry) { return entry.first == token; });
        }

        void Invoke(Args... args) const
        {
            std::vector<std::pair<uint64_t, Handler>> handlers;
            {
                std::lock_guard lock(_lock);
                handlers = _handlers;
            }
            for (const auto& entry : handlers)
            {
                entry.second(args...);
            }
        }

      private:
        mutable std::mutex _lock;
        std::vector<std::pair<uint64_t, Handler>> _handlers;
        uint64_t _nextToken = 1;
    };
}
