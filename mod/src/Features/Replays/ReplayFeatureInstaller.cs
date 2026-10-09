using SnoreSaber.Features.Replays.Services;
using SnoreSaber.Features.Replays.UI;
using Zenject;

namespace SnoreSaber.Features.Replays {
    internal class ReplayFeatureInstaller : Installer {
        public override void InstallBindings() {
            Container.Bind<ReplayLoader>().AsSingle().NonLazy();
            Container.Bind<ReplayStorageService>().AsSingle();
            Container.Bind<ReplayQueryService>().AsSingle();
            Container.BindInterfacesTo<ResultsViewReplayButtonController>().AsSingle();
            Container.BindInterfacesTo<ReplayXrEventHandler>().AsSingle().NonLazy();
        }
    }
}
