#include "Features/Replays/Recorders/PoseRecorder.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/Time.hpp>
#include "Utils/StringUtils.hpp"
#include "logging.hpp"

using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Recorders, PoseRecorder);

namespace SnoreSaber::ReplaySystem::Recorders
{
    void PoseRecorder::ctor(AudioTimeSyncController* audioTimeSyncController, PlayerTransforms* playerTransforms, SnoreSaber::Features::Live::Replay::LiveReplayStreamingService* liveReplayStreamingService)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        _playerTransforms = playerTransforms;
        _liveReplayStreamingService = liveReplayStreamingService;
        _recording = true;
    }

    void PoseRecorder::Tick()
    {
        if(!_recording)
            return;

        Vector3 headPos = _playerTransforms->headPseudoLocalPos;
        Quaternion headRot = _playerTransforms->headPseudoLocalRot;

        Vector3 leftControllerPosition = _playerTransforms->leftHandPseudoLocalPos;
        Quaternion leftControllerRotation = _playerTransforms->leftHandPseudoLocalRot;

        Vector3 rightControllerPosition = _playerTransforms->rightHandPseudoLocalPos;
        Quaternion rightControllerRotation = _playerTransforms->rightHandPseudoLocalRot;

        int fps = (int)(1 / Time::get_unscaledDeltaTime());
        _vrPoseGroup.push_back(VRPoseGroup(
            VRPose(VRPosition(headPos.x, headPos.y,
                              headPos.z),
                   VRRotation(headRot.x,
                              headRot.y, headRot.z, headRot.w)),
            VRPose(VRPosition(leftControllerPosition.x, leftControllerPosition.y,
                              leftControllerPosition.z),
                   VRRotation(leftControllerRotation.x,
                              leftControllerRotation.y, leftControllerRotation.z, leftControllerRotation.w)),
            VRPose(VRPosition(rightControllerPosition.x, rightControllerPosition.y, rightControllerPosition.z),
                   VRRotation(rightControllerRotation.x, rightControllerRotation.y, rightControllerRotation.z,
                              rightControllerRotation.w)),
            fps, _audioTimeSyncController->songTime));
        _liveReplayStreamingService->RecordPose(_vrPoseGroup.back());
    }

    void PoseRecorder::StopRecording() {
        _recording = false;
    }

    vector<VRPoseGroup> PoseRecorder::Export()
    {
        return _vrPoseGroup;
    }
} // namespace SnoreSaber::ReplaySystem::Recorders