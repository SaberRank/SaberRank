#pragma once

#include "Core/Timing/SnoreSaberClock.hpp"
#include "Features/Live/Cancellation.hpp"
#include "Features/Live/Compete/Domain/CompeteMapStartCountdown.hpp"
#include "Features/Live/Ludus/Services/LudusMainThreadQueue.hpp"

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <string>

namespace SnoreSaber::Features::Live::Ludus::Services
{
    // ticks the pre-map countdown on a worker thread and reports changes through the
    // main thread queue; clock must stay rooted by the owning il2cpp service
    class LudusMapStartCountdown
    {
      public:
        LudusMapStartCountdown(LudusMainThreadQueue* mainThread, std::function<std::string()> defaultMatchId, Core::Timing::SnoreSaberClock* clock);

        // nullopt clears the countdown display (PC Changed(null)); ticks arrive via the
        // main thread queue, but Cancel invokes this on the calling thread like PC
        std::function<void(const std::optional<Compete::Domain::CompeteMapStartCountdown>&)> changed;

        CancellationToken Begin(const std::string& matchId, int delayMs, const CancellationToken& cancellationToken);
        bool TryCancel(const std::string& matchId);
        void Complete(const std::string& matchId, const CancellationToken& countdownToken);
        void Cancel();

      private:
        void Run(const std::string& matchId, int64_t startDeadlineMs, int version, const CancellationToken& cancellationToken);
        void EnqueueChanged(const std::string& matchId, int remainingSeconds, int version);
        bool MatchesLocked(const std::string& matchId) const;
        std::string MatchIdOrDefault(const std::string& matchId) const;
        int RemainingSeconds(int64_t startDeadlineMs) const;

        LudusMainThreadQueue* _mainThread;
        std::function<std::string()> _defaultMatchId;
        Core::Timing::SnoreSaberClock* _clock;

        // pc leaves these fields unsynchronized; c++ needs the lock to keep the races defined
        mutable std::mutex _lock;
        std::optional<CancellationSource> _cancellation;
        CancellationToken _cancellationToken;
        std::string _matchId;
        std::atomic<int> _version{0};
    };
}
