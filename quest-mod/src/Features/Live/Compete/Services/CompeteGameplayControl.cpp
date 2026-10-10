#include "Features/Live/Compete/Services/CompeteGameplayControl.hpp"

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::Services, CompeteGameplayControl);
DEFINE_TYPE(SnoreSaber::Features::Live::Compete::Services, CompeteGameplayControlBinder);

namespace SnoreSaber::Features::Live::Compete::Services
{
    void CompeteGameplayControl::ctor(CompeteGameplayState* gameplayState)
    {
        INVOKE_CTOR();
        _gameplayState = gameplayState;
    }

    void CompeteGameplayControl::Register(GlobalNamespace::PauseController* pauseController)
    {
        _pauseController = pauseController;
    }

    void CompeteGameplayControl::Unregister(GlobalNamespace::PauseController* pauseController)
    {
        if (_pauseController && _pauseController.ptr() == pauseController)
        {
            _pauseController = nullptr;
        }
    }

    bool CompeteGameplayControl::TryStopMap(const std::string& matchId)
    {
        if (!_gameplayState->IsLiveGameplayActive() || !_pauseController)
        {
            return false;
        }

        if (!matchId.empty() && matchId != _gameplayState->MatchId())
        {
            return false;
        }

        _gameplayState->MarkHostStopRequested();
        // same path SiraUtil ISongControl.Quit takes on pc
        _pauseController->HandlePauseMenuManagerDidPressMenuButton();
        return true;
    }

    void CompeteGameplayControlBinder::ctor(CompeteGameplayControl* gameplayControl, GlobalNamespace::PauseController* pauseController)
    {
        INVOKE_CTOR();
        _gameplayControl = gameplayControl;
        _pauseController = pauseController;
    }

    void CompeteGameplayControlBinder::Initialize()
    {
        if (!_gameplayControl || !_pauseController)
        {
            return;
        }

        _gameplayControl->Register(_pauseController.ptr());
    }

    void CompeteGameplayControlBinder::Dispose()
    {
        if (!_gameplayControl || !_pauseController)
        {
            return;
        }

        _gameplayControl->Unregister(_pauseController.ptr());
    }
}
