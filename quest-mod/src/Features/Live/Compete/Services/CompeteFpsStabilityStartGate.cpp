#include "Features/Live/Compete/Services/CompeteFpsStabilityStartGate.hpp"

#include "logging.hpp"

#include <GlobalNamespace/OVRPlugin.hpp>
#include <UnityEngine/Time.hpp>

#include <cmath>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::Services, CompeteFpsStabilityStartGate);

namespace SnoreSaber::Features::Live::Compete::Services
{
    void CompeteFpsStabilityStartGate::ctor(CompeteGameplayState* gameplayState, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::SongController* songController, Zenject::DiContainer* container)
    {
        INVOKE_CTOR();
        _gameplayState = gameplayState;
        _audioTimeSyncController = audioTimeSyncController;
        _songController = songController;
        // pc uses [InjectOptional]; quest resolves through the container
        _scoreController = container->TryResolve<GlobalNamespace::ScoreController*>();
    }

    void CompeteFpsStabilityStartGate::Initialize()
    {
        _fpsThreshold = RecommendedFpsThreshold();
        _stateChangedDelegate = { &CompeteFpsStabilityStartGate::AudioTimeSyncControllerStateChangedEvent, this };
        _audioTimeSyncController->___stateChangedEvent += _stateChangedDelegate;
    }

    void CompeteFpsStabilityStartGate::Dispose()
    {
        if (_audioTimeSyncController)
        {
            _audioTimeSyncController->___stateChangedEvent -= _stateChangedDelegate;
        }
        RestoreScoreController();
    }

    void CompeteFpsStabilityStartGate::Tick()
    {
        if (_initialPausePending)
        {
            if (UnityEngine::Time::get_frameCount() <= _initialPauseFrame)
            {
                return;
            }

            StartStableFpsWait();
            return;
        }

        if (!_waitingForStableFps)
        {
            return;
        }

        if (!_gameplayState->IsLiveGameplayActive())
        {
            CancelStableFpsWait("live gameplay ended before FPS stabilized.");
            return;
        }

        if (!IsAudioPaused())
        {
            CancelStableFpsWait("playback state changed before FPS stabilized; leaving song alone.");
            return;
        }

        float deltaTime = UnityEngine::Time::get_unscaledDeltaTime();
        if (deltaTime <= 0.0f)
        {
            return;
        }

        _waitSeconds += deltaTime;
        float fps = 1.0f / deltaTime;
        if (fps >= _fpsThreshold)
        {
            _stableSeconds += deltaTime;
        }
        else
        {
            _stableSeconds = 0.0f;
        }

        if (_stableSeconds >= StabilityDurationSeconds)
        {
            ResumeSong(fmt::format("FPS stabilized at {:.0f}.", fps));
        }
        else if (_waitSeconds >= MaxWaitSeconds)
        {
            ResumeSong(fmt::format("FPS did not stabilize within {:g}s; starting anyway.", MaxWaitSeconds));
        }
    }

    void CompeteFpsStabilityStartGate::AudioTimeSyncControllerStateChangedEvent()
    {
        if (!_waitingForInitialStart || !_gameplayState->IsLiveGameplayActive() || !IsAudioPlaying())
        {
            return;
        }

        _waitingForInitialStart = false;
        _initialPauseFrame = UnityEngine::Time::get_frameCount();
        _initialPausePending = true;
    }

    void CompeteFpsStabilityStartGate::StartStableFpsWait()
    {
        _initialPausePending = false;
        if (!_gameplayState->IsLiveGameplayActive() || !IsAudioPlaying())
        {
            INFO("Ludus: Skipping FPS start gate because playback is no longer starting.");
            _gameplayState->MarkMapStartReady();
            return;
        }

        _stableSeconds = 0.0f;
        _waitSeconds = 0.0f;
        _scoreControllerWasEnabled = _scoreController ? _scoreController->get_enabled() : false;
        _scoreControllerStateCaptured = static_cast<bool>(_scoreController);

        if (_scoreController)
        {
            _scoreController->set_enabled(false);
        }

        _songController->PauseSong();
        if (!IsAudioPaused())
        {
            RestoreScoreController();
            WARN("Ludus: Skipping FPS start gate because the start pause did not take.");
            _gameplayState->MarkMapStartReady();
            return;
        }

        _waitingForStableFps = true;
        INFO("Ludus: Waiting for stable FPS before live map start (target {:.0f}+ FPS).", _fpsThreshold);
    }

    void CompeteFpsStabilityStartGate::ResumeSong(const std::string& reason)
    {
        if (!_waitingForStableFps)
        {
            return;
        }

        _waitingForStableFps = false;
        RestoreScoreController();
        if (IsAudioPaused())
        {
            _songController->ResumeSong();
        }
        else
        {
            WARN("Ludus: FPS start gate ended while playback was not paused; leaving song alone.");
        }

        _gameplayState->MarkMapStartReady();
        INFO("Ludus: {}", reason);
    }

    void CompeteFpsStabilityStartGate::CancelStableFpsWait(const std::string& reason)
    {
        if (!_waitingForStableFps)
        {
            return;
        }

        _waitingForStableFps = false;
        RestoreScoreController();
        _gameplayState->MarkMapStartReady();
        WARN("Ludus: Canceling FPS start gate: {}", reason);
    }

    void CompeteFpsStabilityStartGate::RestoreScoreController()
    {
        if (_scoreController && _scoreControllerStateCaptured)
        {
            _scoreController->set_enabled(_scoreControllerWasEnabled);
            _scoreControllerStateCaptured = false;
        }
    }

    bool CompeteFpsStabilityStartGate::IsAudioPlaying()
    {
        return _audioTimeSyncController->state == GlobalNamespace::AudioTimeSyncController::State::Playing;
    }

    bool CompeteFpsStabilityStartGate::IsAudioPaused()
    {
        return _audioTimeSyncController->state == GlobalNamespace::AudioTimeSyncController::State::Paused;
    }

    float CompeteFpsStabilityStartGate::RecommendedFpsThreshold()
    {
        // pc reads XRDevice.refreshRate; quest asks OVRPlugin for the hmd display frequency
        float refreshRate = GlobalNamespace::OVRPlugin::get_systemDisplayFrequency();
        if (refreshRate <= 0.0f)
        {
            refreshRate = FallbackRefreshRate;
        }

        return std::max(1.0f, std::round(std::round(refreshRate) / 5.0f) * 5.0f - 5.0f);
    }
}
