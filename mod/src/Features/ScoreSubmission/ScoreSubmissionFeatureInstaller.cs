using SnoreSaber.Features.ScoreSubmission.Services;
using Zenject;

namespace SnoreSaber.Features.ScoreSubmission {
    internal class ScoreSubmissionFeatureInstaller : Installer {
        public override void InstallBindings() {
            Container.Bind<ScoreUploadPayloadBuilder>().AsSingle();
            Container.Bind<ScoreSubmissionWorkflow>().AsSingle();
            Container.Bind<ScoreSubmissionService>().AsSingle();
            Container.BindInterfacesAndSelfTo<ScoreSubmissionController>().AsSingle().NonLazy();
        }
    }
}
