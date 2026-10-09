using BeatSaberMarkupLanguage;
using HMUI;
using SnoreSaber.Core.Configuration;
using SnoreSaber.Features.Players.Services;
using SnoreSaber.Features.Leaderboards.Services;
using SnoreSaber.Features.MainMenu.Settings.ViewControllers;
using Zenject;

namespace SnoreSaber.Features.MainMenu.Settings {
    internal class SnoreSaberSettingsFlowCoordinator : FlowCoordinator, IInitializable, ISnoreSaberFlowCoordinator {

        private FlowCoordinator _lastFlowCoordinator;
        private MainSettingsViewController _mainSettingsHandlerViewController;
        private LeaderboardScreenSession _leaderboardSession;
        private LocalPlayerPanelSession _localPlayerPanelSession;
        private SettingsService _settings;
        FlowCoordinator ISnoreSaberFlowCoordinator.FlowCoordinator => this;

        protected override void DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {

            if (firstActivation) {
                SetTitle("SnoreSaber Settings");
                showBackButton = true;
                ProvideInitialViewControllers(_mainSettingsHandlerViewController);
            }
        }

        [Inject]
        internal void Construct(MainSettingsViewController mainSettingsViewController, LeaderboardScreenSession leaderboardSession, LocalPlayerPanelSession localPlayerPanelSession, SettingsService settings) {

            _mainSettingsHandlerViewController = mainSettingsViewController;
            _leaderboardSession = leaderboardSession;
            _localPlayerPanelSession = localPlayerPanelSession;
            _settings = settings;
            Plugin.Log.Debug("SnoreSaberSettingsFlowCoordinator Setup");
        }

        protected override void BackButtonWasPressed(ViewController topViewController) {

            SetLeftScreenViewController(null, ViewController.AnimationType.None);
            SetRightScreenViewController(null, ViewController.AnimationType.None);
            _settings.Save();
            _lastFlowCoordinator.DismissFlowCoordinator(this);
            _leaderboardSession.RefreshFromFirstPage();
            _localPlayerPanelSession.ApplyCurrentSettings();
        }

        public void SetPresentingFlowCoordinator(FlowCoordinator flowCoordinator) => _lastFlowCoordinator = flowCoordinator;

        public void Initialize() { }
    }
}
