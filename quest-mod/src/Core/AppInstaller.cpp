#include "Core/AppInstaller.hpp"

#include "Core/SnoreSaberRuntimeInfo.hpp"
#include "Core/Timing/SnoreSaberClock.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayControl.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayState.hpp"
#include "Features/Live/Replay/LiveReplayStreamingService.hpp"
#include <Zenject/DiContainer.hpp>
#include <Zenject/FromBinderNonGeneric.hpp>

DEFINE_TYPE(SnoreSaber::Core, AppInstaller);

namespace SnoreSaber::Core
{
    void AppInstaller::InstallBindings()
    {
        auto container = Container;
        container->Bind<Core::SnoreSaberRuntimeInfo*>()->AsSingle();
        container->BindInterfacesAndSelfTo<Timing::SnoreSaberClock*>()->AsSingle()->NonLazy();
        container->Bind<Features::Live::Compete::Services::CompeteGameplayState*>()->AsSingle();
        container->Bind<Features::Live::Compete::Services::CompeteGameplayControl*>()->AsSingle();
        container->BindInterfacesAndSelfTo<Features::Live::Replay::LiveReplayStreamingService*>()->AsSingle()->NonLazy();
    }
}
