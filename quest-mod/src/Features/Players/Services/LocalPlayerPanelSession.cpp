#include "Features/Players/Services/LocalPlayerPanelSession.hpp"

#include "Core/Presentation/PlayerPresentation.hpp"
#include "Utils/AsyncUtils.hpp"
#include "Utils/SafePtr.hpp"
#include "logging.hpp"

#include <utility>

DEFINE_TYPE(SnoreSaber::Features::Players::Services, LocalPlayerPanelSession);

namespace SnoreSaber::Features::Players::Services
{
    void LocalPlayerPanelSession::ctor(GameSessionService* gameSessionService, PlayerProfileService* playerProfileService)
    {
        INVOKE_CTOR();
        _gameSessionService = gameSessionService;
        _playerProfileService = playerProfileService;
        _currentState = SnoreSaber::Data::LocalPlayerPanelState::Initial();
    }

    SnoreSaber::Data::LocalPlayerPanelState LocalPlayerPanelSession::Load()
    {
        std::optional<SnoreSaber::Data::GameSession> session = _gameSessionService->GetGameSession();
        if (!session.has_value())
        {
            return SnoreSaber::Data::LocalPlayerPanelState::Unavailable();
        }

        std::optional<SnoreSaber::Data::Player> player = _playerProfileService->GetPlayerInfo(session->playerId, false);
        if (!player.has_value())
        {
            return SnoreSaber::Data::LocalPlayerPanelState::Message("Welcome to SnoreSaber! Set a snore to create a profile");
        }

        return SnoreSaber::Data::LocalPlayerPanelState::WithPlayer(std::move(player.value()),
                                                                    SnoreSaber::Core::Presentation::PlayerPresentation::UsesWilliumsPanel(session->playerId),
                                                                    SnoreSaber::Core::Presentation::PlayerPresentation::UsesDenyahPanel(session->playerId));
    }

    void LocalPlayerPanelSession::Refresh(std::function<void(SnoreSaber::Data::LocalPlayerPanelState)> finished)
    {
        Publish(SnoreSaber::Data::LocalPlayerPanelState::Loading(_currentState));

        SafePtr<LocalPlayerPanelSession> self(this);
        SnoreSaber::Utils::Async::RunThenMain(
            [self] {
                try
                {
                    return self->Load();
                }
                catch (const std::exception& exception)
                {
                    ERROR("Failed to update local player ranking: {:s}", exception.what());
                    return SnoreSaber::Data::LocalPlayerPanelState::PromptError(SnoreSaber::Data::LocalPlayerPanelState::Initial(), "Failed to update local player ranking", 1.5f);
                }
            },
            [self, finished = std::move(finished)](SnoreSaber::Data::LocalPlayerPanelState state) {
                self->Publish(state);
                if (finished)
                {
                    finished(state);
                }
            });
    }

    void LocalPlayerPanelSession::ApplyCurrentSettings()
    {
        Publish(_currentState);
    }

    SnoreSaber::Data::LocalPlayerPanelState LocalPlayerPanelSession::CurrentState() const
    {
        return _currentState;
    }

    void LocalPlayerPanelSession::SetStateChangedCallback(StateChangedCallback callback)
    {
        _stateChanged = std::move(callback);
    }

    void LocalPlayerPanelSession::Publish(SnoreSaber::Data::LocalPlayerPanelState state)
    {
        _currentState = state;
        if (_stateChanged)
        {
            _stateChanged(_currentState);
        }
    }
}
