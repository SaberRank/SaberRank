using BeatSaberMarkupLanguage;
using HMUI;
using SaberRank.Features.MainMenu.MainFlow.FAQ;
using SaberRank.Features.MainMenu.MainFlow.GlobalLeaderboard;
using SaberRank.Features.MainMenu.MainFlow.Teams.UI;
using Zenject;

namespace SaberRank.Features.MainMenu.MainFlow {
    internal class SaberRankFlowCoordinator : FlowCoordinator, IInitializable, ISaberRankFlowCoordinator {

        private FlowCoordinator _lastFlowCoordinator;
        private FAQViewController _faqViewController;
        private TeamViewController _teamViewController;
        private GlobalViewController _globalViewController;
        FlowCoordinator ISaberRankFlowCoordinator.FlowCoordinator => this;

        protected override void DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {

            if (firstActivation) {
                SetTitle("SaberRank");
                showBackButton = true;
                ProvideInitialViewControllers(_globalViewController, _teamViewController, _faqViewController);
            }
        }

        [Inject]
        internal void Construct(
            FAQViewController faqViewController,
            TeamViewController teamViewController,
            GlobalViewController globalViewController) {

            _faqViewController = faqViewController;
            _teamViewController = teamViewController;
            _globalViewController = globalViewController;
            Plugin.Log.Debug("SaberRankFlowCoordinator Setup");
        }

        protected override void BackButtonWasPressed(ViewController topViewController) {

            SetLeftScreenViewController(null, ViewController.AnimationType.None);
            SetRightScreenViewController(null, ViewController.AnimationType.None);
            _lastFlowCoordinator.DismissFlowCoordinator(this);
        }

        public void SetPresentingFlowCoordinator(FlowCoordinator flowCoordinator) => _lastFlowCoordinator = flowCoordinator;

        public void Initialize() { }
    }
}
