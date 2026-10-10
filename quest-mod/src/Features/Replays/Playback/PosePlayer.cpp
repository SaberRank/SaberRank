#include "Features/Replays/Playback/PosePlayer.hpp"
#include "Features/Replays/Playback/ReplaySaberVisibility.hpp"
#include "Features/Replays/ReplayTime.hpp"
#include <GlobalNamespace/Saber.hpp>
#include <GlobalNamespace/SettingsManager.hpp>
#include <GlobalNamespace/VRController.hpp>
#include <UnityEngine/Component.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Mathf.hpp>
#include <UnityEngine/Object.hpp>
#include <UnityEngine/Quaternion.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/StereoTargetEyeMask.hpp>
#include <UnityEngine/Time.hpp>
#include <UnityEngine/Transform.hpp>
#include <UnityEngine/Vector3.hpp>
#include <UnityEngine/SpatialTracking/TrackedPoseDriver.hpp>

#include <GlobalNamespace/SaberManager.hpp>
#include <UnityEngine/Vector3.hpp>
#include "logging.hpp"
#include <algorithm>

using namespace UnityEngine;
using namespace UnityEngine::SpatialTracking;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Playback, PosePlayer);

namespace SnoreSaber::ReplaySystem::Playback
{
    void PosePlayer::ctor(ReplayPlaybackContext* replayContext, GlobalNamespace::MainCamera* mainCamera, GlobalNamespace::SaberManager* saberManager, GlobalNamespace::IReturnToMenuController* returnToMenuController, GlobalNamespace::PlayerTransforms* playerTransforms, GlobalNamespace::SettingsManager* settingsManager, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController)
    {
        INVOKE_CTOR();
        _mainCamera = mainCamera;
        _saberManager = saberManager;
        _sortedPoses = replayContext->GetReplayFile()->poseKeyframes;
        std::stable_sort(_sortedPoses.begin(), _sortedPoses.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.Time < rhs.Time;
        });
        _returnToMenuController = returnToMenuController;
        _spectatorOffset = Vector3(0, 0, -2);
        _playerTransforms = playerTransforms;
        _settingsManager = settingsManager;
        _audioTimeSyncController = audioTimeSyncController;
    }
    void PosePlayer::Initialize()
    {
        SetupCameras();
        ReplaySaberVisibility::EnsureVisible(_saberManager);
        if (_saberManager && _saberManager->leftSaber)
        {
            auto leftController = _saberManager->leftSaber->transform->GetComponentInParent<GlobalNamespace::VRController*>();
            if (leftController)
                leftController->enabled = false;
        }
        if (_saberManager && _saberManager->rightSaber)
        {
            auto rightController = _saberManager->rightSaber->transform->GetComponentInParent<GlobalNamespace::VRController*>();
            if (rightController)
                rightController->enabled = false;
        }
    }
    void PosePlayer::SetupCameras()
    {
        if (!_mainCamera)
        {
            ERROR("Cannot set up replay cameras without a main camera");
            return;
        }

        _desktopCamera = Resources::FindObjectsOfTypeAll<Camera*>()->First([](Camera* camera) {
            return camera->name == "RecorderCamera";
        });
        if (!_desktopCamera)
        {
            ERROR("Cannot set up replay cameras without RecorderCamera");
            return;
        }
        if (!_settingsManager)
        {
            ERROR("Cannot set up replay spectator camera without settings");
            return;
        }

        _mainCamera->enabled = false;
        _mainCamera->gameObject->SetActive(false);
        _desktopCamera->fieldOfView = 65.0f;
        auto desktopCameraTransform = _desktopCamera->transform;
        desktopCameraTransform->position = Vector3(desktopCameraTransform->position.x, desktopCameraTransform->position.y, desktopCameraTransform->position.z);
        // _desktopCamera->gameObject->SetActive(true);
        _desktopCamera->tag = "MainCamera";
        _desktopCamera->depth = 1;

        //_mainCamera->camera = _desktopCamera;

        GameObject* spectatorObject = GameObject::New_ctor("SpectatorParent");
        _spectatorCamera = UnityEngine::Object::Instantiate(_desktopCamera);

        spectatorObject->transform->position = Vector3(_settingsManager->settings.room.center.x + _spectatorOffset.x, _settingsManager->settings.room.center.y + _spectatorOffset.y, _settingsManager->settings.room.center.z + _spectatorOffset.z);
        Quaternion rotation = Quaternion::Euler(0.0f, _settingsManager->settings.room.rotation, 0.0f);
        spectatorObject->transform->rotation = rotation;
        _spectatorCamera->stereoTargetEye = StereoTargetEyeMask::Both;

        // copy headset tracking to spectator camera
        auto oldTrackedPoseDriver = _mainCamera->gameObject->GetComponent<TrackedPoseDriver*>();
        auto newTrackedPoseDriver = _spectatorCamera->gameObject->AddComponent<TrackedPoseDriver*>();
        if (oldTrackedPoseDriver && newTrackedPoseDriver)
        {
            newTrackedPoseDriver->UseRelativeTransform = oldTrackedPoseDriver->UseRelativeTransform;
            newTrackedPoseDriver->deviceType = oldTrackedPoseDriver->deviceType;
            newTrackedPoseDriver->m_Device = oldTrackedPoseDriver->m_Device;
            newTrackedPoseDriver->m_OriginPose = oldTrackedPoseDriver->m_OriginPose;
            newTrackedPoseDriver->m_PoseProviderComponent = oldTrackedPoseDriver->m_PoseProviderComponent;
            newTrackedPoseDriver->m_PoseSource = oldTrackedPoseDriver->m_PoseSource;
            newTrackedPoseDriver->m_TrackingType = oldTrackedPoseDriver->m_TrackingType;
            newTrackedPoseDriver->m_UpdateType = oldTrackedPoseDriver->m_UpdateType;
            newTrackedPoseDriver->m_UseRelativeTransform = oldTrackedPoseDriver->m_UseRelativeTransform;
            newTrackedPoseDriver->originPose = oldTrackedPoseDriver->originPose;
            newTrackedPoseDriver->poseProviderComponent = oldTrackedPoseDriver->poseProviderComponent;
            newTrackedPoseDriver->poseSource = oldTrackedPoseDriver->poseSource;
            newTrackedPoseDriver->trackingType = oldTrackedPoseDriver->trackingType;
            newTrackedPoseDriver->updateType = oldTrackedPoseDriver->updateType;
        }
        else
        {
            ERROR("Replay spectator camera is missing tracked pose driver data");
        }


        _spectatorCamera->gameObject->SetActive(true);
        _spectatorCamera->depth = 0;
        _spectatorCamera->transform->SetParent(spectatorObject->transform);
    }
    void PosePlayer::Tick()
    {
        if (ReachedEnd()) {
            _returnToMenuController->ReturnToMenu();
            return;
        }
        while (_audioTimeSyncController->songTime >= _sortedPoses[_nextIndex].Time)
        {
            _nextIndex++;
            if (ReachedEnd()) {
                return;
            }
        }
        if (_nextIndex > 0) {
            UpdatePoses(_sortedPoses[_nextIndex - 1], _sortedPoses[_nextIndex]);
        }
    }
    void PosePlayer::Dispose()
    {
    }
    void PosePlayer::UpdatePoses(Data::Private::VRPoseGroup activePose, Data::Private::VRPoseGroup nextPose)
    {
        float lerpTime = (_audioTimeSyncController->songTime - activePose.Time) / Mathf::Max(0.0001f, nextPose.Time - activePose.Time);

        ReplaySaberVisibility::EnsureVisible(_saberManager);

        _playerTransforms->_headTransform->SetPositionAndRotation(VRVector3(activePose.Head.Position), VRQuaternion(activePose.Head.Rotation));

        _saberManager->leftSaber->OverridePositionAndRotation(Vector3::Lerp(VRVector3(activePose.Left.Position), VRVector3(nextPose.Left.Position), lerpTime),
                                                              Quaternion::Lerp(VRQuaternion(activePose.Left.Rotation), VRQuaternion(nextPose.Left.Rotation), lerpTime));

        _saberManager->rightSaber->OverridePositionAndRotation(Vector3::Lerp(VRVector3(activePose.Right.Position), VRVector3(nextPose.Right.Position), lerpTime),
                                                               Quaternion::Lerp(VRQuaternion(activePose.Right.Rotation), VRQuaternion(nextPose.Right.Rotation), lerpTime));

        auto pos = Vector3::Lerp(VRVector3(activePose.Head.Position), VRVector3(nextPose.Head.Position), lerpTime);
        auto rot = Quaternion::Lerp(VRQuaternion(activePose.Head.Rotation), VRQuaternion(nextPose.Head.Rotation), lerpTime);

        auto eulerAngles = rot.eulerAngles;
        // TODO: Apply rotation offset

        float t2 = 4.0f == 0.0f ? 1.0f : Time::get_deltaTime() * 6.0f;
        if (_desktopCamera)
        {
            _desktopCamera->transform->SetPositionAndRotation(Vector3::Lerp(_desktopCamera->transform->position, pos, t2), Quaternion::Lerp(_desktopCamera->transform->rotation, rot, t2));
        }

        // TODO: Move camera

        if (DidUpdatePose)
        {
            DidUpdatePose(activePose);
        }
    }

    bool PosePlayer::ReachedEnd()
    {
        return _nextIndex >= _sortedPoses.size();
    }

    void PosePlayer::TimeUpdate(float newTime)
    {
        _nextIndex = ReplayTimeSearch::CountAtOrBefore(_sortedPoses, newTime, [](const auto& pose) {
            return pose.Time;
        });
        if (!ReachedEnd()) {
            Tick();
        }
    }

    void PosePlayer::SetSpectatorOffset(Vector3 value) {
        if (_spectatorCamera && _spectatorCamera->transform->parent && _settingsManager)
        {
            _spectatorCamera->transform->parent->position = Vector3(_settingsManager->settings.room.center.x + value.x, _settingsManager->settings.room.center.y + value.y, _settingsManager->settings.room.center.z + value.z);
        }
        _spectatorOffset = value;
    }

    void PosePlayer::AddCallback(std::function<void(SnoreSaber::Data::Private::VRPoseGroup)> callback)
    {
        DidUpdatePose = callback;
    }
} // namespace SnoreSaber::ReplaySystem::Playback
