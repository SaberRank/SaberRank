#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>

namespace SnoreSaber::Features::Live::Ludus::Services
{
    // hands transport-thread callbacks to the unity main thread; drained by the session service tick
    class LudusMainThreadQueue
    {
      public:
        void Enqueue(std::function<void()> action);
        // runs up to maxActions queued actions, returns how many are still pending
        size_t Drain(size_t maxActions);

      private:
        std::queue<std::function<void()>> _actions;
        std::mutex _lock;
    };
}
