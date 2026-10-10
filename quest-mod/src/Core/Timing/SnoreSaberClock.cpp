#include "Core/Timing/SnoreSaberClock.hpp"

#include "Utils/AsyncUtils.hpp"
#include "logging.hpp"

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <limits>
#include <mutex>
#include <optional>
#include <string>

DEFINE_TYPE(SnoreSaber::Core::Timing, SnoreSaberClock);

namespace SnoreSaber::Core::Timing
{
    namespace Detail
    {
        struct ClockState
        {
            std::mutex mutex;
            std::condition_variable cv;
            std::atomic<bool> stop = false;
            bool hasNetworkTime = false;
            int64_t anchorUnixMs = 0;
            std::chrono::steady_clock::time_point anchorSteady;
            std::chrono::steady_clock::time_point lastSyncSteady;
            int64_t accuracyMs = std::numeric_limits<int64_t>::max();
        };
    }

    namespace
    {
        constexpr size_t NtpPacketSize = 48;
        constexpr int NtpTimeoutMs = 1500;
        constexpr int64_t MaxAcceptedRoundTripMs = 5000;
        constexpr int64_t NtpUnixEpochOffsetSeconds = 2208988800LL;
        constexpr int64_t LudusServerTimeAccuracyMs = 1000;
        constexpr auto RefreshInterval = std::chrono::minutes(10);
        constexpr auto RetryInterval = std::chrono::seconds(30);
        constexpr const char* NtpServers[] = {
            "time.cloudflare.com",
            "time.google.com",
            "pool.ntp.org",
            "time.windows.com"
        };

        struct ClockSample
        {
            int64_t unixMs;
            std::chrono::steady_clock::time_point steadyTime;
            int64_t accuracyMs;
            int64_t offsetMs;
            std::string source;
        };

        int64_t LocalUnixTimeMilliseconds()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }

        int64_t ElapsedMilliseconds(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end)
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }

        int64_t ReadNtpTimestamp(const uint8_t* bytes, size_t offset)
        {
            uint64_t seconds =
                (uint64_t(bytes[offset]) << 24) |
                (uint64_t(bytes[offset + 1]) << 16) |
                (uint64_t(bytes[offset + 2]) << 8) |
                bytes[offset + 3];
            uint64_t fraction =
                (uint64_t(bytes[offset + 4]) << 24) |
                (uint64_t(bytes[offset + 5]) << 16) |
                (uint64_t(bytes[offset + 6]) << 8) |
                bytes[offset + 7];

            int64_t unixSeconds = int64_t(seconds) - NtpUnixEpochOffsetSeconds;
            int64_t fractionMs = int64_t((fraction * 1000ULL) >> 32);
            return unixSeconds * 1000LL + fractionMs;
        }

        void WriteNtpTimestamp(uint8_t* bytes, size_t offset, int64_t unixMs)
        {
            uint64_t seconds = uint64_t(unixMs / 1000LL + NtpUnixEpochOffsetSeconds);
            uint64_t fraction = uint64_t((unixMs % 1000LL) * 0x100000000LL / 1000LL);

            bytes[offset] = uint8_t(seconds >> 24);
            bytes[offset + 1] = uint8_t(seconds >> 16);
            bytes[offset + 2] = uint8_t(seconds >> 8);
            bytes[offset + 3] = uint8_t(seconds);
            bytes[offset + 4] = uint8_t(fraction >> 24);
            bytes[offset + 5] = uint8_t(fraction >> 16);
            bytes[offset + 6] = uint8_t(fraction >> 8);
            bytes[offset + 7] = uint8_t(fraction);
        }

        bool IsValidNtpResponse(const uint8_t* response, ssize_t length)
        {
            if (!response || length < ssize_t(NtpPacketSize))
            {
                return false;
            }

            int leapIndicator = (response[0] >> 6) & 0x3;
            int mode = response[0] & 0x7;
            int stratum = response[1];
            return leapIndicator != 3 && (mode == 4 || mode == 5) && stratum > 0;
        }

        std::optional<ClockSample> QueryNtpAddress(const char* server, const addrinfo* address)
        {
            int fd = socket(address->ai_family, SOCK_DGRAM, address->ai_protocol);
            if (fd < 0)
            {
                return std::nullopt;
            }

            timeval timeout {};
            timeout.tv_sec = NtpTimeoutMs / 1000;
            timeout.tv_usec = (NtpTimeoutMs % 1000) * 1000;
            setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

            if (connect(fd, address->ai_addr, address->ai_addrlen) != 0)
            {
                close(fd);
                return std::nullopt;
            }

            uint8_t request[NtpPacketSize] = {};
            request[0] = 0x23;
            int64_t t1UnixMs = LocalUnixTimeMilliseconds();
            WriteNtpTimestamp(request, 40, t1UnixMs);

            if (send(fd, request, sizeof(request), 0) != ssize_t(sizeof(request)))
            {
                close(fd);
                return std::nullopt;
            }

            uint8_t response[NtpPacketSize * 2];
            ssize_t received = recv(fd, response, sizeof(response), 0);
            int64_t t4UnixMs = LocalUnixTimeMilliseconds();
            auto t4Steady = std::chrono::steady_clock::now();
            close(fd);

            if (!IsValidNtpResponse(response, received))
            {
                return std::nullopt;
            }

            int64_t t2UnixMs = ReadNtpTimestamp(response, 32);
            int64_t t3UnixMs = ReadNtpTimestamp(response, 40);
            int64_t roundTripMs = (t4UnixMs - t1UnixMs) - (t3UnixMs - t2UnixMs);
            if (roundTripMs < 0 || roundTripMs > MaxAcceptedRoundTripMs)
            {
                return std::nullopt;
            }

            int64_t offsetMs = ((t2UnixMs - t1UnixMs) + (t3UnixMs - t4UnixMs)) / 2;
            return ClockSample {t4UnixMs + offsetMs, t4Steady, roundTripMs, offsetMs, std::string("NTP ") + server};
        }

        std::optional<ClockSample> QueryNtpServer(const char* server, Detail::ClockState& state)
        {
            addrinfo hints {};
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_DGRAM;
            addrinfo* results = nullptr;
            if (getaddrinfo(server, "123", &hints, &results) != 0 || !results)
            {
                return std::nullopt;
            }

            std::optional<ClockSample> sample;
            for (auto* address = results; address && !sample; address = address->ai_next)
            {
                if (state.stop)
                {
                    break;
                }
                if (address->ai_family != AF_INET && address->ai_family != AF_INET6)
                {
                    continue;
                }
                sample = QueryNtpAddress(server, address);
            }

            freeaddrinfo(results);
            return sample;
        }

        void ApplySample(Detail::ClockState& state, const ClockSample& sample, bool logSync)
        {
            int64_t accuracyMs = std::max<int64_t>(0, sample.accuracyMs);
            bool applied = false;
            {
                std::lock_guard lock(state.mutex);
                int64_t ageMs = state.hasNetworkTime ? ElapsedMilliseconds(state.lastSyncSteady, sample.steadyTime) : std::numeric_limits<int64_t>::max();
                if (!state.hasNetworkTime || accuracyMs < state.accuracyMs || ageMs >= std::chrono::duration_cast<std::chrono::milliseconds>(RefreshInterval).count())
                {
                    state.hasNetworkTime = true;
                    state.anchorUnixMs = sample.unixMs;
                    state.anchorSteady = sample.steadyTime;
                    state.lastSyncSteady = sample.steadyTime;
                    state.accuracyMs = accuracyMs;
                    applied = true;
                }
            }

            if (applied && logSync)
            {
                INFO("SnoreSaber clock: synchronized with {} (offset {:+}ms, accuracy ~= {}ms).", sample.source, sample.offsetMs, accuracyMs);
            }
        }

        bool TrySynchronize(Detail::ClockState& state)
        {
            // PC queries all servers on parallel tasks; sequential keeps the sync thread simple
            std::optional<ClockSample> best;
            for (auto* server : NtpServers)
            {
                if (state.stop)
                {
                    return false;
                }

                auto sample = QueryNtpServer(server, state);
                if (!sample)
                {
                    SnoreSaber::Logging::Logger.debug("SnoreSaber clock: {} did not answer NTP", server);
                    continue;
                }
                if (!best || sample->accuracyMs < best->accuracyMs)
                {
                    best = std::move(sample);
                }
            }

            if (!best)
            {
                return false;
            }

            ApplySample(state, *best, true);
            return true;
        }

        void RefreshLoop(std::shared_ptr<Detail::ClockState> state)
        {
            bool failureLogged = false;
            while (!state->stop)
            {
                bool synchronized = TrySynchronize(*state);
                if (synchronized)
                {
                    failureLogged = false;
                }
                else if (!failureLogged)
                {
                    failureLogged = true;
                    WARN("SnoreSaber clock: NTP sync failed; using local clock until retry.");
                }

                std::unique_lock lock(state->mutex);
                if (state->cv.wait_for(lock, synchronized ? std::chrono::duration_cast<std::chrono::milliseconds>(RefreshInterval) : std::chrono::duration_cast<std::chrono::milliseconds>(RetryInterval), [&state] { return state->stop.load(); }))
                {
                    return;
                }
            }
        }
    }

    void SnoreSaberClock::ctor()
    {
        INVOKE_CTOR();
    }

    void SnoreSaberClock::Initialize()
    {
        _state = std::make_shared<Detail::ClockState>();
        SnoreSaber::Utils::Async::RunCpp([state = _state] {
            RefreshLoop(state);
        });
    }

    void SnoreSaberClock::Dispose()
    {
        if (!_state)
        {
            return;
        }

        {
            std::lock_guard lock(_state->mutex);
            _state->stop = true;
        }
        _state->cv.notify_all();
        _state = nullptr;
    }

    int64_t SnoreSaberClock::UnixTimeMilliseconds()
    {
        if (!_state)
        {
            return LocalUnixTimeMilliseconds();
        }

        bool hasNetworkTime;
        int64_t anchorUnixMs;
        std::chrono::steady_clock::time_point anchorSteady;
        {
            std::lock_guard lock(_state->mutex);
            hasNetworkTime = _state->hasNetworkTime;
            anchorUnixMs = _state->anchorUnixMs;
            anchorSteady = _state->anchorSteady;
        }

        if (!hasNetworkTime)
        {
            return LocalUnixTimeMilliseconds();
        }

        return anchorUnixMs + ElapsedMilliseconds(anchorSteady, std::chrono::steady_clock::now());
    }

    int64_t SnoreSaberClock::MonotonicMilliseconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    void SnoreSaberClock::RecordLudusServerTime(int64_t serverTimeUnixMs)
    {
        if (serverTimeUnixMs <= 0 || !_state)
        {
            return;
        }

        int64_t localUnixMs = LocalUnixTimeMilliseconds();
        ClockSample sample {serverTimeUnixMs, std::chrono::steady_clock::now(), LudusServerTimeAccuracyMs, serverTimeUnixMs - localUnixMs, "Ludus server"};
        ApplySample(*_state, sample, false);
    }
}
