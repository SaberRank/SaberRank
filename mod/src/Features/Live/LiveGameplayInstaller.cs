using SnoreSaber.Features.Live.Compete.Services;
using Zenject;

namespace SnoreSaber.Features.Live {
    internal class LiveGameplayInstaller : Installer {
        public override void InstallBindings() {
            Container.BindInterfacesTo<CompeteGameplayControlBinder>().AsSingle();
            Container.BindInterfacesTo<CompeteFpsStabilityStartGate>().AsSingle();
        }
    }
}
