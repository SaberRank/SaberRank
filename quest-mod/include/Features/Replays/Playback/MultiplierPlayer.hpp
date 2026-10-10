#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include "Features/Replays/ReplayPlaybackContext.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/ScoreController.hpp>
#include <Zenject/DiContainer.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::ReplaySystem::Playback, MultiplierPlayer, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::AudioTimeSyncController>, _audioTimeSyncController);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::ScoreController>, _scoreController);
    DECLARE_CTOR(ctor, SnoreSaber::ReplaySystem::ReplayPlaybackContext* replayContext, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::ScoreController* scoreController);
    DECLARE_INSTANCE_METHOD(void, TimeUpdate, float songTime);
    vector<Data::Private::MultiplierEvent> _sortedMultiplierEvents;
    void UpdateMultiplier(int multiplier, float progress);
};
