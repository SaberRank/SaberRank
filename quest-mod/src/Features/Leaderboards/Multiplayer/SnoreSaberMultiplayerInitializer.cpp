#include "Features/Leaderboards/Multiplayer/SnoreSaberMultiplayerInitializer.hpp"

#include "Features/Leaderboards/UI/SnoreSaberLeaderboardView.hpp"
#include <custom-types/shared/delegate.hpp>
#include <functional>

DEFINE_TYPE(SnoreSaber::UI::Multiplayer, SnoreSaberMultiplayerInitializer);

namespace SnoreSaber::UI::Multiplayer
{
    void SnoreSaberMultiplayerInitializer::ctor(GameServerLobbyFlowCoordinator* gameServerLobbyFlowCoordinator,
                                                SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService)
    {
        _gameServerLobbyFlowCoordinator = gameServerLobbyFlowCoordinator;
        _gameSessionService = gameSessionService;
    }

    void SnoreSaberMultiplayerInitializer::Initialize()
    {
        didSetupDelegate = { &SnoreSaberMultiplayerInitializer::GameServerLobbyFlowCoordinator_didSetupEvent, this };
        didFinishDelegate = { &SnoreSaberMultiplayerInitializer::GameServerLobbyFlowCoordinator_didFinishEvent, this };

        _gameServerLobbyFlowCoordinator->___didSetupEvent += didSetupDelegate;
        _gameServerLobbyFlowCoordinator->___didFinishEvent += didFinishDelegate;
    }

    void SnoreSaberMultiplayerInitializer::Dispose()
    {
        if(_gameServerLobbyFlowCoordinator) {
            _gameServerLobbyFlowCoordinator->___didSetupEvent -= didSetupDelegate;
            _gameServerLobbyFlowCoordinator->___didFinishEvent -= didFinishDelegate;
        }
    }

    void SnoreSaberMultiplayerInitializer::GameServerLobbyFlowCoordinator_didSetupEvent()
    {
        _gameSessionService->EnsureAuthenticated(false, [](SnoreSaber::Features::Players::Services::GameSessionService::LoginStatus loginStatus) {});
        Other::SnoreSaberLeaderboardView::AllowReplayWatching(false);
    }

    void SnoreSaberMultiplayerInitializer::GameServerLobbyFlowCoordinator_didFinishEvent()
    {
        Other::SnoreSaberLeaderboardView::AllowReplayWatching(true);
    }
} // namespace SnoreSaber::UI::Multiplayer
