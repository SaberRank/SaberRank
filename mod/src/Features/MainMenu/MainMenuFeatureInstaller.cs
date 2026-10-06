using SaberRank.Features.MainMenu.Settings.ViewControllers;
using SaberRank.Features.MainMenu.MainFlow;
using SaberRank.Features.MainMenu.MainFlow.FAQ;
using SaberRank.Features.MainMenu.MainFlow.GlobalLeaderboard;
using SaberRank.Features.MainMenu.MainFlow.Teams.UI;
using SaberRank.Features.MainMenu.Settings;
using Zenject;

namespace SaberRank.Features.MainMenu {
    internal class MainMenuFeatureInstaller : Installer {
        public override void InstallBindings() {
            Container.Bind<TeamViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<FAQViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<GlobalViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<GlobalLeaderboardHost>().AsSingle();
            Container.Bind<SaberRankMenuNavigator>().AsSingle();
            Container.Bind<MainSettingsViewController>().FromNewComponentAsViewController().AsSingle();

            Container.BindInterfacesAndSelfTo<SaberRankFlowCoordinator>().FromNewComponentOnNewGameObject().AsSingle();
            Container.BindInterfacesAndSelfTo<SaberRankSettingsFlowCoordinator>().FromNewComponentOnNewGameObject().AsSingle();
        }
    }
}
