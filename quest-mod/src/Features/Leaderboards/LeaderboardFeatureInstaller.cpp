#include "Features/Leaderboards/LeaderboardFeatureInstaller.hpp"

#include "Features/Leaderboards/LeaderboardBeatmapController.hpp"
#include "Features/Leaderboards/LeaderboardInteractionController.hpp"
#include "Features/Leaderboards/LeaderboardPresentationController.hpp"
#include "Features/Leaderboards/LeaderboardStatusController.hpp"
#include "Features/Leaderboards/Services/LeaderboardScreenLoader.hpp"
#include "Features/Leaderboards/Services/LeaderboardScreenSession.hpp"
#include "Features/Leaderboards/Services/LeaderboardTweeningService.hpp"
#include "Features/Leaderboards/Services/LeaderboardPlayerScoreCache.hpp"
#include "Features/Leaderboards/Services/LeaderboardQueryService.hpp"
#include "Features/Leaderboards/Services/MaxScoreCache.hpp"
#include <Zenject/ConcreteBinderGeneric_1.hpp>
#include <Zenject/ConcreteIdBinderGeneric_1.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/FromBinderNonGeneric.hpp>

DEFINE_TYPE(SnoreSaber::Features::Leaderboards, LeaderboardFeatureInstaller);

namespace SnoreSaber::Features::Leaderboards
{
    void LeaderboardFeatureInstaller::InstallBindings()
    {
        auto container = Container;
        container->Bind<SnoreSaber::Utils::MaxScoreCache*>()->AsSingle();
        container->Bind<Services::LeaderboardPlayerScoreCache*>()->AsSingle();
        container->Bind<Services::LeaderboardQueryService*>()->AsSingle();
        container->Bind<Services::LeaderboardScreenLoader*>()->AsSingle();
        container->Bind<Services::LeaderboardScreenSession*>()->AsSingle();
        container->Bind<Services::LeaderboardTweeningService*>()->AsSingle();
        container->Bind<LeaderboardBeatmapController*>()->AsSingle();
        container->Bind<LeaderboardInteractionController*>()->AsSingle();
        container->Bind<LeaderboardPresentationController*>()->AsSingle();
        container->Bind<LeaderboardStatusController*>()->AsSingle();
    }
}
