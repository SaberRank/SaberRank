#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include "Features/Replays/ReplayPlaybackContext.hpp"
#include <Zenject/DiContainer.hpp>

#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/GamePause.hpp>
#include "Features/Replays/Playback/PosePlayer.hpp"
#include "Features/Replays/Playback/ReplayTimeSyncController.hpp"
#include "Features/Replays/UI/ImberScrubber.hpp"
#include "Features/Replays/UI/ImberSpecsReporter.hpp"
#include "Features/Replays/UI/ImberUIPositionController.hpp"
#include "Features/Replays/UI/MainImberPanelView.hpp"
#include "Features/Replays/UI/SpectateAreaController.hpp"
#include <UnityEngine/Quaternion.hpp>
#include <UnityEngine/Vector3.hpp>
#include <UnityEngine/XR/XRNode.hpp>

#include <Zenject/IInitializable.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include "Utils/DelegateUtils.hpp"

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::ReplaySystem::UI,
        ImberManager,
        System::Object,
        Zenject::IInitializable*,
        System::IDisposable*) {
    DECLARE_INSTANCE_FIELD(float, _initialTimeScale);
    DECLARE_INSTANCE_FIELD(GlobalNamespace::GamePause*, _gamePause);
    DECLARE_INSTANCE_FIELD(SnoreSaber::ReplaySystem::UI::ImberScrubber*, _imberScrubber);
    DECLARE_INSTANCE_FIELD(SnoreSaber::ReplaySystem::UI::ImberSpecsReporter*, _imberSpecsReporter);
    DECLARE_INSTANCE_FIELD(UnityW<SnoreSaber::ReplaySystem::UI::MainImberPanelView>, _mainImberPanelView);
    DECLARE_INSTANCE_FIELD(SnoreSaber::ReplaySystem::UI::SpectateAreaController*, _spectateAreaController);
    DECLARE_INSTANCE_FIELD(UnityW<GlobalNamespace::AudioTimeSyncController>, _audioTimeSyncController);
    DECLARE_INSTANCE_FIELD(SnoreSaber::ReplaySystem::Playback::ReplayTimeSyncController*, _replayTimeSyncController);
    DECLARE_INSTANCE_FIELD(SnoreSaber::ReplaySystem::UI::ImberUIPositionController*, _imberUIPositionController);
    DECLARE_INSTANCE_FIELD(GlobalNamespace::AudioTimeSyncController::InitData*, _initData);
    DECLARE_INSTANCE_FIELD(SnoreSaber::ReplaySystem::Playback::PosePlayer*, _posePlayer);

    DECLARE_CTOR(ctor,
        GlobalNamespace::IGamePause* gamePause,
        SnoreSaber::ReplaySystem::ReplayPlaybackContext* replayContext,
        SnoreSaber::ReplaySystem::UI::ImberScrubber* imberScrubber,
        SnoreSaber::ReplaySystem::UI::ImberSpecsReporter* imberSpecsReporter,
        SnoreSaber::ReplaySystem::UI::MainImberPanelView* mainImberPanelView,
        SnoreSaber::ReplaySystem::UI::SpectateAreaController* spectateAreaController,
        GlobalNamespace::AudioTimeSyncController* audioTimeSyncController,
        SnoreSaber::ReplaySystem::Playback::ReplayTimeSyncController* replayTimeSyncController,
        SnoreSaber::ReplaySystem::UI::ImberUIPositionController* imberUIPositionController,
        GlobalNamespace::AudioTimeSyncController::InitData* initData,
        SnoreSaber::ReplaySystem::Playback::PosePlayer* posePlayer);

    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);

    DECLARE_INSTANCE_METHOD(void, MainImberPanelView_DidHandSwitchEvent, UnityEngine::XR::XRNode hand);
    DECLARE_INSTANCE_METHOD(void, GamePause_didResumeEvent);
    DECLARE_INSTANCE_METHOD(void, ImberSpecsReporter_DidReport, int fps, float leftSaberSpeed, float rightSaberSpeed);
    DECLARE_INSTANCE_METHOD(void, SpectateAreaController_DidUpdatePlayerSpectatorPose, UnityEngine::Vector3 position, UnityEngine::Quaternion rotation);
    DECLARE_INSTANCE_METHOD(void, CreateWatermark);

    // UI Callbacks
    DECLARE_INSTANCE_METHOD(void, MainImberPanelView_DidPositionTabVisibilityChange, bool value);
    DECLARE_INSTANCE_METHOD(void, MainImberPanelView_DidPositionPreviewChange, StringW value);
    DECLARE_INSTANCE_METHOD(void, MainImberPanelView_DidPositionJump);
    DECLARE_INSTANCE_METHOD(void, ImberScrubber_DidCalculateNewTime, float time);
    DECLARE_INSTANCE_METHOD(void, MainImberPanelView_DidClickLoop);
    // DECLARE_INSTANCE_METHOD(void, MainImberPanelView_DidClickLogo);
    DECLARE_INSTANCE_METHOD(void, MainImberPanelView_DidClickRestart);
    DECLARE_INSTANCE_METHOD(void, MainImberPanelView_DidClickPausePlay);
    DECLARE_INSTANCE_METHOD(void, MainImberPanelView_DidTimeSyncChange, float value);
    DECLARE_INSTANCE_METHOD(void, MainImberPanelView_DidChangeVisibility, bool value);

    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);
    DelegateUtils::DelegateW<System::Action> _didResumeDelegate;
    std::vector<std::string> _positions;
};
