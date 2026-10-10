#include "Features/MainMenu/SnoreSaberMenuNavigator.hpp"

#include <HMUI/ViewController.hpp>

DEFINE_TYPE(SnoreSaber::Features::MainMenu, SnoreSaberMenuNavigator);

namespace SnoreSaber::Features::MainMenu
{
    void SnoreSaberMenuNavigator::ctor(
        GlobalNamespace::MainFlowCoordinator* mainFlowCoordinator,
        SnoreSaber::UI::FlowCoordinators::SnoreSaberFlowCoordinator* scoreSaberFlowCoordinator,
        SnoreSaber::UI::FlowCoordinators::SnoreSaberSettingsFlowCoordinator* settingsFlowCoordinator,
        ::SnoreSaber::Features::Live::Compete::UI::FlowCoordinators::CompeteFlowCoordinator* competeFlowCoordinator)
    {
        INVOKE_CTOR();
        _mainFlowCoordinator = mainFlowCoordinator;
        _scoreSaberFlowCoordinator = scoreSaberFlowCoordinator;
        _settingsFlowCoordinator = settingsFlowCoordinator;
        _competeFlowCoordinator = competeFlowCoordinator;
        if (_competeFlowCoordinator)
        {
            // navigator and flow coordinator are menu singletons that die together; PC never unsubscribes either
            _competeFlowCoordinator->DidFinishEvent.Add([this] { TournamentFlowDidFinish(); });
        }
    }

    void SnoreSaberMenuNavigator::ShowMain()
    {
        auto activeFlowCoordinator = ActiveFlowCoordinator();
        if (!activeFlowCoordinator || !_scoreSaberFlowCoordinator)
        {
            return;
        }

        _scoreSaberFlowCoordinator->SetPresentingFlowCoordinator(activeFlowCoordinator);
        Present(activeFlowCoordinator, _scoreSaberFlowCoordinator);
    }

    void SnoreSaberMenuNavigator::ShowSettings()
    {
        auto activeFlowCoordinator = ActiveFlowCoordinator();
        if (!activeFlowCoordinator || !_settingsFlowCoordinator)
        {
            return;
        }

        _settingsFlowCoordinator->SetPresentingFlowCoordinator(activeFlowCoordinator);
        Present(activeFlowCoordinator, _settingsFlowCoordinator);
    }

    void SnoreSaberMenuNavigator::ShowCompete()
    {
        PresentTournamentFlow(_competeFlowCoordinator);
    }

    void SnoreSaberMenuNavigator::PresentTournamentFlow(HMUI::FlowCoordinator* flowCoordinator)
    {
        if (_activeTournamentFlowCoordinator || !flowCoordinator)
        {
            return;
        }

        auto presentingFlowCoordinator = ActiveFlowCoordinator();
        if (!presentingFlowCoordinator || presentingFlowCoordinator == flowCoordinator)
        {
            return;
        }

        _tournamentPresentingFlowCoordinator = presentingFlowCoordinator;
        _activeTournamentFlowCoordinator = flowCoordinator;
        Present(presentingFlowCoordinator, flowCoordinator);
    }

    void SnoreSaberMenuNavigator::TournamentFlowDidFinish()
    {
        if (!_activeTournamentFlowCoordinator || !_tournamentPresentingFlowCoordinator)
        {
            return;
        }

        auto flowCoordinator = _activeTournamentFlowCoordinator;
        auto presentingFlowCoordinator = _tournamentPresentingFlowCoordinator;
        _activeTournamentFlowCoordinator = nullptr;
        _tournamentPresentingFlowCoordinator = nullptr;
        presentingFlowCoordinator->DismissFlowCoordinator(flowCoordinator, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);
    }

    HMUI::FlowCoordinator* SnoreSaberMenuNavigator::ActiveFlowCoordinator()
    {
        if (!_mainFlowCoordinator)
        {
            return nullptr;
        }
        return _mainFlowCoordinator->YoungestChildFlowCoordinatorOrSelf();
    }

    void SnoreSaberMenuNavigator::Present(HMUI::FlowCoordinator* activeFlowCoordinator, HMUI::FlowCoordinator* flowCoordinator)
    {
        if (!activeFlowCoordinator || !flowCoordinator || activeFlowCoordinator == flowCoordinator)
        {
            return;
        }

        activeFlowCoordinator->PresentFlowCoordinator(flowCoordinator, nullptr, HMUI::ViewController::AnimationDirection::Horizontal, false, false);
    }
}
