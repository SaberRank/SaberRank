using SnoreSaber.Features.MainMenu.Settings.ViewControllers;
using SnoreSaber.Features.MainMenu.MainFlow;
using SnoreSaber.Features.MainMenu.MainFlow.FAQ;
using SnoreSaber.Features.MainMenu.MainFlow.GlobalLeaderboard;
using SnoreSaber.Features.MainMenu.MainFlow.Teams.UI;
using SnoreSaber.Features.MainMenu.Settings;
using Zenject;

namespace SnoreSaber.Features.MainMenu {
    internal class MainMenuFeatureInstaller : Installer {
        public override void InstallBindings() {
            Container.Bind<TeamViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<FAQViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<GlobalViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<GlobalLeaderboardHost>().AsSingle();
            Container.Bind<SnoreSaberMenuNavigator>().AsSingle();
            Container.Bind<MainSettingsViewController>().FromNewComponentAsViewController().AsSingle();

            Container.BindInterfacesAndSelfTo<SnoreSaberFlowCoordinator>().FromNewComponentOnNewGameObject().AsSingle();
            Container.BindInterfacesAndSelfTo<SnoreSaberSettingsFlowCoordinator>().FromNewComponentOnNewGameObject().AsSingle();
        }
    }
}
