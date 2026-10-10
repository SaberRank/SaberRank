#include "Features/Live/Compete/Services/CompeteSongService.hpp"

#include "Utils/AsyncUtils.hpp"
#include "logging.hpp"

#include <GlobalNamespace/BeatmapBasicData.hpp>
#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/BeatmapDifficulty.hpp>
#include <GlobalNamespace/BeatmapDifficultyMethods.hpp>
#include <System/Collections/Generic/Dictionary_2.hpp>
#include <System/ValueTuple_2.hpp>
#include <songcore/shared/SongCore.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fmt/format.h>
#include <future>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::Services, CompeteSongService);

namespace SnoreSaber::Features::Live::Compete::Services
{
    using namespace SnoreSaber::Core::Api::Generated;
    using namespace SnoreSaber::Core::BeatSaver;
    using SnoreSaber::Live::V1::LiveSongCommand;

    namespace
    {
        constexpr int SongRefreshTimeoutMs = 30000;

        // concurrent ResolveOrDownload calls for the same hash share one download+refresh,
        // mirroring the pc static _mapDownloadsByHash task map
        struct SharedDownload
        {
            std::promise<void> promise;
            std::shared_future<void> future;
        };
        std::mutex mapDownloadsLock;
        std::map<std::string, std::shared_ptr<SharedDownload>> mapDownloadsByHash;

        bool IsBlank(const std::string& value)
        {
            return std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
        }

        std::string ToLower(std::string value)
        {
            for (auto& c : value)
            {
                c = std::tolower(static_cast<unsigned char>(c));
            }
            return value;
        }

        std::string ToUpper(std::string value)
        {
            for (auto& c : value)
            {
                c = std::toupper(static_cast<unsigned char>(c));
            }
            return value;
        }

        std::string FromStringW(StringW value)
        {
            return value ? static_cast<std::string>(value) : "";
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

        std::string FirstDetailValue(std::initializer_list<std::string> values)
        {
            for (const auto& value : values)
            {
                if (!IsBlank(value) && value != "--")
                {
                    return value;
                }
            }
            return FirstNonEmpty(values);
        }

        std::string SongHash(const LiveSongCommand* song)
        {
            return BeatSaverService::NormalizeHash(song ? song->Hash : "");
        }

        // beatsaver metadata can carry stray newlines (a leading "\n" renders as
        // "..." in single-line tmp labels), so collapse whitespace runs
        std::string CollapseWhitespace(const std::string& value)
        {
            std::string result;
            bool pendingSpace = false;
            for (unsigned char c : value)
            {
                if (std::isspace(c))
                {
                    pendingSpace = !result.empty();
                    continue;
                }
                if (pendingSpace)
                {
                    result += ' ';
                    pendingSpace = false;
                }
                result += static_cast<char>(c);
            }
            return result;
        }

        std::string DisplaySongName(const std::string& songName, const std::string& songSubName)
        {
            return CollapseWhitespace(songSubName.empty() ? songName : songName + " " + songSubName);
        }

        std::string MapperName(const std::vector<std::string>& mappers)
        {
            std::string joined;
            for (const auto& mapper : mappers)
            {
                if (IsBlank(mapper))
                {
                    continue;
                }
                if (!joined.empty())
                {
                    joined += ", ";
                }
                joined += mapper;
            }
            return joined.empty() ? "Unknown" : joined;
        }

        std::string FormatDifficulty(const std::string& difficulty)
        {
            return ToLower(difficulty) == "expertplus" ? "Expert+" : difficulty;
        }

        std::string RawDifficultyPart(const std::string& rawDifficulty, int index)
        {
            if (IsBlank(rawDifficulty))
            {
                return "";
            }

            std::string trimmed = rawDifficulty;
            trimmed.erase(0, trimmed.find_first_not_of('_'));
            trimmed.erase(trimmed.find_last_not_of('_') + 1);

            std::vector<std::string> parts;
            size_t start = 0;
            while (true)
            {
                size_t split = trimmed.find('_', start);
                if (split == std::string::npos)
                {
                    parts.push_back(trimmed.substr(start));
                    break;
                }
                parts.push_back(trimmed.substr(start, split - start));
                start = split + 1;
            }

            return index >= 0 && index < static_cast<int>(parts.size()) ? parts[index] : "";
        }

        std::string DifficultyName(const MapDetailsResponseLeaderboardsItem* leaderboard)
        {
            std::string rawDifficulty = RawDifficultyPart(leaderboard ? leaderboard->RawDifficulty : std::string(), 0);
            if (!rawDifficulty.empty())
            {
                return rawDifficulty;
            }

            switch (leaderboard ? static_cast<int>(leaderboard->Difficulty) : 0)
            {
                case 1: return "Easy";
                case 3: return "Normal";
                case 5: return "Hard";
                case 7: return "Expert";
                case 9: return "ExpertPlus";
                default: return "";
            }
        }

        std::string CharacteristicName(const MapDetailsResponseLeaderboardsItem* leaderboard)
        {
            std::string gameMode = leaderboard ? leaderboard->GameMode : std::string();
            if (IsBlank(gameMode))
            {
                gameMode = RawDifficultyPart(leaderboard ? leaderboard->RawDifficulty : std::string(), 1);
            }

            if (IsBlank(gameMode))
            {
                return "";
            }

            return ToLower(gameMode.substr(0, 4)) == "solo" ? gameMode.substr(4) : gameMode;
        }

        std::string NormalizeDifficultyName(const std::string& difficulty)
        {
            std::string normalized;
            for (char c : difficulty)
            {
                if (c == '+')
                {
                    normalized += "Plus";
                }
                else if (c != '_' && c != ' ')
                {
                    normalized.push_back(c);
                }
            }
            return ToLower(normalized);
        }

        std::string NormalizeCharacteristicName(const std::string& characteristic)
        {
            std::string normalized;
            for (char c : characteristic)
            {
                if (c != '_' && c != ' ')
                {
                    normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                }
            }

            if (normalized.starts_with("solo"))
            {
                normalized = normalized.substr(4);
            }

            if (normalized == "90degree" || normalized == "generated90degree")
            {
                return "ninetydegree";
            }
            if (normalized == "360degree" || normalized == "generated360degree")
            {
                return "threesixtydegree";
            }
            return normalized;
        }

        const MapDetailsResponseLeaderboardsItem* SelectLeaderboard(const MapDetailsResponse& map, const LiveSongCommand* song)
        {
            if (map.Leaderboards.empty())
            {
                return nullptr;
            }

            std::string difficulty = NormalizeDifficultyName(song ? song->Difficulty : "");
            std::string characteristic = NormalizeCharacteristicName(song ? song->Characteristic : "");
            std::vector<const MapDetailsResponseLeaderboardsItem*> leaderboards;
            for (const auto& leaderboard : map.Leaderboards)
            {
                if (difficulty.empty() || NormalizeDifficultyName(DifficultyName(&leaderboard)) == difficulty)
                {
                    leaderboards.push_back(&leaderboard);
                }
            }

            if (leaderboards.empty())
            {
                for (const auto& leaderboard : map.Leaderboards)
                {
                    leaderboards.push_back(&leaderboard);
                }
            }

            if (!characteristic.empty())
            {
                for (const auto* leaderboard : leaderboards)
                {
                    if (NormalizeCharacteristicName(CharacteristicName(leaderboard)) == characteristic)
                    {
                        return leaderboard;
                    }
                }
            }

            return leaderboards.front();
        }

        std::string FormatDuration(double seconds)
        {
            if (seconds <= 0)
            {
                return "--";
            }

            int totalSeconds = static_cast<int>(seconds);
            return fmt::format("{}:{:02}", totalSeconds / 60, totalSeconds % 60);
        }

        std::string FormatDuration(const std::optional<float>& seconds)
        {
            return seconds.has_value() ? FormatDuration(*seconds) : "--";
        }

        std::string FormatWholeNumber(double value)
        {
            return value > 0 ? fmt::format("{:.0f}", value) : "--";
        }

        std::string FormatWholeNumber(const std::optional<float>& value)
        {
            return value.has_value() ? FormatWholeNumber(*value) : "--";
        }

        std::string FormatTwoDecimals(const std::optional<float>& value)
        {
            return value.has_value() && *value > 0 ? fmt::format("{:.2f}", *value) : "--";
        }

        // .NET "0.0#": one to two decimals, trailing zero in the second place trimmed
        std::string FormatUpToTwoDecimals(float value)
        {
            std::string formatted = fmt::format("{:.2f}", value);
            if (formatted.back() == '0')
            {
                formatted.pop_back();
            }
            return formatted;
        }

        std::string FormatUpToTwoDecimals(const std::optional<float>& value)
        {
            return value.has_value() && *value > 0 ? FormatUpToTwoDecimals(*value) : "--";
        }

        std::string FormatStars(double value)
        {
            return value > 0.0 ? fmt::format("{:.2f}", value) : "--";
        }

        std::string FormatInt(int value)
        {
            return value > 0 ? std::to_string(value) : "--";
        }

        std::string NotesPerSecond(int cuttableObjectsCount, float duration)
        {
            return duration <= 0 ? "--" : fmt::format("{:.2f}", cuttableObjectsCount / duration);
        }

        float JumpDistance(float bpm, float njs, float offset)
        {
            float oneBeatDuration = 60.0f / bpm;
            float halfJumpDuration = 4.0f;
            while (njs * oneBeatDuration * halfJumpDuration > 17.999f)
            {
                halfJumpDuration /= 2.0f;
            }

            halfJumpDuration += offset;
            return njs * oneBeatDuration * std::max(halfJumpDuration, 0.25f) * 2.0f;
        }

        std::string PreviewJumpDistance(const BeatSaverMapMetadata* metadata, const BeatSaverDifficulty* diff)
        {
            if (!metadata || !diff || !metadata->bpm.has_value() || !diff->njs.has_value() || *metadata->bpm <= 0 || *diff->njs <= 0)
            {
                return "--";
            }

            return FormatUpToTwoDecimals(JumpDistance(*metadata->bpm, *diff->njs, diff->offset.value_or(0.0f)));
        }

        bool TryParseBeatmapDifficulty(const std::string& value, GlobalNamespace::BeatmapDifficulty& difficulty)
        {
            std::string normalized = ToLower(value);
            if (normalized == "0" || normalized == "easy")
            {
                difficulty = GlobalNamespace::BeatmapDifficulty::Easy;
            }
            else if (normalized == "1" || normalized == "normal")
            {
                difficulty = GlobalNamespace::BeatmapDifficulty::Normal;
            }
            else if (normalized == "2" || normalized == "hard")
            {
                difficulty = GlobalNamespace::BeatmapDifficulty::Hard;
            }
            else if (normalized == "3" || normalized == "expert")
            {
                difficulty = GlobalNamespace::BeatmapDifficulty::Expert;
            }
            else if (normalized == "4" || normalized == "expertplus")
            {
                difficulty = GlobalNamespace::BeatmapDifficulty::ExpertPlus;
            }
            else
            {
                return false;
            }
            return true;
        }

        // key.difficulty.ToString().Replace("Plus", "+") on pc
        std::string DifficultyDisplayName(GlobalNamespace::BeatmapDifficulty difficulty)
        {
            switch (difficulty.value__)
            {
                case 0: return "Easy";
                case 1: return "Normal";
                case 2: return "Hard";
                case 3: return "Expert";
                case 4: return "Expert+";
                default: return fmt::format("{}", difficulty.value__);
            }
        }

        std::vector<GlobalNamespace::BeatmapKey> CollectBeatmapKeys(GlobalNamespace::BeatmapLevel* level)
        {
            std::vector<GlobalNamespace::BeatmapKey> keys;
            auto source = level->__cordl_internal_get__beatmapBasicDatas();
            if (!source)
            {
                return keys;
            }

            auto entries = source->__cordl_internal_get__entries();
            if (!entries)
            {
                return keys;
            }

            int count = std::min<int>(source->__cordl_internal_get__count(), entries.size());
            for (int i = 0; i < count; i++)
            {
                auto entry = entries[i];
                if (entry.hashCode >= 0 && entry.key.Item1 && entry.value)
                {
                    keys.emplace_back(entry.key.Item1, entry.key.Item2, level->levelID);
                }
            }
            return keys;
        }

        std::optional<GlobalNamespace::BeatmapKey> FindBeatmapKey(GlobalNamespace::BeatmapLevel* level, const LiveSongCommand* song)
        {
            GlobalNamespace::BeatmapDifficulty difficulty = GlobalNamespace::BeatmapDifficulty::Easy;
            bool hasDifficulty = TryParseBeatmapDifficulty(BeatSaverService::NormalizeDifficulty(song ? song->Difficulty : ""), difficulty);
            std::string characteristic = NormalizeCharacteristicName(song ? song->Characteristic : "");
            std::optional<GlobalNamespace::BeatmapKey> difficultyMatch;

            auto keys = CollectBeatmapKeys(level);
            for (auto key : keys)
            {
                if (hasDifficulty && key.difficulty != difficulty)
                {
                    continue;
                }

                if (!difficultyMatch.has_value())
                {
                    difficultyMatch = key;
                }

                std::string keyCharacteristic = key.beatmapCharacteristic ? FromStringW(key.beatmapCharacteristic->get_serializedName()) : "";
                if (characteristic.empty() || NormalizeCharacteristicName(keyCharacteristic) == characteristic)
                {
                    return key;
                }
            }

            if (difficultyMatch.has_value())
            {
                return difficultyMatch;
            }
            return keys.empty() ? std::nullopt : std::optional(keys.front());
        }

        std::vector<std::string> MappersFor(GlobalNamespace::BeatmapLevel* level, GlobalNamespace::BeatmapBasicData* beatmapData)
        {
            std::vector<std::string> mappers;
            if (ArrayW<StringW> beatmapMappers = beatmapData->mappers)
            {
                for (StringW mapper : beatmapMappers)
                {
                    std::string name = FromStringW(mapper);
                    if (!IsBlank(name))
                    {
                        mappers.push_back(name);
                    }
                }
            }

            if (!mappers.empty())
            {
                return mappers;
            }

            if (ArrayW<StringW> allMappers = level->allMappers)
            {
                for (StringW mapper : allMappers)
                {
                    mappers.push_back(FromStringW(mapper));
                }
            }
            return mappers;
        }

        std::shared_ptr<Domain::CompeteSongSelection> CreateSongSelection(GlobalNamespace::BeatmapLevel* level, GlobalNamespace::BeatmapKey key, const LiveSongCommand* song, const std::optional<LiveSongDetails>& scoreSaberDetails)
        {
            auto beatmapData = level->GetDifficultyBeatmapData(key.beatmapCharacteristic, key.difficulty);
            if (!beatmapData)
            {
                return nullptr;
            }

            float njs = GlobalNamespace::BeatmapDifficultyMethods::NoteJumpMovementSpeed(key.difficulty, beatmapData->noteJumpMovementSpeed, false);

            auto selection = std::make_shared<Domain::CompeteSongSelection>();
            selection->beatmapLevel = level;
            selection->beatmapKey = key;
            selection->name = DisplaySongName(FromStringW(level->songName), FromStringW(level->songSubName));
            selection->mapper = MapperName(MappersFor(level, beatmapData));
            selection->difficulty = DifficultyDisplayName(key.difficulty);
            selection->characteristic = key.beatmapCharacteristic ? FromStringW(key.beatmapCharacteristic->get_serializedName()) : "";
            selection->coverSource = "";
            selection->duration = FormatDuration(level->songDuration);
            selection->bpm = fmt::format("{}", static_cast<long long>(std::llround(level->beatsPerMinute)));
            selection->nps = NotesPerSecond(beatmapData->cuttableObjectsCount, level->songDuration);
            selection->notes = FormatInt(beatmapData->notesCount);
            selection->obstacles = FormatInt(beatmapData->obstaclesCount);
            selection->bombs = FormatInt(beatmapData->bombsCount);
            selection->njs = FormatUpToTwoDecimals(njs);
            selection->jumpDistance = FormatUpToTwoDecimals(JumpDistance(level->beatsPerMinute, njs, beatmapData->noteJumpStartBeatOffset));
            selection->stars = FirstDetailValue({ scoreSaberDetails.has_value() ? scoreSaberDetails->Stars : std::string(), "--" });
            selection->mapHash = SongHash(song);
            selection->downloadUrl = "";
            return selection;
        }

        LiveSongDetails BuildSnoreSaberSongDetails(const LiveSongCommand* song, const MapDetailsResponse& map)
        {
            std::string hash = ToUpper(FirstNonEmpty({ map.Hash, SongHash(song) }));
            const auto* leaderboard = SelectLeaderboard(map, song);
            std::string name = DisplaySongName(map.SongName, map.SongSubName);

            LiveSongDetails details;
            details.Hash = hash;
            details.Name = FirstNonEmpty({ name, hash });
            details.Mapper = FirstNonEmpty({ map.LevelAuthorName, map.SongAuthorName, "Unknown" });
            details.Difficulty = DifficultyName(leaderboard);
            details.Characteristic = CharacteristicName(leaderboard);
            details.CoverUrl = map.CoverUrl;
            details.Bpm = FormatWholeNumber(map.Bpm);
            details.Stars = FormatStars(leaderboard ? leaderboard->Realm.Stars : 0.0);
            return details;
        }

        LiveSongDetails MergeSongDetails(const std::optional<LiveSongDetails>& scoreSaber, const LiveSongDetails& beatSaver)
        {
            if (!scoreSaber.has_value())
            {
                return beatSaver;
            }

            LiveSongDetails merged;
            merged.Hash = FirstNonEmpty({ scoreSaber->Hash, beatSaver.Hash });
            merged.Name = FirstNonEmpty({ scoreSaber->Name, beatSaver.Name });
            merged.Mapper = FirstNonEmpty({ scoreSaber->Mapper, beatSaver.Mapper });
            merged.Difficulty = FirstNonEmpty({ scoreSaber->Difficulty, beatSaver.Difficulty });
            merged.Characteristic = FirstNonEmpty({ scoreSaber->Characteristic, beatSaver.Characteristic });
            merged.CoverUrl = FirstNonEmpty({ scoreSaber->CoverUrl, beatSaver.CoverUrl });
            merged.DownloadUrl = FirstNonEmpty({ beatSaver.DownloadUrl, scoreSaber->DownloadUrl });
            merged.Duration = FirstDetailValue({ scoreSaber->Duration, beatSaver.Duration });
            merged.Bpm = FirstDetailValue({ scoreSaber->Bpm, beatSaver.Bpm });
            merged.Nps = FirstDetailValue({ scoreSaber->Nps, beatSaver.Nps });
            merged.Notes = FirstDetailValue({ scoreSaber->Notes, beatSaver.Notes });
            merged.Obstacles = FirstDetailValue({ scoreSaber->Obstacles, beatSaver.Obstacles });
            merged.Bombs = FirstDetailValue({ scoreSaber->Bombs, beatSaver.Bombs });
            merged.Njs = FirstDetailValue({ scoreSaber->Njs, beatSaver.Njs });
            merged.JumpDistance = FirstDetailValue({ scoreSaber->JumpDistance, beatSaver.JumpDistance });
            merged.Stars = FirstDetailValue({ scoreSaber->Stars, beatSaver.Stars });
            return merged;
        }

        std::shared_ptr<Domain::CompeteSongSelection> CreatePreview(const LiveSongCommand* song, const LiveSongDetails& details)
        {
            if (!song)
            {
                return nullptr;
            }

            std::string hash = ToUpper(SongHash(song));
            auto selection = std::make_shared<Domain::CompeteSongSelection>();
            selection->name = FirstNonEmpty({ details.Name, hash });
            selection->mapper = FirstNonEmpty({ details.Mapper, "Unknown" });
            selection->difficulty = FirstNonEmpty({ FormatDifficulty(song->Difficulty), FormatDifficulty(details.Difficulty) });
            selection->characteristic = details.Characteristic;
            selection->coverSource = details.CoverUrl;
            selection->duration = details.Duration;
            selection->bpm = details.Bpm;
            selection->nps = details.Nps;
            selection->notes = details.Notes;
            selection->obstacles = details.Obstacles;
            selection->bombs = details.Bombs;
            selection->njs = details.Njs;
            selection->jumpDistance = details.JumpDistance;
            selection->stars = details.Stars;
            selection->mapHash = FirstNonEmpty({ details.Hash, hash });
            selection->downloadUrl = details.DownloadUrl;
            return selection;
        }

        // pc waits on the reflection-hooked SongsLoadedEvent; quest songcore hands
        // out a shared_future for the refresh instead, so these polls replace it

        void WaitForCurrentSongRefresh(const CancellationToken& cancellationToken)
        {
            if (!SongCore::API::Loading::AreSongsRefreshing())
            {
                return;
            }

            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(SongRefreshTimeoutMs);
            while (SongCore::API::Loading::AreSongsRefreshing())
            {
                cancellationToken.ThrowIfCancellationRequested();
                if (std::chrono::steady_clock::now() >= deadline)
                {
                    throw std::runtime_error("Timed out waiting for SongCore to refresh songs");
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            }
        }

        void StartSongRefresh(const CancellationToken& cancellationToken)
        {
            auto started = std::make_shared<std::promise<std::shared_future<void>>>();
            auto startedFuture = started->get_future();
            Utils::Async::Main([started] {
                started->set_value(SongCore::API::Loading::RefreshSongs(false));
            });

            while (startedFuture.wait_for(std::chrono::milliseconds(25)) != std::future_status::ready)
            {
                cancellationToken.ThrowIfCancellationRequested();
            }

            std::shared_future<void> refresh = startedFuture.get();
            if (!refresh.valid())
            {
                return;
            }

            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(SongRefreshTimeoutMs);
            while (refresh.wait_for(std::chrono::milliseconds(25)) != std::future_status::ready)
            {
                cancellationToken.ThrowIfCancellationRequested();
                if (std::chrono::steady_clock::now() >= deadline)
                {
                    throw std::runtime_error("Timed out waiting for SongCore to refresh songs");
                }
            }
        }

        void RefreshSongs(const CancellationToken& cancellationToken)
        {
            WaitForCurrentSongRefresh(cancellationToken);
            StartSongRefresh(cancellationToken);
        }

        void RemoveDownloadEntry(const std::string& downloadKey, const std::shared_ptr<SharedDownload>& download)
        {
            std::lock_guard<std::mutex> lock(mapDownloadsLock);
            auto it = mapDownloadsByHash.find(downloadKey);
            if (it != mapDownloadsByHash.end() && it->second == download)
            {
                mapDownloadsByHash.erase(it);
            }
        }
    }

    void CompeteSongService::ctor(Core::BeatSaver::BeatSaverService* beatSaver)
    {
        INVOKE_CTOR();
        _beatSaver = beatSaver;
    }

    std::shared_ptr<Domain::CompeteSongSelection> CompeteSongService::ResolveOrDownload(const LiveSongCommand* song, const CancellationToken& cancellationToken)
    {
        auto installed = ResolveInstalled(song, cancellationToken);
        if (installed)
        {
            return installed;
        }

        auto scoreSaberDetails = TryFetchSnoreSaberSongDetails(song, cancellationToken);
        auto map = TryFetchBeatSaverMap(song, cancellationToken);
        const BeatSaverVersion* version = _beatSaver->SelectVersion(map.has_value() ? &*map : nullptr, SongHash(song));
        LiveSongDetails details = MergeSongDetails(scoreSaberDetails, BuildBeatSaverSongDetails(song, map.has_value() ? &*map : nullptr, version));
        DownloadAndRefresh(SongHash(song), version, cancellationToken);

        auto resolved = ResolveInstalled(song, details, cancellationToken);
        if (resolved)
        {
            return resolved;
        }

        RefreshSongs(cancellationToken);
        resolved = ResolveInstalled(song, details, cancellationToken);
        if (!resolved)
        {
            // distinguish "songcore never loaded the folder" from "loaded but no
            // usable beatmap key" for the next time this reproduces on device
            std::string lowerHash = ToLower(SongHash(song));
            std::error_code folderError;
            bool folderExists = std::filesystem::exists(std::filesystem::path(SongCore::API::Loading::GetPreferredCustomLevelPath()) / lowerHash, folderError);
            bool levelLoaded = SongCore::API::Loading::GetLevelByHash(ToUpper(lowerHash));
            WARN("SongCore did not resolve hash {} after download (folder on disk: {}, level loaded: {})", lowerHash, folderExists, levelLoaded);
            throw std::runtime_error("SongCore could not resolve the downloaded song");
        }

        return resolved;
    }

    std::shared_ptr<Domain::CompeteSongSelection> CompeteSongService::ResolveInstalled(const LiveSongCommand* song, const CancellationToken& cancellationToken)
    {
        auto scoreSaberDetails = TryFetchSnoreSaberSongDetails(song, cancellationToken);
        return ResolveInstalled(song, scoreSaberDetails, cancellationToken);
    }

    std::shared_ptr<Domain::CompeteSongSelection> CompeteSongService::ResolveInstalled(const LiveSongCommand* song, const std::optional<LiveSongDetails>& scoreSaberDetails, const CancellationToken& cancellationToken)
    {
        std::string hash = SongHash(song);
        if (hash.empty())
        {
            return nullptr;
        }

        cancellationToken.ThrowIfCancellationRequested();
        auto level = SongCore::API::Loading::GetLevelByHash(ToUpper(hash));
        if (!level)
        {
            return nullptr;
        }

        auto key = FindBeatmapKey(level, song);
        if (!key.has_value())
        {
            return nullptr;
        }

        return CreateSongSelection(level, *key, song, scoreSaberDetails);
    }

    std::shared_ptr<Domain::CompeteSongSelection> CompeteSongService::CreatePreview(const LiveSongCommand* song, const CancellationToken& cancellationToken)
    {
        if (!song)
        {
            return nullptr;
        }

        auto scoreSaberDetails = TryFetchSnoreSaberSongDetails(song, cancellationToken);
        auto map = TryFetchBeatSaverMap(song, cancellationToken);
        const BeatSaverVersion* version = _beatSaver->SelectVersion(map.has_value() ? &*map : nullptr, SongHash(song));
        return Services::CreatePreview(song, MergeSongDetails(scoreSaberDetails, BuildBeatSaverSongDetails(song, map.has_value() ? &*map : nullptr, version)));
    }

    std::optional<LiveSongDetails> CompeteSongService::TryFetchSnoreSaberSongDetails(const LiveSongCommand* song, const CancellationToken& cancellationToken)
    {
        std::string hash = SongHash(song);
        if (hash.empty())
        {
            return std::nullopt;
        }

        try
        {
            cancellationToken.ThrowIfCancellationRequested();
            auto map = _apiClient.GetMapByHash(hash);
            return BuildSnoreSaberSongDetails(song, map);
        }
        catch (const OperationCanceledException&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            WARN("Unable to fetch SnoreSaber live song details: {}", ex.what());
            return std::nullopt;
        }
    }

    std::optional<BeatSaverMap> CompeteSongService::TryFetchBeatSaverMap(const LiveSongCommand* song, const CancellationToken& cancellationToken)
    {
        try
        {
            cancellationToken.ThrowIfCancellationRequested();
            return _beatSaver->GetMapByHash(SongHash(song));
        }
        catch (const OperationCanceledException&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            WARN("Unable to fetch BeatSaver live song details: {}", ex.what());
            return std::nullopt;
        }
    }

    LiveSongDetails CompeteSongService::BuildBeatSaverSongDetails(const LiveSongCommand* song, const BeatSaverMap* map, const BeatSaverVersion* version)
    {
        std::string hash = SongHash(song);
        const BeatSaverDifficulty* diff = _beatSaver->SelectDifficulty(version, song ? song->Difficulty : "");
        const BeatSaverMapMetadata* metadata = map && map->metadata.has_value() ? &*map->metadata : nullptr;

        std::string name = DisplaySongName(metadata ? metadata->songName : std::string(), metadata ? metadata->songSubName : std::string());
        if (name.empty())
        {
            name = FirstNonEmpty({ map ? map->name : std::string(), hash });
        }

        LiveSongDetails details;
        details.Hash = ToUpper(FirstNonEmpty({ version ? version->hash : std::string(), hash }));
        details.Name = name;
        details.Mapper = FirstNonEmpty({ metadata ? metadata->levelAuthorName : std::string(),
                                         map && map->uploader.has_value() ? map->uploader->name : std::string(),
                                         metadata ? metadata->songAuthorName : std::string(),
                                         "Unknown" });
        details.Difficulty = diff ? diff->difficulty : "";
        details.Characteristic = diff ? diff->characteristic : "";
        details.CoverUrl = version ? version->coverUrl : "";
        details.DownloadUrl = version ? version->downloadUrl : "";
        details.Duration = FormatDuration(metadata ? metadata->duration : std::optional<float>());
        details.Bpm = FormatWholeNumber(metadata ? metadata->bpm : std::optional<float>());
        details.Nps = FormatTwoDecimals(diff ? diff->nps : std::optional<float>());
        details.Notes = FormatInt(diff && diff->notes.has_value() ? *diff->notes : 0);
        details.Obstacles = FormatInt(diff && diff->obstacles.has_value() ? *diff->obstacles : 0);
        details.Bombs = FormatInt(diff && diff->bombs.has_value() ? *diff->bombs : 0);
        details.Njs = FormatUpToTwoDecimals(diff ? diff->njs : std::optional<float>());
        details.JumpDistance = PreviewJumpDistance(metadata, diff);
        return details;
    }

    void CompeteSongService::DownloadAndRefresh(const std::string& hash, const BeatSaverVersion* version, const CancellationToken& cancellationToken)
    {
        std::string normalizedHash = BeatSaverService::NormalizeHash(hash);
        std::string downloadKey = ToUpper(normalizedHash); // the pc map compares OrdinalIgnoreCase
        std::shared_ptr<SharedDownload> download;
        bool isOwner = false;
        {
            std::lock_guard<std::mutex> lock(mapDownloadsLock);
            auto existing = mapDownloadsByHash.find(downloadKey);
            if (existing != mapDownloadsByHash.end())
            {
                download = existing->second;
            }
            else
            {
                download = std::make_shared<SharedDownload>();
                download->future = download->promise.get_future().share();
                mapDownloadsByHash[downloadKey] = download;
                isOwner = true;
            }
        }

        if (isOwner)
        {
            try
            {
                DownloadAndRefreshCore(normalizedHash, version, cancellationToken);
                download->promise.set_value();
            }
            catch (...)
            {
                download->promise.set_exception(std::current_exception());
            }
        }

        try
        {
            while (download->future.wait_for(std::chrono::milliseconds(25)) != std::future_status::ready)
            {
                cancellationToken.ThrowIfCancellationRequested();
            }
            download->future.get();
        }
        catch (...)
        {
            RemoveDownloadEntry(downloadKey, download);
            throw;
        }
        RemoveDownloadEntry(downloadKey, download);
    }

    void CompeteSongService::DownloadAndRefreshCore(const std::string& hash, const BeatSaverVersion* version, const CancellationToken& cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        // quest beatsaver downloads are blocking and not cancellable mid-transfer
        _beatSaver->DownloadMapByHash(hash, version);
        RefreshSongs(cancellationToken);
    }
}
