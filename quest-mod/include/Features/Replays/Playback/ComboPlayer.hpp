#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include "Features/Replays/ReplayPlaybackContext.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/ComboController.hpp>
#include <GlobalNamespace/ComboUIController.hpp>
#include <UnityEngine/AnimationClip.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <vector>

struct ComboReplayStats
{
    int cutOrMissRecorded = 0;
    int bombsHitL = 0;
    int bombsHitR = 0;
    int leftCombo = 0;
    int rightCombo = 0;
    int leftHighest = 0;
    int rightHighest = 0;
    int highestCombo = 0;
};

DECLARE_CLASS_CODEGEN(SnoreSaber::ReplaySystem::Playback, ComboPlayer, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::AudioTimeSyncController>, _audioTimeSyncController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::ComboController>, _comboController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::ComboUIController>, _comboUIController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::AnimationClip>, _comboLostClip);
    DECLARE_CTOR(ctor, SnoreSaber::ReplaySystem::ReplayPlaybackContext* replayContext, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::ComboController* comboController);
    DECLARE_INSTANCE_METHOD(void, TimeUpdate, float songTime);
    vector<Data::Private::NoteEvent> _sortedNoteEvents;
    vector<Data::Private::ComboEvent> _sortedComboEvents;
    std::vector<float> _noteEventTimes;
    std::vector<float> _comboLossTimes;
    std::vector<ComboReplayStats> _comboStats;
    void UpdateCombo(float time, int combo);
};
