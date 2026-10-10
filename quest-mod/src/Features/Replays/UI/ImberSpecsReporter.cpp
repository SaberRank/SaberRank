#include "Features/Replays/UI/ImberSpecsReporter.hpp"
#include <GlobalNamespace/Saber.hpp>
#include <GlobalNamespace/SaberMovementData.hpp>
#include <UnityEngine/Mathf.hpp>
#include <UnityEngine/RectTransformUtility.hpp>
#include <UnityEngine/Vector2.hpp>
#include "logging.hpp"

using namespace UnityEngine;
using namespace GlobalNamespace;

DEFINE_TYPE(SnoreSaber::ReplaySystem::UI, ImberSpecsReporter);

namespace SnoreSaber::ReplaySystem::UI
{
    void ImberSpecsReporter::ctor(SnoreSaber::ReplaySystem::Playback::PosePlayer* posePlayer, SaberManager* saberManager)
    {
        INVOKE_CTOR();
        _posePlayer = posePlayer;
        _saberManager = saberManager;
    }
    void ImberSpecsReporter::Initialize()
    {
        SafePtr<ImberSpecsReporter> self(this);
        _posePlayer->AddCallback([self](SnoreSaber::Data::Private::VRPoseGroup pose) {
            self->PosePlayer_DidUpdatePose(pose);
        });
    }
    void ImberSpecsReporter::PosePlayer_DidUpdatePose(SnoreSaber::Data::Private::VRPoseGroup pose)
    {
        if (DidReport)
        {
            DidReport(pose.FPS, _saberManager->leftSaber->movementDataForLogic->bladeSpeed, _saberManager->rightSaber->movementDataForLogic->bladeSpeed);
        }
    }
    void ImberSpecsReporter::Dispose()
    {
        _posePlayer->AddCallback(nullptr);
    }
} // namespace SnoreSaber::ReplaySystem::UI