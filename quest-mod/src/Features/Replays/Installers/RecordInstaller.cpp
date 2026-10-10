#include "Features/Replays/Installers/RecordInstaller.hpp"

#include "Features/Live/Compete/Services/CompetePauseGuard.hpp"
#include <GlobalNamespace/PlayerSpecificSettings.hpp>
#include "Features/Replays/Recorders/MainRecorder.hpp"
#include "Features/Replays/ReplayStateRegistry.hpp"
#include "Features/Replays/UI/GameReplayUI.hpp"
#include <Zenject/ConcreteBinderGeneric_1.hpp>
#include <Zenject/ConcreteIdBinderGeneric_1.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/FromBinderNonGeneric.hpp>
#include <lapiz/shared/utilities/ZenjectExtensions.hpp>
#include "logging.hpp"

DEFINE_TYPE(SnoreSaber::ReplaySystem::Installers, RecordInstaller);

using namespace Lapiz::Zenject::ZenjectExtensions;

namespace SnoreSaber::ReplaySystem::Installers
{
    void RecordInstaller::InstallBindings()
    {
        if (!SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled()) {
            auto container = Container;
            container->BindInterfacesAndSelfTo<Recorders::MetadataRecorder*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Recorders::HeightEventRecorder*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Recorders::NoteEventRecorder*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Recorders::PoseRecorder*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Recorders::ScoreEventRecorder*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Recorders::EnergyEventRecorder*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Recorders::PauseEventRecorder*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Recorders::WallEventRecorder*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Recorders::HsvConfigRecorder*>()->AsSingle();
            container->BindInterfacesAndSelfTo<Recorders::MainRecorder*>()->AsSingle();
            container->BindInterfacesTo<SnoreSaber::Features::Live::Compete::Services::CompetePauseGuard*>()->AsSingle();
        }
    }
} // namespace SnoreSaber::ReplaySystem::Installers
