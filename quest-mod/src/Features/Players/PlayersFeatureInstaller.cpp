#include "Features/Players/PlayersFeatureInstaller.hpp"

#include "Features/Players/Services/GameSessionService.hpp"
#include "Features/Players/Services/GlobalPlayerQueryService.hpp"
#include "Features/Players/Services/GlobalPlayerSession.hpp"
#include "Features/Players/Services/LocalPlayerPanelSession.hpp"
#include "Features/Players/Services/PlayerProfileService.hpp"

#include <Zenject/ConcreteBinderGeneric_1.hpp>
#include <Zenject/DiContainer.hpp>

DEFINE_TYPE(SnoreSaber::Features::Players, PlayersFeatureInstaller);

namespace SnoreSaber::Features::Players
{
    void PlayersFeatureInstaller::InstallBindings()
    {
        Container->Bind<Services::GameSessionService*>()->AsSingle();
        Container->Bind<Services::GlobalPlayerQueryService*>()->AsSingle();
        Container->Bind<Services::GlobalPlayerSession*>()->AsSingle();
        Container->Bind<Services::LocalPlayerPanelSession*>()->AsSingle();
        Container->Bind<Services::PlayerProfileService*>()->AsSingle();
    }
}
