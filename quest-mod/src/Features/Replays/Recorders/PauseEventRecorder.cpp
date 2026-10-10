#include "Features/Replays/Recorders/PauseEventRecorder.hpp"

#include <System/Action.hpp>
#include <UnityEngine/Time.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <custom-types/shared/delegate.hpp>

using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Recorders, PauseEventRecorder);

namespace SnoreSaber::ReplaySystem::Recorders
{
    namespace
    {
        int64_t UnixTimeMilliseconds()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }
    }

    void PauseEventRecorder::ctor(AudioTimeSyncController* audioTimeSyncController, Zenject::DiContainer* container)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        auto gamePause = container->TryResolve<IGamePause*>();
        _gamePause = gamePause ? il2cpp_utils::try_cast<GamePause>(gamePause).value_or(nullptr) : nullptr;
    }

    void PauseEventRecorder::Initialize()
    {
        if (_gamePause) {
            didPauseDelegate = { &PauseEventRecorder::GamePause_didPauseEvent, this };
            didResumeDelegate = { &PauseEventRecorder::GamePause_didResumeEvent, this };
            _gamePause->___didPauseEvent += didPauseDelegate;
            _gamePause->___didResumeEvent += didResumeDelegate;
        }
    }

    void PauseEventRecorder::Dispose()
    {
        if (_gamePause) {
            if (didPauseDelegate) {
                _gamePause->___didPauseEvent -= didPauseDelegate;
            }
            if (didResumeDelegate) {
                _gamePause->___didResumeEvent -= didResumeDelegate;
            }
        }
    }

    void PauseEventRecorder::GamePause_didPauseEvent()
    {
        if (_paused) {
            return;
        }

        _paused = true;
        _pauseSongTime = _audioTimeSyncController->songTime;
        _pauseRealtime = Time::get_realtimeSinceStartup();
        _pauseUnixStartTime = UnixTimeMilliseconds();
    }

    void PauseEventRecorder::GamePause_didResumeEvent()
    {
        FinishOpenPause();
    }

    void PauseEventRecorder::FinishOpenPause()
    {
        if (!_paused) {
            return;
        }

        int64_t duration = std::max<int64_t>(0, (int64_t)std::llround((Time::get_realtimeSinceStartup() - _pauseRealtime) * 1000.0f));
        int64_t unixEndTime = std::max(_pauseUnixStartTime, UnixTimeMilliseconds());
        _pauseEvents.push_back(PauseEvent{_pauseSongTime, duration, _pauseUnixStartTime, unixEndTime});
        _paused = false;
    }

    vector<PauseEvent> PauseEventRecorder::Export()
    {
        FinishOpenPause();
        return _pauseEvents;
    }
} // namespace SnoreSaber::ReplaySystem::Recorders
