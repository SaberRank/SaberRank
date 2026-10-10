#include "Features/Live/Compete/Services/CompetePauseGuard.hpp"

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::Services, CompetePauseGuard);

namespace SnoreSaber::Features::Live::Compete::Services
{
    void CompetePauseGuard::ctor(GlobalNamespace::PauseController* pauseController, CompeteGameplayState* gameplayState)
    {
        INVOKE_CTOR();
        _pauseController = pauseController;
        _gameplayState = gameplayState;
    }

    void CompetePauseGuard::Initialize()
    {
        _canPauseDelegate = { &CompetePauseGuard::CanPause, this };
        _pauseController->___canPauseEvent += _canPauseDelegate;
    }

    void CompetePauseGuard::Dispose()
    {
        if (_pauseController)
        {
            _pauseController->___canPauseEvent -= _canPauseDelegate;
        }
    }

    void CompetePauseGuard::CanPause(System::Action_1<bool>* canPause)
    {
        if (_gameplayState->IsLiveGameplayActive() && canPause)
        {
            canPause->Invoke(false);
        }
    }
}
