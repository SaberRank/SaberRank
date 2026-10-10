#include "Features/ScoreSubmission/Services/ScoreUploadPayloadBuilder.hpp"

#include "Core/Gameplay/SnoreSaberGameplayModifiers.hpp"
#include "Core/Gameplay/SnoreSaberPlayOutcome.hpp"
#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"
#include "Features/ScoreSubmission/Domain/SnoreSaberUploadData.hpp"
#include "Utils/StringUtils.hpp"
#include "Utils/md5.h"
#include "logging.hpp"
#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/BeatmapDifficultyMethods.hpp>
#include <GlobalNamespace/OVRPlugin.hpp>
#include <paper2_scotland2/shared/string_convert.hpp>
#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace GlobalNamespace;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::Features::ScoreSubmission::Services, ScoreUploadPayloadBuilder);

namespace SnoreSaber::Features::ScoreSubmission::Services
{
    namespace
    {
        StringW FriendlyLevelAuthorName(ArrayW<StringW> mappers, ArrayW<StringW> lighters)
        {
            std::vector<StringW> mappersAndLighters;
            for (auto mapper : mappers)
            {
                mappersAndLighters.push_back(mapper);
            }
            for (auto lighter : lighters)
            {
                mappersAndLighters.push_back(lighter);
            }

            if (mappersAndLighters.empty())
            {
                return "";
            }
            if (mappersAndLighters.size() == 1)
            {
                return mappersAndLighters.front();
            }

            StringW result;
            for (int i = 0; i < mappersAndLighters.size() - 1; i++)
            {
                result += mappersAndLighters[i];
                if (i < mappersAndLighters.size() - 2)
                {
                    result += ", ";
                }
            }
            result += " & " + mappersAndLighters.back();
            return result;
        }

        std::vector<unsigned char> Panda(std::vector<unsigned char> scoreData, std::vector<unsigned char> key)
        {
            int n1 = 11;
            int n2 = 13;
            int ns = 257;

            for (int i = 0; i <= key.size() - 1; i++)
            {
                ns += ns % (key[i] + 1);
            }

            std::vector<unsigned char> encrypted(scoreData.size());
            for (int i = 0; i <= scoreData.size() - 1; i++)
            {
                ns = key[i % key.size()] + ns;
                n1 = (ns + 5) * (n1 & 255) + (n1 >> 8);
                n2 = (ns + 7) * (n2 & 255) + (n2 >> 8);
                ns = ((n1 << 8) + n2) & 255;
                encrypted[i] = static_cast<unsigned char>(scoreData[i] ^ static_cast<unsigned char>(ns));
            }

            return encrypted;
        }

        std::string ToHex(const std::vector<unsigned char>& bytes)
        {
            std::stringstream buffer;
            for (int i = 0; i < bytes.size(); i++)
            {
                buffer << std::hex << std::setfill('0');
                buffer << std::setw(2) << static_cast<unsigned>(bytes[i]);
            }
            return buffer.str();
        }

        std::string BuildUploadKey(SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService)
        {
            std::string playerId = gameSessionService->GetLocalPlayerId();
            return md5("f0b4a81c9bd3ded1081b365f7628781f-" + gameSessionService->GetPlayerKey() + "-" + playerId + "-f0b4a81c9bd3ded1081b365f7628781f");
        }

        std::string EncryptUploadData(const std::string& uploadData, SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService)
        {
            std::string key = BuildUploadKey(gameSessionService);
            std::vector<unsigned char> keyBytes(key.begin(), key.end());
            std::vector<unsigned char> uploadDataBytes(uploadData.begin(), uploadData.end());
            std::string encryptedScoreData = ToHex(Panda(uploadDataBytes, keyBytes));
            std::transform(encryptedScoreData.begin(), encryptedScoreData.end(), encryptedScoreData.begin(), ::toupper);
            return encryptedScoreData;
        }
    }

    void ScoreUploadPayloadBuilder::ctor(SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService,
                                         SnoreSaber::Core::SnoreSaberRuntimeInfo* runtimeInfo)
    {
        INVOKE_CTOR();
        _gameSessionService = gameSessionService;
        _runtimeInfo = runtimeInfo;
    }

    ScoreUploadPayload ScoreUploadPayloadBuilder::Build(BeatmapLevel* beatmapLevel, BeatmapKey beatmapKey, LevelCompletionResults* levelCompletionResults, float playOutcomeTime, std::optional<SnoreSaber::Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride)
    {
        std::string levelHash = SnoreSaber::Utils::SnoreSaberBeatmapKey::GetSongHash(beatmapKey);
        std::string gameMode = "Solo" + std::string(beatmapKey.beatmapCharacteristic->serializedName);
        std::string deviceHmd = fmt::format("standalone_hmd:(ovrplugin):{:s}({:d})", StringUtils::stringify_OVRPlugin_SystemHeadset(OVRPlugin::GetSystemHeadsetType()), (int)OVRPlugin::GetSystemHeadsetType());
        std::string deviceController = fmt::format("standalone_controller:(ovrplugin):{:s}({:d})", StringUtils::stringify_OVRPlugin_Controller(OVRPlugin::GetActiveController()), (int)OVRPlugin::GetActiveController());

        std::string uploadVersionHash = GetVersionHash();
        std::optional<SnoreSaber::Data::GameSession> session = _gameSessionService->GetGameSession();
        SnoreSaberUploadData data(Paper::StringConvert::from_utf8(session.has_value() ? session->playerName : ""),
                                  _gameSessionService->GetLocalPlayerId(),
                                  levelCompletionResults->multipliedScore,
                                  levelHash,
                                  beatmapLevel->songName,
                                  beatmapLevel->songSubName,
                                  FriendlyLevelAuthorName(beatmapLevel->allMappers, beatmapLevel->allLighters),
                                  beatmapLevel->songAuthorName,
                                  beatmapLevel->beatsPerMinute,
                                  BeatmapDifficultyMethods::DefaultRating(beatmapKey.difficulty),
                                  levelHash,
                                  SnoreSaber::Core::Gameplay::SnoreSaberGameplayModifiers::ToCodeList(levelCompletionResults->gameplayModifiers, levelCompletionResults->energy),
                                  gameMode,
                                  SnoreSaber::Core::Gameplay::ToString(playOutcomeOverride.value_or(SnoreSaber::Core::Gameplay::FromLevelCompletionResults(levelCompletionResults))),
                                  playOutcomeTime,
                                  levelCompletionResults->badCutsCount,
                                  levelCompletionResults->missedCount,
                                  levelCompletionResults->maxCombo,
                                  levelCompletionResults->fullCombo,
                                  deviceHmd,
                                  deviceController,
                                  deviceController);

        std::string uploadData = data.serialize();

        return {EncryptUploadData(uploadData, _gameSessionService), uploadVersionHash, levelCompletionResults->multipliedScore, uploadData};
    }

    ScoreUploadPayload ScoreUploadPayloadBuilder::RebuildWithCurrentSession(const ScoreUploadPayload& payload)
    {
        if (payload.serializedScoreData.empty())
        {
            return payload;
        }

        return {EncryptUploadData(payload.serializedScoreData, _gameSessionService), payload.uploadVersionHash, payload.multipliedScore, payload.serializedScoreData};
    }

    std::string ScoreUploadPayloadBuilder::GetVersionHash()
    {
        return _runtimeInfo->UploadVersionHash();
    }
}
