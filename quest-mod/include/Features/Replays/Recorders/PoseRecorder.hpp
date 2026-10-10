#pragma once

#include "Features/Live/Replay/LiveReplayStreamingService.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/PlayerTransforms.hpp>
#include <UnityEngine/Quaternion.hpp>
#include <UnityEngine/Vector3.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/ITickable.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <vector>

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::ReplaySystem::Recorders,
        PoseRecorder,
        System::Object,
        Zenject::ITickable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::AudioTimeSyncController>, _audioTimeSyncController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::PlayerTransforms>, _playerTransforms);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Live::Replay::LiveReplayStreamingService*, _liveReplayStreamingService);
    DECLARE_CTOR(ctor, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::PlayerTransforms* playerTransforms, SnoreSaber::Features::Live::Replay::LiveReplayStreamingService* liveReplayStreamingService);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Tick, &::Zenject::ITickable::Tick);
    std::vector<Data::Private::VRPoseGroup> _vrPoseGroup;
    bool _recording;
public:
    std::vector<Data::Private::VRPoseGroup> Export();
    void StopRecording();
};
