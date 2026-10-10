#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include "Features/Replays/ReplayPlaybackContext.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/BasicBeatmapObjectManager.hpp>
#include <GlobalNamespace/BombNoteController.hpp>
#include <GlobalNamespace/CutScoreBuffer.hpp>
#include <GlobalNamespace/GameNoteController.hpp>
#include <GlobalNamespace/GoodCutScoringElement.hpp>
#include <GlobalNamespace/MemoryPoolContainer_1.hpp>
#include <GlobalNamespace/NoteController.hpp>
#include <GlobalNamespace/NoteCutInfo.hpp>
#include <GlobalNamespace/NoteData.hpp>
#include <GlobalNamespace/SaberManager.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/ITickable.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <vector>

struct RecognizedNoteCutInfo
{
    RecognizedNoteCutInfo(GlobalNamespace::NoteCutInfo info, SnoreSaber::Data::Private::NoteEvent event)
        : info(info), event(event)
    {
    }
    GlobalNamespace::NoteCutInfo info;
    SnoreSaber::Data::Private::NoteEvent event;
};

struct NoteReplayStats
{
    int notesLeftBadCut = 0;
    int notesRightBadCut = 0;
    int notesLeftMissed = 0;
    int notesRightMissed = 0;
    int notesLeftSeen = 0;
    int notesRightSeen = 0;
};

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::ReplaySystem::Playback,
        NotePlayer,
        System::Object,
        Zenject::ITickable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(int, _nextIndex);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::SaberManager>, _saberManager);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::AudioTimeSyncController>, _audioTimeSyncController);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::BasicBeatmapObjectManager*, _basicBeatmapObjectManager);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::MemoryPoolContainer_1<UnityW<GlobalNamespace::GameNoteController>>*, _gameNotePool);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::MemoryPoolContainer_1<UnityW<GlobalNamespace::GameNoteController>>*, _burstSliderHeadNotePool);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::MemoryPoolContainer_1<UnityW<GlobalNamespace::BurstSliderGameNoteController>>*, _burstSliderNotePool);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::MemoryPoolContainer_1<UnityW<GlobalNamespace::BombNoteController>>*, _bombNotePool);
    DECLARE_INSTANCE_METHOD(void, ForceCompleteGoodScoringElements, GlobalNamespace::GoodCutScoringElement* scoringElement, GlobalNamespace::NoteCutInfo noteCutInfo, GlobalNamespace::CutScoreBuffer* cutScoreBuffer);
    DECLARE_CTOR(ctor, SnoreSaber::ReplaySystem::ReplayPlaybackContext* replayContext, GlobalNamespace::SaberManager* saberManager, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::BasicBeatmapObjectManager* basicBeatmapObjectManager);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Tick, &::Zenject::ITickable::Tick);
    DECLARE_INSTANCE_METHOD(void, TimeUpdate, float songTime);
    std::vector<Data::Private::NoteEvent> _sortedNoteEvents;
    std::vector<NoteReplayStats> _noteStats;
    std::vector<RecognizedNoteCutInfo> _recognizedNoteCutInfos;
    std::shared_ptr<Data::Private::ReplayFile> _replayFile;
    int _totalNotesLeft;
    int _totalNotesRight;
    void ProcessEvent(Data::Private::NoteEvent &activeEvent);
    bool HandleEvent(Data::Private::NoteEvent &activeEvent, GlobalNamespace::NoteController* noteController);
    bool HandleMissEvent(Data::Private::NoteEvent &activeEvent, GlobalNamespace::NoteController* noteController);
    bool DoesNoteMatchID(Data::Private::NoteID &id, GlobalNamespace::NoteData* noteData);
};
