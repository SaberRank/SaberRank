#include "MainInstaller.hpp"

#include "Features/Leaderboards/Multiplayer/SnoreSaberMultiplayerInitializer.hpp"
#include "Features/Leaderboards/Multiplayer/SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager.hpp"
#include "Features/Leaderboards/Multiplayer/SnoreSaberMultiplayerResultsLeaderboardFlowManager.hpp"
#include "Features/Leaderboards/LeaderboardFeatureInstaller.hpp"
#include "Features/Live/LiveFeatureInstaller.hpp"
#include "Features/MainMenu/MainMenuFeatureInstaller.hpp"
#include "Features/Players/PlayersFeatureInstaller.hpp"
#include "Features/Replays/ReplayFeatureInstaller.hpp"
#include "Features/ScoreSubmission/ScoreSubmissionFeatureInstaller.hpp"
#include "Core/BeatSaver/BeatSaverService.hpp"
#include "Core/Configuration/SettingsService.hpp"
#include "Core/Presentation/RemoteImageService.hpp"
#include "Core/Presentation/SnoreSaberUIMaterials.hpp"
#include <Zenject/DiContainer.hpp>
#include <Zenject/FromBinderNonGeneric.hpp>

DEFINE_TYPE(SnoreSaber, MainInstaller);

namespace SnoreSaber
{
    void MainInstaller::InstallBindings()
    {
        auto container = Container;
        container->Bind<Core::Configuration::SettingsService*>()->AsSingle();
        container->Bind<Core::BeatSaver::BeatSaverService*>()->AsSingle();
        container->BindInterfacesAndSelfTo<Core::Presentation::RemoteImageService*>()->AsSingle();
        container->Bind<Core::Presentation::SnoreSaberUIMaterials*>()->AsSingle();
        container->Install<Features::Players::PlayersFeatureInstaller*>();
        container->Install<ReplaySystem::ReplayFeatureInstaller*>();
        container->Install<Features::Leaderboards::LeaderboardFeatureInstaller*>();
        container->Install<Features::Live::LiveFeatureInstaller*>();
        container->Install<Features::MainMenu::MainMenuFeatureInstaller*>();
        container->Install<Features::ScoreSubmission::ScoreSubmissionFeatureInstaller*>();
        container->BindInterfacesTo<UI::Multiplayer::SnoreSaberMultiplayerInitializer*>()->AsSingle();
        container->BindInterfacesTo<UI::Multiplayer::SnoreSaberMultiplayerLevelSelectionLeaderboardFlowManager*>()->AsSingle();
        container->BindInterfacesTo<UI::Multiplayer::SnoreSaberMultiplayerResultsLeaderboardFlowManager*>()->AsSingle();
    }
} // namespace SnoreSaber
