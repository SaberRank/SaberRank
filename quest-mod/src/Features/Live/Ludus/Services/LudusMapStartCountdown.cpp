#include "Features/Live/Ludus/Services/LudusMapStartCountdown.hpp"

#include "Utils/AsyncUtils.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>
#include <utility>

namespace SnoreSaber::Features::Live::Ludus::Services
{
    LudusMapStartCountdown::LudusMapStartCountdown(LudusMainThreadQueue* mainThread, std::function<std::string()> defaultMatchId, Core::Timing::SnoreSaberClock* clock)
        : _mainThread(mainThread), _defaultMatchId(std::move(defaultMatchId)), _clock(clock)
    {
    }

    CancellationToken LudusMapStartCountdown::Begin(const std::string& matchId, int delayMs, const CancellationToken& cancellationToken)
    {
        Cancel();

        std::lock_guard lock(_lock);
        _matchId = MatchIdOrDefault(matchId);
        _cancellation.emplace();
        _cancellationToken = _cancellation->Token().Linked(cancellationToken);
        if (delayMs > 0)
        {
            int version = _version.fetch_add(1) + 1;
            int64_t startDeadlineMs = _clock->MonotonicMilliseconds() + delayMs;
            // countdown state is app-scoped in the session service, safe to capture across the worker
            Utils::Async::Run([this, matchId = _matchId, startDeadlineMs, version, token = _cancellationToken] {
                Run(matchId, startDeadlineMs, version, token);
            });
        }

        return _cancellationToken;
    }

    bool LudusMapStartCountdown::TryCancel(const std::string& matchId)
    {
        {
            std::lock_guard lock(_lock);
            if (!_cancellation || !MatchesLocked(matchId))
            {
                return false;
            }
        }

        Cancel();
        return true;
    }

    void LudusMapStartCountdown::Complete(const std::string& matchId, const CancellationToken& countdownToken)
    {
        bool shouldCancel;
        {
            std::lock_guard lock(_lock);
            shouldCancel = _cancellation && MatchesLocked(matchId) && _cancellationToken == countdownToken;
        }

        if (shouldCancel)
        {
            Cancel();
        }
    }

    void LudusMapStartCountdown::Cancel()
    {
        {
            std::lock_guard lock(_lock);
            _version.fetch_add(1);
            if (_cancellation)
            {
                _cancellation->Cancel();
            }

            _cancellation.reset();
            _cancellationToken = {};
            _matchId.clear();
        }

        // cancel can run on async workers (start map handler); ui subscribers need main thread
        _mainThread->Enqueue([this] {
            if (changed)
            {
                changed(std::nullopt);
            }
        });
    }

    void LudusMapStartCountdown::Run(const std::string& matchId, int64_t startDeadlineMs, int version, const CancellationToken& cancellationToken)
    {
        int lastSeconds = -1;

        try
        {
            while (true)
            {
                int remainingSeconds = RemainingSeconds(startDeadlineMs);
                if (remainingSeconds != lastSeconds)
                {
                    lastSeconds = remainingSeconds;
                    EnqueueChanged(matchId, remainingSeconds, version);
                }

                if (remainingSeconds == 0)
                {
                    return;
                }

                // Task.Delay(200, ct) on pc; poll in 25ms slices so cancellation lands promptly
                for (int waited = 0; waited < 200; waited += 25)
                {
                    cancellationToken.ThrowIfCancellationRequested();
                    std::this_thread::sleep_for(std::chrono::milliseconds(25));
                }
            }
        }
        catch (const OperationCanceledException&)
        {
        }
    }

    void LudusMapStartCountdown::EnqueueChanged(const std::string& matchId, int remainingSeconds, int version)
    {
        _mainThread->Enqueue([this, matchId, remainingSeconds, version] {
            if (version == _version.load() && changed)
            {
                changed(Compete::Domain::CompeteMapStartCountdown{.matchId = matchId, .remainingSeconds = remainingSeconds});
            }
        });
    }

    bool LudusMapStartCountdown::MatchesLocked(const std::string& matchId) const
    {
        return matchId.empty() || MatchIdOrDefault(matchId) == _matchId;
    }

    std::string LudusMapStartCountdown::MatchIdOrDefault(const std::string& matchId) const
    {
        return !matchId.empty() ? matchId : _defaultMatchId();
    }

    int LudusMapStartCountdown::RemainingSeconds(int64_t startDeadlineMs) const
    {
        int64_t remainingMs = startDeadlineMs - _clock->MonotonicMilliseconds();
        return remainingMs <= 0 ? 0 : std::max(1, static_cast<int>(std::ceil(remainingMs / 1000.0)));
    }
}
