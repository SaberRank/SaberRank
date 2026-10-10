#include "hooks.hpp"

#include <GlobalNamespace/BeatmapObjectManager.hpp>
#include <GlobalNamespace/BeatmapObjectSpawnController.hpp>
#include <GlobalNamespace/BeatmapObjectSpawnMovementData.hpp>
#include <GlobalNamespace/ComboController.hpp>
#include <GlobalNamespace/EnvironmentInfoSO.hpp>
#include <GlobalNamespace/GameNoteController.hpp>
#include <GlobalNamespace/GameplayCoreSceneSetupData.hpp>
#include <GlobalNamespace/GameplayModifiersModelSO.hpp>
#include <GlobalNamespace/GoodCutScoringElement.hpp>
#include <GlobalNamespace/LevelCompletionResults.hpp>
#include <GlobalNamespace/MainCamera.hpp>
#include <GlobalNamespace/NoteDebrisSpawner.hpp>
#include <GlobalNamespace/NoteController.hpp>
#include <GlobalNamespace/NoteCutSoundEffectManager.hpp>
#include <GlobalNamespace/PauseController.hpp>
#include <GlobalNamespace/PlayerHeightDetector.hpp>
#include <GlobalNamespace/PlayerSpecificSettings.hpp>
#include <GlobalNamespace/PlayerTransforms.hpp>
#include <GlobalNamespace/PrepareLevelCompletionResults.hpp>
#include <GlobalNamespace/RankModel.hpp>
#include <GlobalNamespace/RelativeScoreAndImmediateRankCounter.hpp>
#include <GlobalNamespace/SaberManager.hpp>
#include <GlobalNamespace/ScoreController.hpp>
#include <GlobalNamespace/IGameEnergyCounter.hpp>
#include <GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp>
#include <GlobalNamespace/StandardGameplayInstaller.hpp>
#include <GlobalNamespace/ScoreModel.hpp>
#include <GlobalNamespace/StandardLevelGameplayManager.hpp>
#include <GlobalNamespace/UnityXRHelper.hpp>
#include <GlobalNamespace/VariableMovementDataProvider.hpp>
#include "Data/Private/Settings.hpp"
#include "Features/Replays/Playback/ReplayCutEffects.hpp"
#include "Features/Replays/Playback/ReplayPlaybackRegistry.hpp"
#include "Features/Replays/Playback/ReplayNoteMissEventGuard.hpp"
#include "Features/Replays/Playback/ReplaySaberVisibility.hpp"
#include "Features/Replays/ReplayStateRegistry.hpp"
#include <System/Action.hpp>
#include <System/Action_1.hpp>
#include <System/Action_2.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/WaitForEndOfFrame.hpp>
#include <bsml/shared/BSML/SharedCoroutineStarter.hpp>
#include <custom-types/shared/coroutine.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/MemoryPoolIdInitialSizeMaxSizeBinder_1.hpp>
#include <Zenject/MemoryPool_1.hpp>
#include <Zenject/SceneContext.hpp>
#include <GlobalNamespace/FlyingScoreSpawner.hpp>
#include "Features/Replays/Playback/CutSoundEffectBuffer.hpp"
#include "logging.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

using namespace UnityEngine;
using namespace Zenject;
using namespace GlobalNamespace;
using namespace SnoreSaber::Data::Private;
using namespace SnoreSaber::ReplaySystem;

namespace
{
    bool _scoreControllerLateUpdateRunning = false;

    struct ScoreControllerLateUpdateScope
    {
        bool acquired = false;

        ScoreControllerLateUpdateScope()
        {
            if (!_scoreControllerLateUpdateRunning)
            {
                _scoreControllerLateUpdateRunning = true;
                acquired = true;
            }
        }

        ~ScoreControllerLateUpdateScope()
        {
            if (acquired)
            {
                _scoreControllerLateUpdateRunning = false;
            }
        }
    };
}

namespace SnoreSaber::ReplaySystem::Playback::CutSoundEffectBuffer
{
    namespace
    {
        bool _buffering = false;
        bool _bufferCoroutineRunning = false;
        bool _flushing = false;
        std::uint64_t _generation = 0;
        UnityW<NoteCutSoundEffectManager> _spawnEffectManager;
        std::vector<UnityW<NoteController>> _effects;
    }

    custom_types::Helpers::Coroutine FlushBufferedNoteSpawns(std::uint64_t generation);

    void Reset()
    {
        ++_generation;
        _spawnEffectManager = nullptr;
        _effects.clear();
        _bufferCoroutineRunning = false;
        _flushing = false;
        _buffering = false;
    }

    void SetBuffering(bool value)
    {
        _buffering = value;
        if (!value)
        {
            _effects.clear();
        }
    }

    bool IsBuffering()
    {
        return _buffering;
    }

    bool IsCoroutineRunning()
    {
        return _bufferCoroutineRunning;
    }

    bool IsFlushing()
    {
        return _flushing;
    }

    void SetCoroutineRunning(bool value)
    {
        _bufferCoroutineRunning = value;
    }

    void SetFlushing(bool value)
    {
        _flushing = value;
    }

    void ResetForManager(NoteCutSoundEffectManager* manager)
    {
        ++_generation;
        _spawnEffectManager = manager;
        _effects.clear();
        _bufferCoroutineRunning = false;
        _flushing = false;
        _buffering = false;
    }

    std::uint64_t CurrentGeneration()
    {
        return _generation;
    }

    bool IsCurrentGeneration(std::uint64_t generation)
    {
        return generation == _generation;
    }

    NoteCutSoundEffectManager* CurrentManager()
    {
        if (!_spawnEffectManager.isAlive())
        {
            Reset();
            return nullptr;
        }

        return _spawnEffectManager.ptr();
    }

    bool IsKnownManager(NoteCutSoundEffectManager* manager)
    {
        return CurrentManager() == manager;
    }

    bool Enqueue(NoteController* noteController)
    {
        if (!noteController)
        {
            return false;
        }

        std::erase_if(_effects, [](const auto& effect) {
            return !effect.isAlive();
        });

        auto existing = std::find_if(_effects.begin(), _effects.end(), [noteController](const auto& effect) {
            return effect.isAlive() && effect.ptr() == noteController;
        });
        if (existing != _effects.end())
        {
            return false;
        }

        _effects.emplace_back(noteController);
        return true;
    }

    bool HasQueuedEffects()
    {
        return !_effects.empty();
    }

    UnityW<NoteController> PopEffect()
    {
        if (_effects.empty())
        {
            return nullptr;
        }

        UnityW<NoteController> noteController = _effects.front();
        _effects.erase(_effects.begin());
        return noteController;
    }

    NoteCutSoundEffectManager* Manager()
    {
        return CurrentManager();
    }
}

// Player Hooks

MAKE_AUTO_HOOK_MATCH(SaberManager_Update, &SaberManager::Update, void, SaberManager* self)
{
    SaberManager_Update(self);

    if (SnoreSaber::ReplaySystem::ReplayStateRegistry::IsModernPlaybackEnabled())
    {
        SnoreSaber::ReplaySystem::Playback::ReplaySaberVisibility::EnsureVisible(self);
    }
}

MAKE_AUTO_HOOK_MATCH(NoteDebrisSpawner_SpawnDebris, &NoteDebrisSpawner::SpawnDebris, void, NoteDebrisSpawner* self,
                     NoteData::GameplayType noteGameplayType, Vector3 cutPoint, Vector3 cutNormal, float saberSpeed,
                     Vector3 saberDir, Vector3 notePos, Quaternion noteRotation, Vector3 noteScale, ColorType colorType,
                     float timeToNextColorNote, Vector3 moveVec)
{
    if (SnoreSaber::ReplaySystem::Playback::ReplayCutEffects::ShouldSuppressDebris())
    {
        return;
    }

    NoteDebrisSpawner_SpawnDebris(self, noteGameplayType, cutPoint, cutNormal, saberSpeed, saberDir, notePos, noteRotation, noteScale,
                                  colorType, timeToNextColorNote, moveVec);
}

MAKE_AUTO_HOOK_MATCH(NoteCutSoundEffectManager_HandleNoteWasSpawned, &NoteCutSoundEffectManager::HandleNoteWasSpawned,
                     void, NoteCutSoundEffectManager* self, NoteController* noteController)
{
    if (!ReplayStateRegistry::IsPlaybackEnabled())
    {
        NoteCutSoundEffectManager_HandleNoteWasSpawned(self, noteController);
        return;
    }

    using namespace SnoreSaber::ReplaySystem::Playback::CutSoundEffectBuffer;
    if (IsFlushing())
    {
        NoteCutSoundEffectManager_HandleNoteWasSpawned(self, noteController);
        return;
    }

    if (!IsKnownManager(self))
    {
        ResetForManager(self);
        NoteCutSoundEffectManager_HandleNoteWasSpawned(self, noteController);
        return;
    }

    if (!IsBuffering())
    {
        NoteCutSoundEffectManager_HandleNoteWasSpawned(self, noteController);
        return;
    }

    if (!Enqueue(noteController))
    {
        NoteCutSoundEffectManager_HandleNoteWasSpawned(self, noteController);
        return;
    }

    if (!IsCoroutineRunning())
    {
        auto generation = CurrentGeneration();
        SetCoroutineRunning(true);
        BSML::SharedCoroutineStarter::StartCoroutine(custom_types::Helpers::CoroutineHelper::New(FlushBufferedNoteSpawns(generation)));
    }
}

namespace SnoreSaber::ReplaySystem::Playback::CutSoundEffectBuffer
{
    custom_types::Helpers::Coroutine FlushBufferedNoteSpawns(std::uint64_t generation)
    {
        while (IsCurrentGeneration(generation) && ReplayStateRegistry::IsPlaybackEnabled() && HasQueuedEffects())
        {
            auto noteController = PopEffect();
            auto manager = Manager();
            if (manager && noteController.isAlive())
            {
                SetFlushing(true);
                Hook_NoteCutSoundEffectManager_HandleNoteWasSpawned::NoteCutSoundEffectManager_HandleNoteWasSpawned(manager, noteController.ptr());
                SetFlushing(false);
            }

            co_yield reinterpret_cast<System::Collections::IEnumerator*>(UnityEngine::WaitForEndOfFrame::New_ctor());
        }

        if (IsCurrentGeneration(generation))
        {
            SetFlushing(false);
            SetBuffering(false);
            SetCoroutineRunning(false);
        }
        co_return;
    }
}

MAKE_AUTO_HOOK_MATCH(GoodCutScoringElement_Init, &::GlobalNamespace::GoodCutScoringElement::Init, void, GlobalNamespace::GoodCutScoringElement* self, GlobalNamespace::NoteCutInfo noteCutInfo)
{
    GoodCutScoringElement_Init(self, noteCutInfo);

    if (SnoreSaber::ReplaySystem::ReplayStateRegistry::IsModernPlaybackEnabled())
    {
        if (auto notePlayer = SnoreSaber::ReplaySystem::Playback::ReplayPlaybackRegistry::GetNotePlayer())
        {
            notePlayer->ForceCompleteGoodScoringElements(self, noteCutInfo, self->_cutScoreBuffer);
        }
    }
}

// quest stand-in for PC AntiInterrupt (prefix on PauseController.HandleHMDUnmounted).
// HandleHMDUnmounted is a 4-byte tail-call stub and can't be trampoline-hooked,
// so suppress the unmounted handlers at the raiser instead.
MAKE_AUTO_HOOK_MATCH(UnityXRHelper_set_userPresence, &UnityXRHelper::set_userPresence, void, UnityXRHelper* self, bool value)
{
    if (!SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled())
    {
        UnityXRHelper_set_userPresence(self, value);
        return;
    }

    auto unmountedHandlers = self->hmdUnmountedEvent;
    self->hmdUnmountedEvent = nullptr;
    UnityXRHelper_set_userPresence(self, value);
    self->hmdUnmountedEvent = unmountedHandlers;
}

MAKE_AUTO_HOOK_MATCH(GameNoteController_HandleCut_Event, &GameNoteController::HandleCut,
                     void, GameNoteController* self, GlobalNamespace::Saber* saber, UnityEngine::Vector3 cutPoint, UnityEngine::Quaternion orientation, UnityEngine::Vector3 cutDirVec, bool allowBadCut)
{
    if (SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled())
    {
        return;
    }

    GameNoteController_HandleCut_Event(self, saber, cutPoint, orientation, cutDirVec, allowBadCut);
}

MAKE_AUTO_HOOK_MATCH(BeatmapObjectManager_HandleNoteControllerNoteWasMissed, &BeatmapObjectManager::HandleNoteControllerNoteWasMissed,
                     void, BeatmapObjectManager* self, GlobalNamespace::NoteController* noteController)
{
    if (SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled() && !SnoreSaber::ReplaySystem::Playback::ReplayNoteMissEventGuard::IsAllowed(noteController))
    {
        return;
    }

    BeatmapObjectManager_HandleNoteControllerNoteWasMissed(self, noteController);
}

MAKE_AUTO_HOOK_MATCH(RelativeScoreAndImmediateRankCounter_UpdateRelativeScoreAndImmediateRank, &RelativeScoreAndImmediateRankCounter::UpdateRelativeScoreAndImmediateRank,
                     void, RelativeScoreAndImmediateRankCounter* self, int score, int modifiedScore, int maxPossibleScore, int maxPossibleModifiedScore)
{
    if (!SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled())
    {
        RelativeScoreAndImmediateRankCounter_UpdateRelativeScoreAndImmediateRank(self, score, modifiedScore, maxPossibleScore, maxPossibleModifiedScore);
        return;
    }

    if (score == 0 && maxPossibleScore == 0)
    {
        self->relativeScore = 1.0f;
        self->immediateRank = RankModel::Rank::SS;
        if (self->relativeScoreOrImmediateRankDidChangeEvent)
            self->relativeScoreOrImmediateRankDidChangeEvent->Invoke();
        return;
    }

    RelativeScoreAndImmediateRankCounter_UpdateRelativeScoreAndImmediateRank(self, score, modifiedScore, maxPossibleScore, maxPossibleModifiedScore);
}

MAKE_AUTO_HOOK_MATCH(PrepareLevelCompletionResults_FillLevelCompletionResults, &PrepareLevelCompletionResults::FillLevelCompletionResults,
                     LevelCompletionResults*, PrepareLevelCompletionResults* self, LevelCompletionResults::LevelEndStateType levelEndStateType, LevelCompletionResults::LevelEndAction levelEndAction)
{
    if (SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled())
    {
        levelEndStateType = LevelCompletionResults::LevelEndStateType::Incomplete;
    }
    auto value = PrepareLevelCompletionResults_FillLevelCompletionResults(self, levelEndStateType, levelEndAction);
    return value;
}

// CancelScoreControllerBufferFinisher
MAKE_AUTO_HOOK_MATCH(ScoreController_LateUpdate, &ScoreController::LateUpdate, void, ScoreController* self)
{
    if (!SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled())
    {
        ScoreController_LateUpdate(self);
        return;
    }

    ScoreControllerLateUpdateScope runningScope;
    if (!runningScope.acquired)
    {
        return;
    }

    if (!self || !self->_sortedNoteTimesWithoutScoringElements ||
        !self->_sortedScoringElementsWithoutMultiplier || !self->_scoringElementsWithMultiplier ||
        !self->_scoringElementsToRemove || !self->_audioTimeSyncController ||
        !self->_scoreMultiplierCounter || !self->_maxScoreMultiplierCounter ||
        !self->_gameplayModifiersModel || !self->_gameEnergyCounter)
    {
        return;
    }

    float num = (self->_sortedNoteTimesWithoutScoringElements->Count > 0) ? self->_sortedNoteTimesWithoutScoringElements->get_Item(0) : 3.4028235E+38f;
    float num2 = self->_audioTimeSyncController->songTime + 0.15f;

    int num3 = 0;
    bool flag = false;
    for (int i = 0; i < self->_sortedScoringElementsWithoutMultiplier->Count; i++)
    {
        auto scoringElement = self->_sortedScoringElementsWithoutMultiplier->get_Item(i);
        if (!scoringElement)
        {
            num3++;
            continue;
        }

        if (scoringElement->time >= num2 && scoringElement->time <= num)
        {
            break;
        }
        flag |= self->_scoreMultiplierCounter->ProcessMultiplierEvent(scoringElement->multiplierEventType);
        if (scoringElement->wouldBeCorrectCutBestPossibleMultiplierEventType == ScoreMultiplierCounter::MultiplierEventType::Positive)
        {
            self->_maxScoreMultiplierCounter->ProcessMultiplierEvent(ScoreMultiplierCounter::MultiplierEventType::Positive);
        }
        scoringElement->SetMultipliers(self->_scoreMultiplierCounter->multiplier, self->_maxScoreMultiplierCounter->multiplier);
        self->_scoringElementsWithMultiplier->Add(scoringElement);
        num3++;
    }
    self->_sortedScoringElementsWithoutMultiplier->RemoveRange(0, num3);
    if (flag)
    {
        if (self->multiplierDidChangeEvent)
        {
            self->multiplierDidChangeEvent->Invoke(self->_scoreMultiplierCounter->multiplier, self->_scoreMultiplierCounter->normalizedProgress);
        }
    }
    bool flag2 = false;
    self->_scoringElementsToRemove->Clear();
    for (int j = 0; j < self->_scoringElementsWithMultiplier->Count; j++)
    {
        auto scoringElement2 = self->_scoringElementsWithMultiplier->get_Item(j);
        if (!scoringElement2)
        {
            self->_scoringElementsToRemove->Add(scoringElement2);
            continue;
        }

        if (scoringElement2->isFinished)
        {
            if ((float)scoringElement2->maxPossibleCutScore > 0.0f)
            {
                flag2 = true;
                // self->multipliedScore += scoringElement2->cutScore * scoringElement2->multiplier;
                // self->immediateMaxPossibleMultipliedScore += scoringElement2->maxPossibleCutScore * scoringElement2->maxMultiplier;
            }
            self->_scoringElementsToRemove->Add(scoringElement2);
            bool canNotifyFinished = scoringElement2->noteData;
            if (auto goodCut = il2cpp_utils::try_cast<GoodCutScoringElement>(scoringElement2).value_or(nullptr))
            {
                canNotifyFinished = scoringElement2->noteData &&
                                    goodCut->_cutScoreBuffer &&
                                    goodCut->_cutScoreBuffer->noteCutInfo.noteData;
            }

            if (canNotifyFinished && self->scoringForNoteFinishedEvent)
            {
                self->scoringForNoteFinishedEvent->Invoke(scoringElement2);
            }
        }
    }
    for (int k = 0; k < self->_scoringElementsToRemove->Count; k++)
    {
        auto scoringElement3 = self->_scoringElementsToRemove->get_Item(k);
        if (scoringElement3)
        {
            self->DespawnScoringElement(scoringElement3);
        }
        self->_scoringElementsWithMultiplier->Remove(scoringElement3);
    }
    self->_scoringElementsToRemove->Clear();
    float totalMultiplier = self->_gameplayModifiersModel->GetTotalMultiplier(self->_gameplayModifierParams, self->_gameEnergyCounter->energy);
    if (self->_prevMultiplierFromModifiers != totalMultiplier)
    {
        self->_prevMultiplierFromModifiers = totalMultiplier;
        flag2 = true;
    }
    if (flag2)
    {
        self->_modifiedScore = ScoreModel::GetModifiedScoreForGameplayModifiersScoreMultiplier(self->multipliedScore, totalMultiplier);
        self->_immediateMaxPossibleModifiedScore = ScoreModel::GetModifiedScoreForGameplayModifiersScoreMultiplier(self->immediateMaxPossibleMultipliedScore, totalMultiplier);
        if (self->scoreDidChangeEvent)
        {
            self->scoreDidChangeEvent->Invoke(self->multipliedScore, self->modifiedScore);
        }
    }
}


// fixes scores doubling up in one position when note cuts are done on the same frame (time)
MAKE_AUTO_HOOK_MATCH(FlyingScoreSpawner_SpawnFlyingScoreNextFrame, &GlobalNamespace::FlyingScoreSpawner::SpawnFlyingScoreNextFrame, void, GlobalNamespace::FlyingScoreSpawner* self, GlobalNamespace::IReadonlyCutScoreBuffer* cutScoreBuffer, UnityEngine::Color color)
{
    if (!SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled())
    {
        FlyingScoreSpawner_SpawnFlyingScoreNextFrame(self, cutScoreBuffer, color);
        return;
    }
    self->SpawnFlyingScore(cutScoreBuffer, color);
    return;
}

// quest stand-in for PC ReplayJumpDistanceTweak (Affinity prefix on VariableMovementDataProvider.Init)
MAKE_AUTO_HOOK_MATCH(VariableMovementDataProvider_Init, &VariableMovementDataProvider::Init,
                     void, VariableMovementDataProvider* self,
                     float startHalfJumpDurationInBeats, float maxHalfJumpDistance, float noteJumpMovementSpeed, float minRelativeNoteJumpSpeed, float bpm,
                     BeatmapObjectSpawnMovementData_NoteJumpValueType noteJumpValueType, float noteJumpValue,
                     UnityEngine::Vector3 centerPosition, UnityEngine::Vector3 forwardVector)
{
    if (SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled() && Settings::useRecordedPlayerSettings)
    {
        auto loadedReplayFile = SnoreSaber::ReplaySystem::ReplayStateRegistry::Current.loadedReplayFile;
        if (loadedReplayFile && loadedReplayFile->metadata->HasPlaySettingsExtension &&
            loadedReplayFile->metadata->JumpDistance > 0.0f && noteJumpMovementSpeed > 0.0f)
        {
            noteJumpValueType = BeatmapObjectSpawnMovementData::NoteJumpValueType::JumpDuration;
            noteJumpValue = loadedReplayFile->metadata->JumpDistance / noteJumpMovementSpeed / 2.0f;
        }
    }

    VariableMovementDataProvider_Init(self, startHalfJumpDurationInBeats, maxHalfJumpDistance, noteJumpMovementSpeed, minRelativeNoteJumpSpeed, bpm, noteJumpValueType, noteJumpValue, centerPosition, forwardVector);
}
