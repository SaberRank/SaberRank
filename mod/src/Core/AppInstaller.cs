using SnoreSaber.Core.Presentation;
using SnoreSaber.Core.Timing;
using SnoreSaber.Features.Live.Compete.Services;
using SnoreSaber.Features.Live.Replay;
using SnoreSaber.Features.Replays;
using Zenject;

namespace SnoreSaber.Core {
    internal class AppInstaller : Installer {

        public override void InstallBindings() {
            Container.BindInstance(new SnoreSaberRuntimeInfo(Plugin.Instance.LibVersion, IPA.Utilities.UnityGame.GameVersion.SemverValue, IPA.Utilities.UnityGame.GameVersion.ToString())).AsSingle();
            Container.BindInstance(Plugin.SettingsService).AsSingle();
            Container.BindInstance(Plugin.Instance.HttpInstance).AsSingle();
            Container.BindInstance(Plugin.Instance.ReplayState).AsSingle();

            Container.Bind<SnoreSaberUIMaterials>().AsSingle();
            Container.BindInterfacesAndSelfTo<SnoreSaberClock>().AsSingle().NonLazy();
            Container.Bind<ReplayFileCodec>().AsSingle();
            Container.Bind<ReplayService>().AsSingle().NonLazy();
            Container.Bind<CompeteGameplayState>().AsSingle();
            Container.Bind<CompeteGameplayControl>().AsSingle();
            Container.BindInterfacesAndSelfTo<LiveReplayStreamingService>().AsSingle().NonLazy();
        }
    }
}
