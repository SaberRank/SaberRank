#include "Features/Replays/ReplayFeatureInstaller.hpp"

#include "Features/Replays/ReplayLoader.hpp"
#include "Features/Replays/ReplayStorageService.hpp"
#include "Features/Replays/Services/ReplayQueryService.hpp"
#include "Features/Replays/UI/ResultsViewReplayButtonController.hpp"

#include <Zenject/ConcreteBinderGeneric_1.hpp>
#include <Zenject/DiContainer.hpp>

DEFINE_TYPE(SnoreSaber::ReplaySystem, ReplayFeatureInstaller);

namespace SnoreSaber::ReplaySystem
{
    void ReplayFeatureInstaller::InstallBindings()
    {
        auto container = Container;
        container->Bind<ReplayStorageService*>()->AsSingle();
        container->Bind<Services::ReplayQueryService*>()->AsSingle();
        container->Bind<ReplayLoader*>()->AsSingle();
        container->BindInterfacesTo<UI::ResultsViewReplayButtonController*>()->AsSingle();
    }
}
