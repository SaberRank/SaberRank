#include "Features/Replays/Installers/PlaybackInstaller.hpp"

#include <GlobalNamespace/PlayerSpecificSettings.hpp>
#include "Features/Replays/Playback/ComboPlayer.hpp"
#include "Features/Replays/Playback/EnergyPlayer.hpp"
#include "Features/Replays/Playback/HeightPlayer.hpp"
#include "Features/Replays/Playback/MultiplierPlayer.hpp"
#include "Features/Replays/Playback/NotePlayer.hpp"
#include "Features/Replays/Playback/PosePlayer.hpp"
#include "Features/Replays/Playback/ReplayCutEffects.hpp"
#include "Features/Replays/Playback/ReplayTimeSyncController.hpp"
#include "Features/Replays/Playback/ScorePlayer.hpp"
#include "Features/Replays/ReplayPlaybackContext.hpp"
#include "Features/Replays/ReplayStateRegistry.hpp"
#include "Features/Replays/UI/GameReplayUI.hpp"
#include <Zenject/ConcreteBinderGeneric_1.hpp>
#include <Zenject/ConcreteIdBinderGeneric_1.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/FromBinderNonGeneric.hpp>
#include <lapiz/shared/utilities/ZenjectExtensions.hpp>
#include "logging.hpp"

DEFINE_TYPE(SnoreSaber::ReplaySystem::Installers, PlaybackInstaller);

using namespace Lapiz::Zenject::ZenjectExtensions;

namespace SnoreSaber::ReplaySystem::Installers
{
    void PlaybackInstaller::ctor(GlobalNamespace::GameplayCoreSceneSetupData* gameplayCoreSceneSetupData)
    {
        _gameplayCoreSceneSetupData = gameplayCoreSceneSetupData;
    }

    void PlaybackInstaller::InstallBindings()
    {
        if (SnoreSaber::ReplaySystem::ReplayStateRegistry::IsModernPlaybackEnabled()) {
            auto container = Container;
            Playback::ReplayCutEffects::SetReduceDebris(_gameplayCoreSceneSetupData->playerSpecificSettings->reduceDebris);
            container->Bind<ReplayPlaybackContext*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Playback::PosePlayer*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Playback::NotePlayer*>()->AsSingle();
            container->Bind<Playback::EnergyPlayer*>()->AsSingle(); // needs to be injected before the ScorePlayer to make the TimeUpdate methods run in the correct order (order fixed in case we can ever use interfaces)
            container->BindInterfacesAndSelfTo<Playback::ScorePlayer*>()->AsSingle();
            if(_gameplayCoreSceneSetupData->playerSpecificSettings->automaticPlayerHeight)
                container->BindInterfacesAndSelfTo<Playback::HeightPlayer*>()->AsSingle();

            container->Bind<Playback::ComboPlayer*>()->AsSingle();
            container->Bind<Playback::MultiplierPlayer*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Playback::ReplayTimeSyncController*>()->AsSingle();
            FromNewComponentOnNewGameObject(container->Bind<UI::GameReplayUI*>())->AsSingle()->NonLazy();
        }
    }
} // namespace SnoreSaber::ReplaySystem::Installers
