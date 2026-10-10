#pragma once

#include "Features/Live/Compete/Services/CompeteGameplayState.hpp"
#include "Utils/DelegateUtils.hpp"

#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/ScoreController.hpp>
#include <GlobalNamespace/SongController.hpp>
#include <System/Action.hpp>
#include <System/IDisposable.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/IInitializable.hpp>
#include <Zenject/ITickable.hpp>
#include <custom-types/shared/macros.hpp>

#include <string>

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::Features::Live::Compete::Services,
        CompeteFpsStabilityStartGate,
        System::Object,
        Zenject::IInitializable*,
        Zenject::ITickable*,
        System::IDisposable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(CompeteGameplayState*, _gameplayState);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::AudioTimeSyncController>, _audioTimeSyncController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::SongController>, _songController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::ScoreController>, _scoreController);
    DECLARE_CTOR(ctor, CompeteGameplayState* gameplayState, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::SongController* songController, Zenject::DiContainer* container);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Tick, &::Zenject::ITickable::Tick);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);

private:
    static constexpr float StabilityDurationSeconds = 0.3f;
    static constexpr float MaxWaitSeconds = 5.0f;
    static constexpr float FallbackRefreshRate = 80.0f;

    bool _waitingForInitialStart = true;
    bool _initialPausePending = false;
    bool _waitingForStableFps = false;
    bool _scoreControllerWasEnabled = false;
    bool _scoreControllerStateCaptured = false;
    int _initialPauseFrame = 0;
    float _stableSeconds = 0.0f;
    float _waitSeconds = 0.0f;
    float _fpsThreshold = 0.0f;

    DelegateUtils::DelegateW<System::Action> _stateChangedDelegate;

    void AudioTimeSyncControllerStateChangedEvent();
    void StartStableFpsWait();
    void ResumeSong(const std::string& reason);
    void CancelStableFpsWait(const std::string& reason);
    void RestoreScoreController();
    bool IsAudioPlaying();
    bool IsAudioPaused();
    static float RecommendedFpsThreshold();
};
