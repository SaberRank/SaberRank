using SaberRank.Core.Presentation;
using SaberRank.Core.Timing;
using SaberRank.Features.Live.Compete.Services;
using SaberRank.Features.Live.Replay;
using SaberRank.Features.Replays;
using Zenject;

namespace SaberRank.Core {
    internal class AppInstaller : Installer {

        public override void InstallBindings() {
            Container.BindInstance(new SaberRankRuntimeInfo(Plugin.Instance.LibVersion, IPA.Utilities.UnityGame.GameVersion.SemverValue, IPA.Utilities.UnityGame.GameVersion.ToString())).AsSingle();
            Container.BindInstance(Plugin.SettingsService).AsSingle();
            Container.BindInstance(Plugin.Instance.HttpInstance).AsSingle();
            Container.BindInstance(Plugin.Instance.ReplayState).AsSingle();

            Container.Bind<SaberRankUIMaterials>().AsSingle();
            Container.BindInterfacesAndSelfTo<SaberRankClock>().AsSingle().NonLazy();
            Container.Bind<ReplayFileCodec>().AsSingle();
            Container.Bind<ReplayService>().AsSingle().NonLazy();
            Container.Bind<CompeteGameplayState>().AsSingle();
            Container.Bind<CompeteGameplayControl>().AsSingle();
            Container.BindInterfacesAndSelfTo<LiveReplayStreamingService>().AsSingle().NonLazy();
        }
    }
}
