#pragma once

#include "Core/Api/SnoreSaberApiClient.hpp"
#include "Core/BeatSaver/BeatSaverService.hpp"
#include "Features/Live/Cancellation.hpp"
#include "Features/Live/Compete/Services/CompeteSongService.hpp"
#include "Features/Live/Ludus/Domain/LiveChatEntry.hpp"
#include "Features/Live/Ludus/Services/LiveChatSongNavigator.hpp"
#include "Features/Live/Ludus/Services/LudusSessionService.hpp"
#include "Utils/Event.hpp"

#include <custom-types/shared/macros.hpp>

#include <map>
#include <optional>
#include <set>
#include <string>

namespace SnoreSaber::Features::Live::Ludus::Services
{
    enum class LiveChatLinkKind
    {
        ExternalUrl,
        BeatSaverId,
        BeatSaverHash,
        SnoreSaberMapId,
        SnoreSaberLeaderboardId
    };

    struct LiveChatLinkTarget
    {
        LiveChatLinkKind kind = LiveChatLinkKind::ExternalUrl;
        std::string value;
        std::string secondaryValue;
        std::string url;
    };
}

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Ludus::Services, LiveChatLinkService, System::Object) {
    DECLARE_INSTANCE_FIELD_PRIVATE(Core::BeatSaver::BeatSaverService*, _beatSaver);
    DECLARE_INSTANCE_FIELD_PRIVATE(Compete::Services::CompeteSongService*, _songService);
    DECLARE_INSTANCE_FIELD_PRIVATE(LiveChatSongNavigator*, _songNavigator);
    DECLARE_INSTANCE_FIELD_PRIVATE(LudusSessionService*, _ludusSession);
    DECLARE_CTOR(ctor,
                 Core::BeatSaver::BeatSaverService* beatSaver,
                 Compete::Services::CompeteSongService* songService,
                 LiveChatSongNavigator* songNavigator,
                 LudusSessionService* ludusSession);

  public:
    // fire on the main thread
    Utils::Event<const std::string&> StatusChanged;
    Utils::Event<> ResolvedTextChanged;

    std::optional<LiveChatLinkTarget> FirstLink(const std::string& text);
    std::string DisplaySenderName(const Domain::LiveChatEntry& entry);
    std::string DisplayText(const Domain::LiveChatEntry& entry);

    // blocking; call off the main thread
    void Open(const std::optional<LiveChatLinkTarget>& target, const CancellationToken& cancellationToken);

  private:
    Core::Api::SnoreSaberApiClient _apiClient;
    // resolved-name caches; main-thread only (ui reads + Async::Main writes)
    std::map<std::string, std::string> _playerNames;
    std::map<std::string, std::string> _mapNames;
    std::set<std::string> _pendingPlayerNames;
    std::set<std::string> _pendingMapNames;

    std::string LogPlayerName(const Domain::LiveChatEntry& entry, const std::string& fallbackName, const std::string& fallbackPlayerId);
    std::optional<V1::LiveSongCommand> SongFromTarget(const LiveChatLinkTarget& target, const CancellationToken& cancellationToken);
    V1::LiveSongCommand SongFromBeatSaverId(const std::string& id, const CancellationToken& cancellationToken);
    std::optional<V1::LiveSongCommand> SongFromSnoreSaberMap(const LiveChatLinkTarget& target, const CancellationToken& cancellationToken);
    std::string ResolvedPlayerName(const std::string& playerId);
    std::string ResolvedMapName(const std::string& hash);
    void QueuePlayerResolution(const std::string& playerId);
    void QueueMapResolution(const std::string& hash);
    void ResolvePlayerName(const std::string& playerId);
    void ResolveMapName(const std::string& hash);
    void InvokeStatus(const std::string& status);
};
