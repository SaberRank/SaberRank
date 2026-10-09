using SnoreSaber.Core.Platform;
using SnoreSaber.Features.Players.Services;
using Zenject;

namespace SnoreSaber.Features.Players {
    internal class PlayersFeatureInstaller : Installer {
        public override void InstallBindings() {
            Container.BindInterfacesAndSelfTo<GamePlatformAdapter>().AsSingle();
            Container.Bind<GameSessionService>().AsSingle();
            Container.Bind<PlayerProfileService>().AsSingle();
            Container.BindInterfacesAndSelfTo<LocalPlayerPanelSession>().AsSingle().NonLazy();
            Container.Bind<GlobalPlayerQueryService>().AsSingle();
            Container.Bind<GlobalPlayerSession>().AsSingle();
        }
    }
}
