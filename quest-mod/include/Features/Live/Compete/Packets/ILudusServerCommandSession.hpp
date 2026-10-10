#pragma once

#include "Features/Live/Cancellation.hpp"
#include "Features/Live/Compete/Domain/CompeteOrganizerPrompt.hpp"
#include "Features/Live/Compete/Domain/CompeteRoom.hpp"
#include "Features/Live/Compete/Services/CompeteDirectoryService.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayControl.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayLauncher.hpp"
#include "Features/Live/Compete/Services/CompeteSongService.hpp"
#include "Features/Live/Protocol/Generated/Common.hpp"
#include "Features/Live/Protocol/Generated/RoomState.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace SnoreSaber::Features::Live::Compete::Packets
{
    // server command handlers talk to the session service through this; the delegate-bag
    // CompeteLudusCommandSession is the only implementation (PC parity)
    struct ILudusServerCommandSession
    {
        virtual ~ILudusServerCommandSession() = default;

        virtual std::string LocalPlayerId() const = 0;
        virtual std::shared_ptr<Domain::CompeteRoom> TournamentRoom() const = 0;
        virtual void SetTournamentRoom(const std::shared_ptr<Domain::CompeteRoom>& room) = 0;
        virtual CancellationToken ConnectionCancellationToken() const = 0;
        virtual Services::CompeteSongService* SongService() const = 0;
        virtual Services::CompeteDirectoryService* DirectoryService() const = 0;
        virtual Services::CompeteGameplayLauncher* GameplayLauncher() const = 0;
        virtual Services::CompeteGameplayControl* GameplayControl() const = 0;
        virtual void NotifyPlayerFollowRequested(int viewerCount) = 0;
        // nullopt mirrors the PC null viewers list (no matching room in the snapshot)
        virtual void NotifyViewersUpdated(const std::optional<std::vector<::SnoreSaber::Live::V1::LiveRoomViewerState>>& viewers) = 0;
        virtual void NotifyRoomUpdated(const std::shared_ptr<Domain::CompeteRoom>& room) = 0;
        virtual void NotifyPromptReceived(const Domain::CompeteOrganizerPrompt& prompt) = 0;
        virtual void NotifyStatusChanged(const std::string& status) = 0;
        virtual void CloseTournamentRoom() = 0;
        virtual void SendDownloadState(::SnoreSaber::Live::V1::LudusDownloadState state, const std::string& errorMessage = "") = 0;
        virtual void SendPresence(::SnoreSaber::Live::V1::LudusPlayState playState, ::SnoreSaber::Live::V1::LudusDownloadState downloadState, const std::string& currentMapHash) = 0;
        virtual CancellationToken BeginMapStartCountdown(const std::string& matchId, int delayMs, const CancellationToken& cancellationToken) = 0;
        virtual bool TryCancelPendingMapStart(const std::string& matchId) = 0;
        virtual void CompletePendingMapStart(const std::string& matchId, const CancellationToken& countdownToken) = 0;
    };
}
