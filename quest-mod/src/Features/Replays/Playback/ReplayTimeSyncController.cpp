#include "Features/Replays/Playback/ReplayTimeSyncController.hpp"
#include "Features/Replays/Playback/CutSoundEffectBuffer.hpp"
#include <GlobalNamespace/CallbacksInTime.hpp>
#include <GlobalNamespace/BurstSliderGameNoteController.hpp>
#include <GlobalNamespace/GameNoteController.hpp>
#include <GlobalNamespace/BombNoteController.hpp>
#include <GlobalNamespace/ObstacleController.hpp>
#include <System/Collections/Generic/Dictionary_2.hpp>
#include <System/Collections/Generic/LinkedListNode_1.hpp>
#include <UnityEngine/AudioSource.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Mathf.hpp>
#include <UnityEngine/Time.hpp>
#include <GlobalNamespace/NoteCutSoundEffect.hpp>
#include <GlobalNamespace/NoteCutSoundEffectManager.hpp>
#include "logging.hpp"

using namespace UnityEngine;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Playback, ReplayTimeSyncController);

namespace SnoreSaber::ReplaySystem::Playback
{
    namespace
    {
        template <typename TController>
        void ResetBeatmapObject(TController controller)
        {
            if (!controller)
            {
                return;
            }

            controller->Hide(false);
            controller->Pause(false);
            controller->set_enabled(true);
            controller->get_gameObject()->SetActive(true);
            controller->Dissolve(0.0f);
        }

        template <typename TControllers>
        void ResetBeatmapObjects(TControllers controllers)
        {
            if (!controllers)
            {
                return;
            }

            for (int i = 0; i < controllers->Count; i++)
            {
                ResetBeatmapObject(controllers->get_Item(i));
            }
        }
    }

    void ReplayTimeSyncController::ctor(GlobalNamespace::AudioTimeSyncController* audioTimeSyncController,
                                        GlobalNamespace::AudioTimeSyncController::InitData* audioInitData,
                                        GlobalNamespace::BasicBeatmapObjectManager* basicBeatmapObjectManager,
                                        GlobalNamespace::NoteCutSoundEffectManager* noteCutSoundEffectManager,
                                        GlobalNamespace::BeatmapCallbacksController::InitData* callbackInitData,
                                        GlobalNamespace::BeatmapCallbacksController* beatmapObjectCallbackController,
                                        Zenject::DiContainer* container)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        _audioInitData = audioInitData;
        _basicBeatmapObjectManager = basicBeatmapObjectManager;
        _noteCutSoundEffectManager = noteCutSoundEffectManager;
        _callbackInitData = callbackInitData;
        _beatmapObjectCallbackController = beatmapObjectCallbackController;

        _audioManagerSO = _noteCutSoundEffectManager->_audioManager;

        _comboPlayer = container->Resolve<ComboPlayer*>();
        _energyPlayer = container->Resolve<EnergyPlayer*>();
        _heightPlayer = container->TryResolve<HeightPlayer*>();
        _multiplierPlayer = container->Resolve<MultiplierPlayer*>();
        _notePlayer = container->Resolve<NotePlayer*>();
        _posePlayer = container->Resolve<PosePlayer*>();
        _scorePlayer = container->Resolve<ScorePlayer*>();
    }

    void ReplayTimeSyncController::Tick()
    {
        // Potential input?
    }

    void ReplayTimeSyncController::UpdateTimes()
    {
        if (!_audioTimeSyncController)
        {
            return;
        }

        if(_heightPlayer){
            _heightPlayer->TimeUpdate(_audioTimeSyncController->songTime);
        }
        if (_energyPlayer)
            _energyPlayer->TimeUpdate(_audioTimeSyncController->songTime); // needs to be run before the ScorePlayer
        if (_comboPlayer)
            _comboPlayer->TimeUpdate(_audioTimeSyncController->songTime);
        if (_multiplierPlayer)
            _multiplierPlayer->TimeUpdate(_audioTimeSyncController->songTime);
        if (_scorePlayer)
            _scorePlayer->TimeUpdate(_audioTimeSyncController->songTime);
        if (_posePlayer)
            _posePlayer->TimeUpdate(_audioTimeSyncController->songTime);
        if (_notePlayer)
            _notePlayer->TimeUpdate(_audioTimeSyncController->songTime);
    }

    void ReplayTimeSyncController::OverrideTime(float time)
    {
        if (!_audioTimeSyncController)
        {
            return;
        }

        if (std::abs(time - _audioTimeSyncController->songTime) <= 0.25f)
        {
            return;
        }

        CutSoundEffectBuffer::SetBuffering(true);
        CancelAllHitSounds();
        auto previousState = _audioTimeSyncController->state;

        _audioTimeSyncController->Pause();

        if (_basicBeatmapObjectManager)
        {
            if (_basicBeatmapObjectManager->_basicGameNotePoolContainer)
                ResetBeatmapObjects(_basicBeatmapObjectManager->_basicGameNotePoolContainer->activeItems);
            if (_basicBeatmapObjectManager->_burstSliderHeadGameNotePoolContainer)
                ResetBeatmapObjects(_basicBeatmapObjectManager->_burstSliderHeadGameNotePoolContainer->activeItems);
            if (_basicBeatmapObjectManager->_burstSliderGameNotePoolContainer)
                ResetBeatmapObjects(_basicBeatmapObjectManager->_burstSliderGameNotePoolContainer->activeItems);
            if (_basicBeatmapObjectManager->_bombNotePoolContainer)
                ResetBeatmapObjects(_basicBeatmapObjectManager->_bombNotePoolContainer->activeItems);
            ResetBeatmapObjects(_basicBeatmapObjectManager->activeObstacleControllers);
        }
        
        if (_callbackInitData)
        {
            _callbackInitData->startFilterTime = time;
        }

        if (_beatmapObjectCallbackController)
        {
            auto callbacks = _beatmapObjectCallbackController->_callbacksInTimes;
            if (callbacks)
            {
                auto itr = callbacks->GetEnumerator();
                while (itr.MoveNext())
                {
                    auto callback = itr._current.value;
                    if (callback && callback->lastProcessedNode &&
                        callback->lastProcessedNode->item && callback->lastProcessedNode->item->time > time)
                    {
                        callback->lastProcessedNode = nullptr;
                    }
                }
            }

            _beatmapObjectCallbackController->_prevSongTime = time - 0.01;
            _beatmapObjectCallbackController->_songTime = time;
            _beatmapObjectCallbackController->_startFilterTime = time;
        }

        _audioTimeSyncController->SeekTo(time / _audioTimeSyncController->timeScale);
        _audioTimeSyncController->_songTime = time;
        _audioTimeSyncController->Update();
        
        if (previousState == GlobalNamespace::AudioTimeSyncController::State::Playing)
        {
            _audioTimeSyncController->Resume();
        }
        UpdateTimes();
    }

    void ReplayTimeSyncController::OverrideTimeScale(float newScale)
    {
        if (!_audioTimeSyncController || !_audioTimeSyncController->_audioSource || !_audioInitData)
        {
            return;
        }

        CancelAllHitSounds();
        _audioTimeSyncController->_audioSource->pitch = newScale;
        _audioTimeSyncController->_timeScale = newScale;
        _audioTimeSyncController->_audioStartTimeOffsetSinceStart = (Time::get_timeSinceLevelLoad() * _audioTimeSyncController->timeScale) - (_audioTimeSyncController->songTime + _audioInitData->songTimeOffset);

        if (_audioManagerSO)
        {
            _audioManagerSO->musicPitch = 1.0f / newScale;
        }
        _audioTimeSyncController->Update();
    }

    void ReplayTimeSyncController::CancelAllHitSounds()
    {
        if (!_noteCutSoundEffectManager)
        {
            return;
        }

        auto noteCutPool = _noteCutSoundEffectManager->_noteCutSoundEffectPoolContainer;
        if (!noteCutPool || !noteCutPool->activeItems)
        {
            _noteCutSoundEffectManager->_prevNoteATime = -1.0f;
            _noteCutSoundEffectManager->_prevNoteBTime = -1.0f;
            return;
        }

        auto noteCutPoolItems = noteCutPool->activeItems;
        for (int i = 0; i < noteCutPoolItems->Count; i++)
        {
            auto effect = noteCutPoolItems->get_Item(i);
            if (effect && effect->isActiveAndEnabled)
            {
                effect->StopPlayingAndFinish();
            }
        }
        _noteCutSoundEffectManager->_prevNoteATime = -1.0f;
        _noteCutSoundEffectManager->_prevNoteBTime = -1.0f;
    }
} // namespace SnoreSaber::ReplaySystem::Playback
