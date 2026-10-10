#pragma once

#include "Features/Live/Compete/Packets/ILudusServerCommandSession.hpp"

#include <functional>
#include <utility>

namespace SnoreSaber::Features::Live::Compete::Packets
{
    // forwards every session call to delegates the session service wires to itself
    // (PC passes 19 positional lambdas; designated initializers are the safer spelling here)
    class CompeteLudusCommandSession final : public ILudusServerCommandSession
    {
      public:
        struct Delegates
        {
            std::function<std::string()> localPlayerId;
            std::function<std::shared_ptr<Domain::CompeteRoom>()> getTournamentRoom;
            std::function<void(const std::shared_ptr<Domain::CompeteRoom>&)> setTournamentRoom;
            std::function<CancellationToken()> connectionCancellationToken;
            std::function<void(int)> playerFollowRequested;
            std::function<void(const std::optional<std::vector<::SnoreSaber::Live::V1::LiveRoomViewerState>>&)> viewersUpdated;
            std::function<void(const std::shared_ptr<Domain::CompeteRoom>&)> roomUpdated;
            std::function<void(const Domain::CompeteOrganizerPrompt&)> promptReceived;
            std::function<void(const std::string&)> statusChanged;
            std::function<void()> closeTournamentRoom;
            std::function<void(::SnoreSaber::Live::V1::LudusDownloadState, const std::string&)> sendDownloadState;
            std::function<void(::SnoreSaber::Live::V1::LudusPlayState, ::SnoreSaber::Live::V1::LudusDownloadState, const std::string&)> sendPresence;
            std::function<CancellationToken(const std::string&, int, const CancellationToken&)> beginMapStartCountdown;
            std::function<bool(const std::string&)> tryCancelPendingMapStart;
            std::function<void(const std::string&, const CancellationToken&)> completePendingMapStart;
        };

        // services must stay rooted in il2cpp fields of the owning session service
        CompeteLudusCommandSession(
            Delegates delegates,
            Services::CompeteSongService* songService,
            Services::CompeteDirectoryService* directoryService,
            Services::CompeteGameplayLauncher* gameplayLauncher,
            Services::CompeteGameplayControl* gameplayControl)
            : _delegates(std::move(delegates)),
              _songService(songService),
              _directoryService(directoryService),
              _gameplayLauncher(gameplayLauncher),
              _gameplayControl(gameplayControl) {}

        std::string LocalPlayerId() const override { return _delegates.localPlayerId(); }
        std::shared_ptr<Domain::CompeteRoom> TournamentRoom() const override { return _delegates.getTournamentRoom(); }
        void SetTournamentRoom(const std::shared_ptr<Domain::CompeteRoom>& room) override { _delegates.setTournamentRoom(room); }
        CancellationToken ConnectionCancellationToken() const override { return _delegates.connectionCancellationToken(); }
        Services::CompeteSongService* SongService() const override { return _songService; }
        Services::CompeteDirectoryService* DirectoryService() const override { return _directoryService; }
        Services::CompeteGameplayLauncher* GameplayLauncher() const override { return _gameplayLauncher; }
        Services::CompeteGameplayControl* GameplayControl() const override { return _gameplayControl; }
        void NotifyPlayerFollowRequested(int viewerCount) override { _delegates.playerFollowRequested(viewerCount); }
        void NotifyViewersUpdated(const std::optional<std::vector<::SnoreSaber::Live::V1::LiveRoomViewerState>>& viewers) override { _delegates.viewersUpdated(viewers); }
        void NotifyRoomUpdated(const std::shared_ptr<Domain::CompeteRoom>& room) override { _delegates.roomUpdated(room); }
        void NotifyPromptReceived(const Domain::CompeteOrganizerPrompt& prompt) override { _delegates.promptReceived(prompt); }
        void NotifyStatusChanged(const std::string& status) override { _delegates.statusChanged(status); }
        void CloseTournamentRoom() override { _delegates.closeTournamentRoom(); }
        void SendDownloadState(::SnoreSaber::Live::V1::LudusDownloadState state, const std::string& errorMessage) override { _delegates.sendDownloadState(state, errorMessage); }
        void SendPresence(::SnoreSaber::Live::V1::LudusPlayState playState, ::SnoreSaber::Live::V1::LudusDownloadState downloadState, const std::string& currentMapHash) override { _delegates.sendPresence(playState, downloadState, currentMapHash); }
        CancellationToken BeginMapStartCountdown(const std::string& matchId, int delayMs, const CancellationToken& cancellationToken) override { return _delegates.beginMapStartCountdown(matchId, delayMs, cancellationToken); }
        bool TryCancelPendingMapStart(const std::string& matchId) override { return _delegates.tryCancelPendingMapStart(matchId); }
        void CompletePendingMapStart(const std::string& matchId, const CancellationToken& countdownToken) override { _delegates.completePendingMapStart(matchId, countdownToken); }

      private:
        Delegates _delegates;
        Services::CompeteSongService* _songService;
        Services::CompeteDirectoryService* _directoryService;
        Services::CompeteGameplayLauncher* _gameplayLauncher;
        Services::CompeteGameplayControl* _gameplayControl;
    };
}
