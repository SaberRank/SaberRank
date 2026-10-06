using BeatSaberMarkupLanguage;
using HMUI;
using SaberRank.Core.Configuration;
using SaberRank.Features.Players.Services;
using SaberRank.Features.Leaderboards.Services;
using SaberRank.Features.MainMenu.Settings.ViewControllers;
using Zenject;

namespace SaberRank.Features.MainMenu.Settings {
    internal class SaberRankSettingsFlowCoordinator : FlowCoordinator, IInitializable, ISaberRankFlowCoordinator {

        private FlowCoordinator _lastFlowCoordinator;
        private MainSettingsViewController _mainSettingsHandlerViewController;
        private LeaderboardScreenSession _leaderboardSession;
        private LocalPlayerPanelSession _localPlayerPanelSession;
        private SettingsService _settings;
        FlowCoordinator ISaberRankFlowCoordinator.FlowCoordinator => this;

        protected override void DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {

            if (firstActivation) {
                SetTitle("SaberRank Settings");
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
            Plugin.Log.Debug("SaberRankSettingsFlowCoordinator Setup");
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
