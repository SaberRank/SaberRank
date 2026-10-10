#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/MainCamera.hpp>
#include "Features/Replays/UI/Components/AmeBar.hpp"
#include "Features/Replays/UI/Components/AmeNode.hpp"
#include <Zenject/DiContainer.hpp>
#include <GlobalNamespace/SaberManager.hpp>
#include "Features/Replays/Playback/PosePlayer.hpp"
#include <System/IDisposable.hpp>
#include <Zenject/IInitializable.hpp>
#include <Zenject/ITickable.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::ReplaySystem::UI,
        ImberSpecsReporter,
        System::Object,
        Zenject::IInitializable*,
        System::IDisposable*) {
    DECLARE_INSTANCE_FIELD(SnoreSaber::ReplaySystem::Playback::PosePlayer*, _posePlayer);
    DECLARE_INSTANCE_FIELD(UnityW<GlobalNamespace::SaberManager>, _saberManager);
    DECLARE_CTOR(ctor, SnoreSaber::ReplaySystem::Playback::PosePlayer* posePlayer, GlobalNamespace::SaberManager* _saberManager);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);
public:
    void PosePlayer_DidUpdatePose(SnoreSaber::Data::Private::VRPoseGroup pose);
    std::function<void(int, float, float)> DidReport;
};