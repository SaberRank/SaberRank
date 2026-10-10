#include "Features/Live/LiveGameplayInstaller.hpp"

#include "Features/Live/Compete/Services/CompeteFpsStabilityStartGate.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayControl.hpp"

#include <Zenject/DiContainer.hpp>
#include <Zenject/FromBinderNonGeneric.hpp>
#include <lapiz/shared/utilities/ZenjectExtensions.hpp>

DEFINE_TYPE(SnoreSaber::Features::Live, LiveGameplayInstaller);

using namespace Lapiz::Zenject::ZenjectExtensions;

namespace SnoreSaber::Features::Live
{
    void LiveGameplayInstaller::InstallBindings()
    {
        auto container = Container;
        container->BindInterfacesTo<Compete::Services::CompeteGameplayControlBinder*>()->AsSingle();
        container->BindInterfacesTo<Compete::Services::CompeteFpsStabilityStartGate*>()->AsSingle();
    }
}
