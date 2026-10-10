#include "Features/Live/Ludus/Services/LiveChatLinkService.hpp"

#include "Utils/AsyncUtils.hpp"
#include "logging.hpp"

#include <UnityEngine/Application.hpp>
#include <paper2_scotland2/shared/string_convert.hpp>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <initializer_list>
#include <regex>
#include <vector>

DEFINE_TYPE(SnoreSaber::Features::Live::Ludus::Services, LiveChatLinkService);

namespace SnoreSaber::Features::Live::Ludus::Services
{
    namespace
    {
        // pc compiled regexes; [\s\S] stands in for c# singleline dots
        const std::regex LinkPattern(R"((https?://[^\s<>"]+)|(?:\b(?:bsr|bsid)[:\s#-]*([0-9a-f]{1,8})\b))", std::regex::icase);
        const std::regex HashPattern(R"(\b[0-9a-f]{40}\b)", std::regex::icase);
        const std::regex RawPlayerRoomLogPattern(R"(^(\d{15,20}) (joined|left) the room$)");
        const std::regex NamedPlayerRoomLogPattern(R"(^([\s\S]+) (joined|left) the room$)");
        const std::regex RawLoadedMapLogPattern(R"(^Loaded map\s+([A-Fa-f0-9]{40})([\s\S]*)$)", std::regex::icase);
        const std::regex DisplayMarkupTagPattern(R"(<[^>\r\n]{1,128}>)");
        // minimal absolute-uri parse standing in for c# Uri.TryCreate: scheme, host, path
        const std::regex UrlPattern(R"(^(https?)://([^/?#\s]+)([^?#]*))", std::regex::icase);

        bool IsBlank(const std::string& value)
        {
            return std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
        }

        std::string FirstNonEmpty(std::initializer_list<std::string> values)
        {
            for (const auto& value : values)
            {
                if (!IsBlank(value))
                {
                    return value;
                }
            }

            return "";
        }

        std::string ToLower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);
            return value;
        }

        std::string ToUpper(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), ::toupper);
            return value;
        }

        bool EndsWith(const std::string& value, const std::string& suffix)
        {
            return value.size() >= suffix.size() && value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        bool EqualsIgnoreCase(const std::string& a, const std::string& b)
        {
            return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) { return std::tolower(x) == std::tolower(y); });
        }

        bool TryParseInt(const std::string& value, int& result)
        {
            auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), result);
            return ec == std::errc() && ptr == value.data() + value.size();
        }

        std::string TrimToken(std::string token)
        {
            while (!token.empty() && std::strchr(".,;:)]}", token.back()) && token.back() != '\0')
            {
                token.pop_back();
            }

            return token;
        }

        std::string StripDisplayMarkup(const std::string& value)
        {
            return value.empty() ? std::string() : std::regex_replace(value, DisplayMarkupTagPattern, "");
        }

        // c# split-on-whitespace + join(" ") collapse
        std::string CollapseWhitespace(const std::string& value)
        {
            std::string result;
            bool pendingSpace = false;
            for (char c : value)
            {
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
                {
                    pendingSpace = !result.empty();
                    continue;
                }
                if (pendingSpace)
                {
                    result += ' ';
                    pendingSpace = false;
                }
                result += c;
            }

            return result;
        }

        std::string CleanDisplayName(const std::string& value)
        {
            return CollapseWhitespace(StripDisplayMarkup(value));
        }

        std::string NormalizeLogSuffix(const std::string& suffix)
        {
            std::string collapsed = CollapseWhitespace(suffix);
            return collapsed.empty() ? "" : " " + collapsed;
        }

        std::string FormatMapName(const Core::Api::Generated::MapDetailsResponse& map)
        {
            std::string name = FirstNonEmpty({CollapseWhitespace(map.SongName), "map"});
            return IsBlank(map.SongAuthorName) ? name : fmt::format("{:s} by {:s}", name, map.SongAuthorName);
        }

        std::string MapLinkFailureMessage(const std::exception& ex)
        {
            if (dynamic_cast<const std::filesystem::filesystem_error*>(&ex))
            {
                return "Could not install the downloaded map.";
            }

            std::string message = ex.what() ? ex.what() : "";
            return IsBlank(message) ? "Unknown error." : message;
        }

        bool IsNumericId(const std::string& value)
        {
            if (value.empty())
            {
                return false;
            }

            return std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); });
        }

        std::vector<std::string> SplitPath(const std::string& path)
        {
            std::string trimmed = path;
            size_t begin = trimmed.find_first_not_of('/');
            size_t end = trimmed.find_last_not_of('/');
            if (begin == std::string::npos)
            {
                return {""};
            }
            trimmed = trimmed.substr(begin, end - begin + 1);

            // c# Split('/') keeps empty entries
            std::vector<std::string> parts;
            size_t start = 0;
            while (true)
            {
                size_t slash = trimmed.find('/', start);
                if (slash == std::string::npos)
                {
                    parts.push_back(trimmed.substr(start));
                    break;
                }
                parts.push_back(trimmed.substr(start, slash - start));
                start = slash + 1;
            }

            return parts;
        }

        bool TryResolveSnoreSaberUri(const std::string& url, const std::string& path, LiveChatLinkTarget& target)
        {
            std::vector<std::string> parts = SplitPath(path);
            for (size_t i = 0; i < parts.size(); i++)
            {
                if (!EqualsIgnoreCase(parts[i], "map") || i + 1 >= parts.size() || !IsNumericId(parts[i + 1]))
                {
                    continue;
                }

                for (size_t j = i + 2; j + 1 < parts.size(); j++)
                {
                    if (EqualsIgnoreCase(parts[j], "difficulty") && IsNumericId(parts[j + 1]))
                    {
                        target = {LiveChatLinkKind::SnoreSaberLeaderboardId, parts[i + 1], parts[j + 1], url};
                        return true;
                    }
                }

                target = {LiveChatLinkKind::SnoreSaberMapId, parts[i + 1], "", url};
                return true;
            }

            return false;
        }

        bool TryResolveBeatSaverUri(const std::string& url, const std::string& path, LiveChatLinkTarget& target)
        {
            std::vector<std::string> parts = SplitPath(path);
            for (size_t i = 0; i < parts.size(); i++)
            {
                std::string part = ToLower(parts[i]);
                if ((part == "maps" || part == "map") && i + 1 < parts.size())
                {
                    if (EqualsIgnoreCase(parts[i + 1], "hash") && i + 2 < parts.size())
                    {
                        target = {LiveChatLinkKind::BeatSaverHash, parts[i + 2], "", url};
                        return true;
                    }

                    target = {LiveChatLinkKind::BeatSaverId, parts[i + 1], "", url};
                    return true;
                }
            }

            return false;
        }

        bool TryCreateTarget(const std::smatch& match, const std::string& token, LiveChatLinkTarget& target)
        {
            if (match.size() > 2 && match[2].matched)
            {
                std::string id = match[2].str();
                target = {LiveChatLinkKind::BeatSaverId, id, "", "https://beatsaver.com/maps/" + id};
                return true;
            }

            std::smatch urlMatch;
            if (!std::regex_search(token, urlMatch, UrlPattern))
            {
                return false;
            }

            std::string host = ToLower(urlMatch[2].str());
            // c# Uri.Host drops the port
            size_t portPos = host.rfind(':');
            if (portPos != std::string::npos)
            {
                host = host.substr(0, portPos);
            }
            std::string path = urlMatch[3].str();

            if (EndsWith(host, "beatsaver.com"))
            {
                return TryResolveBeatSaverUri(token, path, target);
            }

            if (EndsWith(host, "snoresaber.com"))
            {
                if (TryResolveSnoreSaberUri(token, path, target))
                {
                    return true;
                }

                std::smatch hashMatch;
                if (std::regex_search(token, hashMatch, HashPattern))
                {
                    target = {LiveChatLinkKind::BeatSaverHash, hashMatch.str(), "", token};
                    return true;
                }

                target = {LiveChatLinkKind::ExternalUrl, token, "", token};
                return true;
            }

            return false;
        }

        const Core::Api::Generated::MapDetailsResponseLeaderboardsItem* FindLeaderboard(const Core::Api::Generated::MapDetailsResponse& map, const std::string& leaderboardIdText)
        {
            int leaderboardId = 0;
            if (!TryParseInt(leaderboardIdText, leaderboardId))
            {
                return nullptr;
            }

            for (const auto& leaderboard : map.Leaderboards)
            {
                if (std::abs(leaderboard.Id - leaderboardId) < 0.5)
                {
                    return &leaderboard;
                }
            }

            return nullptr;
        }

        std::string RawDifficultyPart(const std::string& rawDifficulty, int index)
        {
            if (IsBlank(rawDifficulty))
            {
                return "";
            }

            size_t begin = rawDifficulty.find_first_not_of('_');
            size_t end = rawDifficulty.find_last_not_of('_');
            if (begin == std::string::npos)
            {
                return "";
            }
            std::string trimmed = rawDifficulty.substr(begin, end - begin + 1);

            std::vector<std::string> parts;
            size_t start = 0;
            while (true)
            {
                size_t underscore = trimmed.find('_', start);
                if (underscore == std::string::npos)
                {
                    parts.push_back(trimmed.substr(start));
                    break;
                }
                parts.push_back(trimmed.substr(start, underscore - start));
                start = underscore + 1;
            }

            return index >= 0 && index < static_cast<int>(parts.size()) ? parts[index] : "";
        }

        std::string DifficultyName(const Core::Api::Generated::MapDetailsResponseLeaderboardsItem* leaderboard)
        {
            std::string rawDifficulty = RawDifficultyPart(leaderboard ? leaderboard->RawDifficulty : "", 0);
            if (!rawDifficulty.empty())
            {
                return rawDifficulty;
            }

            switch (static_cast<int>(leaderboard ? leaderboard->Difficulty : 0))
            {
                case 1:
                    return "Easy";
                case 3:
                    return "Normal";
                case 5:
                    return "Hard";
                case 7:
                    return "Expert";
                case 9:
                    return "ExpertPlus";
                default:
                    return "";
            }
        }

        std::string CharacteristicName(const Core::Api::Generated::MapDetailsResponseLeaderboardsItem* leaderboard)
        {
            std::string gameMode = leaderboard ? leaderboard->GameMode : "";
            if (IsBlank(gameMode))
            {
                gameMode = RawDifficultyPart(leaderboard ? leaderboard->RawDifficulty : "", 1);
            }

            if (IsBlank(gameMode))
            {
                return "";
            }

            return gameMode.size() >= 4 && EqualsIgnoreCase(gameMode.substr(0, 4), "Solo")
                       ? gameMode.substr(4)
                       : gameMode;
        }
    }

    void LiveChatLinkService::ctor(Core::BeatSaver::BeatSaverService* beatSaver,
                                   Compete::Services::CompeteSongService* songService,
                                   LiveChatSongNavigator* songNavigator,
                                   LudusSessionService* ludusSession)
    {
        INVOKE_CTOR();
        _beatSaver = beatSaver;
        _songService = songService;
        _songNavigator = songNavigator;
        _ludusSession = ludusSession;
    }

    std::optional<LiveChatLinkTarget> LiveChatLinkService::FirstLink(const std::string& text)
    {
        if (IsBlank(text))
        {
            return std::nullopt;
        }

        for (auto it = std::sregex_iterator(text.begin(), text.end(), LinkPattern); it != std::sregex_iterator(); ++it)
        {
            std::string token = TrimToken(it->str());
            LiveChatLinkTarget target;
            if (TryCreateTarget(*it, token, target))
            {
                return target;
            }
        }

        return std::nullopt;
    }

    std::string LiveChatLinkService::DisplaySenderName(const Domain::LiveChatEntry& entry)
    {
        std::string senderName = CleanDisplayName(entry.senderName);
        if (!IsBlank(senderName) && senderName != "Player")
        {
            return senderName;
        }

        if (IsBlank(entry.senderPlayerId))
        {
            return "Unknown";
        }

        return FirstNonEmpty({ResolvedPlayerName(entry.senderPlayerId), "Loading player"});
    }

    std::string LiveChatLinkService::DisplayText(const Domain::LiveChatEntry& entry)
    {
        if (entry.IsChat())
        {
            return entry.text;
        }

        std::smatch playerLog;
        if (std::regex_search(entry.text, playerLog, RawPlayerRoomLogPattern))
        {
            std::string playerName = LogPlayerName(entry, "", playerLog[1].str());
            return fmt::format("{:s} {:s} the room", playerName, playerLog[2].str());
        }

        std::smatch namedPlayerLog;
        if (std::regex_search(entry.text, namedPlayerLog, NamedPlayerRoomLogPattern))
        {
            std::string playerName = LogPlayerName(entry, namedPlayerLog[1].str(), "");
            return fmt::format("{:s} {:s} the room", playerName, namedPlayerLog[2].str());
        }

        std::smatch mapLog;
        if (std::regex_search(entry.text, mapLog, RawLoadedMapLogPattern))
        {
            std::string hash = ToUpper(mapLog[1].str());
            std::string mapName = FirstNonEmpty({ResolvedMapName(hash), "map"});
            return fmt::format("Loaded {:s}{:s}", mapName, NormalizeLogSuffix(mapLog[2].str()));
        }

        return StripDisplayMarkup(entry.text);
    }

    std::string LiveChatLinkService::LogPlayerName(const Domain::LiveChatEntry& entry, const std::string& fallbackName, const std::string& fallbackPlayerId)
    {
        std::string senderName = CleanDisplayName(entry.senderName);
        if (!IsBlank(senderName) && senderName != "Ludus" && senderName != "Player")
        {
            return senderName;
        }

        std::string playerId = FirstNonEmpty({fallbackPlayerId, entry.senderPlayerId});
        if (!IsBlank(playerId))
        {
            return FirstNonEmpty({ResolvedPlayerName(playerId), "Loading player"});
        }

        std::string fallback = CleanDisplayName(fallbackName);
        return !IsBlank(fallback) && fallback != "Player" ? fallback : "Unknown player";
    }

    void LiveChatLinkService::Open(const std::optional<LiveChatLinkTarget>& target, const CancellationToken& cancellationToken)
    {
        if (!target)
        {
            return;
        }

        if (target->kind == LiveChatLinkKind::ExternalUrl)
        {
            std::string url = target->url;
            Utils::Async::Main([url] { UnityEngine::Application::OpenURL(url); });
            return;
        }

        try
        {
            InvokeStatus("Resolving linked map...");
            std::optional<V1::LiveSongCommand> song = SongFromTarget(*target, cancellationToken);
            if (!song || song->Hash.empty())
            {
                InvokeStatus("Could not resolve linked map.");
                return;
            }

            InvokeStatus("Checking linked map...");
            std::shared_ptr<Compete::Domain::CompeteSongSelection> selection = _songService->ResolveInstalled(&*song, cancellationToken);
            if (!selection)
            {
                InvokeStatus("Downloading linked map...");
                selection = _songService->ResolveOrDownload(&*song, cancellationToken);
            }

            if (!selection)
            {
                InvokeStatus("Linked map downloaded.");
                return;
            }

            bool roomUpdated = _ludusSession->TrySetLinkedSong(selection);
            bool focused = _songNavigator->TryFocusSong(selection, cancellationToken);
            if (focused)
            {
                InvokeStatus("Opened linked map: " + selection->name);
            }
            else if (roomUpdated)
            {
                InvokeStatus("Linked map ready in room: " + selection->name);
            }
            else
            {
                InvokeStatus("Linked map ready: " + selection->name);
            }
        }
        catch (const OperationCanceledException&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            WARN("Failed to open live chat map link: {:s}", ex.what());
            InvokeStatus("Map link failed: " + MapLinkFailureMessage(ex));
        }
    }

    std::optional<V1::LiveSongCommand> LiveChatLinkService::SongFromTarget(const LiveChatLinkTarget& target, const CancellationToken& cancellationToken)
    {
        switch (target.kind)
        {
            case LiveChatLinkKind::BeatSaverId:
                return SongFromBeatSaverId(target.value, cancellationToken);
            case LiveChatLinkKind::SnoreSaberMapId:
            case LiveChatLinkKind::SnoreSaberLeaderboardId:
                return SongFromSnoreSaberMap(target, cancellationToken);
            default: {
                V1::LiveSongCommand song;
                song.Hash = target.value;
                return song;
            }
        }
    }

    V1::LiveSongCommand LiveChatLinkService::SongFromBeatSaverId(const std::string& id, const CancellationToken& cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        Core::BeatSaver::BeatSaverMap map = _beatSaver->GetMapById(id);
        const Core::BeatSaver::BeatSaverVersion* version = map.versions.empty() ? nullptr : &map.versions.front();
        const Core::BeatSaver::BeatSaverDifficulty* diff = version && !version->diffs.empty() ? &version->diffs.front() : nullptr;

        V1::LiveSongCommand song;
        song.Hash = version ? version->hash : "";
        song.Difficulty = diff ? diff->difficulty : "";
        song.Characteristic = diff ? diff->characteristic : "";
        return song;
    }

    std::optional<V1::LiveSongCommand> LiveChatLinkService::SongFromSnoreSaberMap(const LiveChatLinkTarget& target, const CancellationToken& cancellationToken)
    {
        int mapId = 0;
        if (!TryParseInt(target.value, mapId))
        {
            return std::nullopt;
        }

        cancellationToken.ThrowIfCancellationRequested();
        Core::Api::Generated::MapDetailsResponse map = _apiClient.GetMapById(mapId);
        const Core::Api::Generated::MapDetailsResponseLeaderboardsItem* leaderboard = FindLeaderboard(map, target.secondaryValue);
        if (!IsBlank(map.Hash))
        {
            V1::LiveSongCommand song;
            song.Hash = map.Hash;
            song.Difficulty = DifficultyName(leaderboard);
            song.Characteristic = CharacteristicName(leaderboard);
            return song;
        }

        std::string bsid = map.Bsid.value_or("");
        if (IsBlank(bsid))
        {
            return std::nullopt;
        }

        return SongFromBeatSaverId(bsid, cancellationToken);
    }

    std::string LiveChatLinkService::ResolvedPlayerName(const std::string& playerId)
    {
        if (IsBlank(playerId))
        {
            return "";
        }

        auto it = _playerNames.find(playerId);
        if (it != _playerNames.end())
        {
            return it->second;
        }

        QueuePlayerResolution(playerId);
        return "";
    }

    std::string LiveChatLinkService::ResolvedMapName(const std::string& hash)
    {
        if (IsBlank(hash))
        {
            return "";
        }

        auto it = _mapNames.find(hash);
        if (it != _mapNames.end())
        {
            return it->second;
        }

        QueueMapResolution(hash);
        return "";
    }

    void LiveChatLinkService::QueuePlayerResolution(const std::string& playerId)
    {
        if (_pendingPlayerNames.contains(playerId) || _playerNames.contains(playerId))
        {
            return;
        }

        _pendingPlayerNames.insert(playerId);
        ResolvePlayerName(playerId);
    }

    void LiveChatLinkService::ResolvePlayerName(const std::string& playerId)
    {
        Utils::Async::Run([this, playerId] {
            std::string name;
            try
            {
                Data::Player player = _apiClient.GetPlayerProfile(playerId, false);
                name = FirstNonEmpty({CleanDisplayName(Paper::StringConvert::from_utf16(player.name))});
            }
            catch (const std::exception& ex)
            {
                WARN("Failed to resolve live chat player {:s}: {:s}", playerId.c_str(), ex.what());
            }

            Utils::Async::Main([this, playerId, name] {
                _pendingPlayerNames.erase(playerId);
                _playerNames[playerId] = name;
                ResolvedTextChanged.Invoke();
            });
        });
    }

    void LiveChatLinkService::QueueMapResolution(const std::string& hash)
    {
        if (_pendingMapNames.contains(hash) || _mapNames.contains(hash))
        {
            return;
        }

        _pendingMapNames.insert(hash);
        ResolveMapName(hash);
    }

    void LiveChatLinkService::ResolveMapName(const std::string& hash)
    {
        Utils::Async::Run([this, hash] {
            std::string name;
            try
            {
                Core::Api::Generated::MapDetailsResponse map = _apiClient.GetMapByHash(hash);
                name = FormatMapName(map);
            }
            catch (const std::exception& ex)
            {
                WARN("Failed to resolve live chat map {:s}: {:s}", hash.c_str(), ex.what());
            }

            Utils::Async::Main([this, hash, name] {
                _pendingMapNames.erase(hash);
                _mapNames[hash] = name;
                ResolvedTextChanged.Invoke();
            });
        });
    }

    void LiveChatLinkService::InvokeStatus(const std::string& status)
    {
        // pc status events fire on the unity sync context
        Utils::Async::Main([this, status] { StatusChanged.Invoke(status); });
    }
}
