#include "Features/Replays/Playback/ComboPlayer.hpp"
#include "Features/Replays/Format/ReplayFile.hpp"
#include "Features/Replays/ReplayTime.hpp"
#include <System/Action_1.hpp>
#include <UnityEngine/AnimationClip.hpp>
#include <UnityEngine/Animator.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/RuntimeAnimatorController.hpp>
#include <algorithm>
#include <limits>
#include <metacore/shared/internals.hpp>

using namespace UnityEngine;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Playback, ComboPlayer);

namespace SnoreSaber::ReplaySystem::Playback
{
    namespace
    {
        constexpr auto FullComboLostClipName = "FullComboLost";
    }

    void ComboPlayer::ctor(ReplayPlaybackContext* replayContext, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::ComboController* comboController)
    {
        INVOKE_CTOR();
        _audioTimeSyncController = audioTimeSyncController;
        _comboController = comboController;
        // _comboUIController = comboUIController;
        _comboUIController = UnityEngine::Resources::FindObjectsOfTypeAll<GlobalNamespace::ComboUIController*>()->First();
        auto replayFile = replayContext->GetReplayFile();
        _sortedNoteEvents = replayFile->noteKeyframes;
        _sortedComboEvents = replayFile->comboKeyframes;
        std::stable_sort(_sortedNoteEvents.begin(), _sortedNoteEvents.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.Time < rhs.Time;
        });
        std::stable_sort(_sortedComboEvents.begin(), _sortedComboEvents.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.Time < rhs.Time;
        });

        ComboReplayStats stats;
        int comboCounter = 0;
        for (const auto& noteEvent : _sortedNoteEvents)
        {
            if (noteEvent.EventType == NoteEventType::None)
            {
                continue;
            }

            if (ReplayTimeSearch::IsScoringNoteEvent(noteEvent))
            {
                stats.cutOrMissRecorded++;
            }
            else if (noteEvent.EventType == NoteEventType::Bomb)
            {
                if (noteEvent.SaberType == 0) {
                    stats.bombsHitL++;
                }
                else {
                    stats.bombsHitR++;
                }
            }

            if (noteEvent.EventType == NoteEventType::GoodCut)
            {
                comboCounter++;
                if (noteEvent.SaberType == 0)
                {
                    stats.leftCombo++;
                    stats.leftHighest = std::max(stats.leftHighest, stats.leftCombo);
                }
                else
                {
                    stats.rightCombo++;
                    stats.rightHighest = std::max(stats.rightHighest, stats.rightCombo);
                }
            }
            else if (noteEvent.EventType == NoteEventType::BadCut || noteEvent.EventType == NoteEventType::Miss || noteEvent.EventType == NoteEventType::Bomb)
            {
                comboCounter = 0;
                if (noteEvent.SaberType == 0) {
                    stats.leftCombo = 0;
                }
                else {
                    stats.rightCombo = 0;
                }
            }

            stats.highestCombo = std::max(stats.highestCombo, comboCounter);
            _noteEventTimes.push_back(noteEvent.Time);
            _comboStats.push_back(stats);
        }

        for (const auto& comboEvent : _sortedComboEvents)
        {
            if (comboEvent.Combo == 0)
            {
                _comboLossTimes.push_back(comboEvent.Time);
            }
        }

        auto animatorController = _comboUIController->_animator->get_runtimeAnimatorController();
        if (animatorController)
        {
            auto animationClips = animatorController->get_animationClips();
            for (auto clip : animationClips)
            {
                if (clip->name == FullComboLostClipName)
                {
                    _comboLostClip = clip;
                    break;
                }
            }
        }
    }

    void ComboPlayer::TimeUpdate(float newTime)
    {
        int nextIndex = ReplayTimeSearch::CountAtOrBefore(_sortedComboEvents, newTime, [](const auto& comboEvent) {
            return comboEvent.Time;
        });
        int combo = nextIndex > 0 ? _sortedComboEvents[nextIndex - 1].Combo : 0;
        UpdateCombo(newTime, combo);
    }


    void ComboPlayer::UpdateCombo(float time, int combo)
    {
        int previousEventCount = ReplayTimeSearch::CountBefore(_noteEventTimes, time);
        ComboReplayStats stats;
        if (previousEventCount > 0)
        {
            stats = _comboStats[previousEventCount - 1];
        }

        _comboController->_combo = combo;
        _comboController->_maxCombo = stats.highestCombo;

        if (_comboController->comboDidChangeEvent)
        {
            _comboController->comboDidChangeEvent->Invoke(combo);
        }

        bool didLoseCombo = ReplayTimeSearch::CountBefore(_comboLossTimes, time) > 0;

        auto animator = _comboUIController->_animator;
        int comboLostId = _comboUIController->_comboLostId;
        if ((combo == 0 && stats.cutOrMissRecorded == 0) || !didLoseCombo)
        {
            animator->set_enabled(true);
            animator->Rebind();
            _comboUIController->_fullComboLost = false;
        }
        else
        {
            animator->ResetTrigger(comboLostId);
            animator->set_enabled(false);
            if (_comboLostClip)
            {
                // Mathf.Epsilon; avoids the cordl static-field accessor which collides across TUs at link time
                _comboLostClip->SampleAnimation(animator->get_gameObject(), std::max(0.0f, _comboLostClip->get_length() - std::numeric_limits<float>::denorm_min()));
            }
            _comboUIController->_fullComboLost = true;
        }

        MetaCore::Internals::combo = combo;
        MetaCore::Internals::highestCombo = stats.highestCombo;
        MetaCore::Internals::leftCombo = stats.leftCombo;
        MetaCore::Internals::rightCombo = stats.rightCombo;
        MetaCore::Internals::highestLeftCombo = stats.leftHighest;
        MetaCore::Internals::highestRightCombo = stats.rightHighest;
        MetaCore::Internals::bombsLeftHit = stats.bombsHitL;
        MetaCore::Internals::bombsRightHit = stats.bombsHitR;
    }
} // namespace SnoreSaber::ReplaySystem::Playback
