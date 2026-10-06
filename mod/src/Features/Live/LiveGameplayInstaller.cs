using SaberRank.Features.Live.Compete.Services;
using Zenject;

namespace SaberRank.Features.Live {
    internal class LiveGameplayInstaller : Installer {
        public override void InstallBindings() {
            Container.BindInterfacesTo<CompeteGameplayControlBinder>().AsSingle();
            Container.BindInterfacesTo<CompeteFpsStabilityStartGate>().AsSingle();
        }
    }
}
