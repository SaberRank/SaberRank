using HMUI;
using SaberRank.Features.Live.Compete.UI.FlowCoordinators;
using SaberRank.Features.MainMenu.MainFlow;
using SaberRank.Features.MainMenu.Settings;

namespace SaberRank.Features.MainMenu {
    internal class SaberRankMenuNavigator {
        private readonly MainFlowCoordinator _mainFlowCoordinator;
        private readonly SaberRankFlowCoordinator _scoreSaberFlowCoordinator;
        private readonly SaberRankSettingsFlowCoordinator _settingsFlowCoordinator;
        private readonly CompeteFlowCoordinator _competeFlowCoordinator;
        private FlowCoordinator _activeTournamentFlowCoordinator;
        private FlowCoordinator _tournamentPresentingFlowCoordinator;

        public SaberRankMenuNavigator(
            MainFlowCoordinator mainFlowCoordinator,
            SaberRankFlowCoordinator scoreSaberFlowCoordinator,
            SaberRankSettingsFlowCoordinator settingsFlowCoordinator,
            CompeteFlowCoordinator competeFlowCoordinator) {

            _mainFlowCoordinator = mainFlowCoordinator;
            _scoreSaberFlowCoordinator = scoreSaberFlowCoordinator;
            _settingsFlowCoordinator = settingsFlowCoordinator;
            _competeFlowCoordinator = competeFlowCoordinator;
            _competeFlowCoordinator.DidFinishEvent += TournamentFlowDidFinish;
        }

        internal void ShowMain() {
            Present(_scoreSaberFlowCoordinator);
        }

        internal void ShowSettings() {
            Present(_settingsFlowCoordinator);
        }

        internal void ShowCompete() {
            PresentTournamentFlow(_competeFlowCoordinator);
        }

        private void Present(ISaberRankFlowCoordinator flowCoordinator) {
            FlowCoordinator activeFlow = DeepestChildFlowCoordinator(_mainFlowCoordinator);
            activeFlow.PresentFlowCoordinator(flowCoordinator.FlowCoordinator);
            flowCoordinator.SetPresentingFlowCoordinator(activeFlow);
        }

        private void PresentTournamentFlow(FlowCoordinator flowCoordinator) {
            if (_activeTournamentFlowCoordinator != null) {
                return;
            }

            _tournamentPresentingFlowCoordinator = DeepestChildFlowCoordinator(_mainFlowCoordinator);
            _activeTournamentFlowCoordinator = flowCoordinator;
            _tournamentPresentingFlowCoordinator.PresentFlowCoordinator(flowCoordinator);
        }

        private void TournamentFlowDidFinish() {
            if (_activeTournamentFlowCoordinator == null || _tournamentPresentingFlowCoordinator == null) {
                return;
            }

            FlowCoordinator flowCoordinator = _activeTournamentFlowCoordinator;
            FlowCoordinator presentingFlowCoordinator = _tournamentPresentingFlowCoordinator;
            _activeTournamentFlowCoordinator = null;
            _tournamentPresentingFlowCoordinator = null;
            presentingFlowCoordinator.DismissFlowCoordinator(flowCoordinator);
        }

        private FlowCoordinator DeepestChildFlowCoordinator(FlowCoordinator root) {
            FlowCoordinator flow = root.childFlowCoordinator;
            if (flow == null) {
                return root;
            }

            if (flow.childFlowCoordinator == null || flow.childFlowCoordinator == flow) {
                return flow;
            }

            return DeepestChildFlowCoordinator(flow);
        }
    }

    internal interface ISaberRankFlowCoordinator {
        FlowCoordinator FlowCoordinator { get; }
        void SetPresentingFlowCoordinator(FlowCoordinator flowCoordinator);
    }
}
