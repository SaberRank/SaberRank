using BeatSaberMarkupLanguage;
using HMUI;
using SnoreSaber.Features.MainMenu.MainFlow.FAQ;
using SnoreSaber.Features.MainMenu.MainFlow.GlobalLeaderboard;
using SnoreSaber.Features.MainMenu.MainFlow.Teams.UI;
using Zenject;

namespace SnoreSaber.Features.MainMenu.MainFlow {
    internal class SnoreSaberFlowCoordinator : FlowCoordinator, IInitializable, ISnoreSaberFlowCoordinator {

        private FlowCoordinator _lastFlowCoordinator;
        private FAQViewController _faqViewController;
        private TeamViewController _teamViewController;
        private GlobalViewController _globalViewController;
        FlowCoordinator ISnoreSaberFlowCoordinator.FlowCoordinator => this;

        protected override void DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {

            if (firstActivation) {
                SetTitle("SnoreSaber");
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
            Plugin.Log.Debug("SnoreSaberFlowCoordinator Setup");
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
