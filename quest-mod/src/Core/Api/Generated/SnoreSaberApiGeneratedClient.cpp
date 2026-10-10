#include "Core/Api/Generated/SnoreSaberApiGeneratedClient.hpp"

#include "Utils/WebUtils.hpp"

#include <beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h>
#include <beatsaber-hook/shared/rapidjson/include/rapidjson/writer.h>

#include <sstream>
#include <utility>

namespace SnoreSaber::Core::Api::Generated
{
    namespace
    {
        constexpr long TimeoutSeconds = 10;

        const rapidjson::Value* FindMember(const rapidjson::Value& value, const char* name)
        {
            if (!value.IsObject())
                return nullptr;

            auto member = value.FindMember(name);
            if (member == value.MemberEnd())
                return nullptr;
            return &member->value;
        }

        std::string ReadString(const rapidjson::Value& value)
        {
            if (value.IsString())
                return value.GetString();
            return "";
        }

        int ReadInt(const rapidjson::Value& value)
        {
            if (value.IsInt())
                return value.GetInt();
            if (value.IsNumber())
                return static_cast<int>(value.GetDouble());
            return 0;
        }

        double ReadDouble(const rapidjson::Value& value)
        {
            if (value.IsNumber())
                return value.GetDouble();
            return 0.0;
        }

        bool ReadBool(const rapidjson::Value& value)
        {
            return value.IsBool() && value.GetBool();
        }

        rapidjson::Document ParseDocument(std::string_view json)
        {
            rapidjson::Document document;
            document.Parse(json.data(), json.size());
            if (document.HasParseError())
                throw ApiException("Failed to parse SnoreSaber API response", 0, std::string(json));
            return document;
        }

        std::string PivotValue(Pivot value)
        {
            return value == Pivot::Friends ? "friends" : "player";
        }

        std::string PlayerScopeValue(PlayerScope value)
        {
            return value == PlayerScope::Region ? "region" : "country";
        }

        void AddQuery(std::string& url, const std::string& name, const std::string& value)
        {
            url += url.find('?') == std::string::npos ? "?" : "&";
            url += name + "=" + value;
        }

        void AddQuery(std::string& url, const std::string& name, int value)
        {
            AddQuery(url, name, std::to_string(value));
        }

        std::string TrimBaseUrl(std::string baseUrl)
        {
            while (!baseUrl.empty() && baseUrl.back() == '/')
                baseUrl.pop_back();
            return baseUrl;
        }

        void EnsureSuccess(long statusCode, const std::string& response, std::initializer_list<long> expected)
        {
            for (long status : expected) {
                if (statusCode == status)
                    return;
            }
            throw ApiException("Unexpected SnoreSaber API response", statusCode, response);
        }

        std::vector<std::string> HeaderStrings(const RequestHeaders& headers)
        {
            std::vector<std::string> values;
            if (headers.sessionId.has_value())
                values.push_back("x-session-id: " + headers.sessionId.value());
            if (headers.sessionKey.has_value())
                values.push_back("x-session-key: " + headers.sessionKey.value());
            return values;
        }

        template <typename T>
        T GetJson(const std::string& url, const RequestHeaders& headers = {})
        {
            auto [statusCode, response] = WebUtils::GetSync(url, TimeoutSeconds, HeaderStrings(headers));
            EnsureSuccess(statusCode, response, {200});
            return T::FromJson(response);
        }

        template <typename T>
        std::vector<T> GetJsonArray(const std::string& url, const RequestHeaders& headers = {})
        {
            auto [statusCode, response] = WebUtils::GetSync(url, TimeoutSeconds, HeaderStrings(headers));
            EnsureSuccess(statusCode, response, {200});
            rapidjson::Document document = ParseDocument(response);
            std::vector<T> values;
            if (!document.IsArray())
                return values;

            for (auto& item : document.GetArray()) {
                T value;
                value.Parse(item);
                values.push_back(std::move(value));
            }
            return values;
        }
    }

    ApiException::ApiException(std::string message, long statusCode, std::string response)
        : message(std::move(message)), statusCode(statusCode), response(std::move(response))
    {
    }

    const char* ApiException::what() const noexcept
    {
        return message.c_str();
    }

    GameAuthenticateResponse GameAuthenticateResponse::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        GameAuthenticateResponse value;
        value.Parse(document);
        return value;
    }

    void GameAuthenticateResponse::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "key")) {
            Key = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "sessionId")) {
            SessionId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "uploadProtocolVersion")) {
            UploadProtocolVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "clientTrust")) {
            ClientTrust = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "buildId")) {
            BuildId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "uploadVersionHash")) {
            UploadVersionHash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "questKey")) {
            QuestKey = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerId")) {
            PlayerId = ReadString(*member);
        }
    }

    GameAuthenticateRequest GameAuthenticateRequest::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        GameAuthenticateRequest value;
        value.Parse(document);
        return value;
    }

    void GameAuthenticateRequest::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "at")) {
            At = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerId")) {
            PlayerId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "nonce")) {
            Nonce = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "friends")) {
            Friends = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "clientBuildId")) {
            ClientBuildId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "uploadProtocolVersion")) {
            UploadProtocolVersion = ReadInt(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "pluginVersion")) {
            PluginVersion = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "gameVersion")) {
            GameVersion = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "uploadVersionHash")) {
            UploadVersionHash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "clientKind")) {
            ClientKind = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "clientProof")) {
            ClientProof = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "devUploadToken")) {
            DevUploadToken = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "artifactSha256")) {
            ArtifactSha256 = ReadString(*member);
        }
    }

    GameOfficialBuildResponseBuild GameOfficialBuildResponseBuild::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        GameOfficialBuildResponseBuild value;
        value.Parse(document);
        return value;
    }

    void GameOfficialBuildResponseBuild::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "buildId")) {
            BuildId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "pluginVersion")) {
            PluginVersion = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "gameVersion")) {
            GameVersion = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "uploadVersionHash")) {
            UploadVersionHash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "protocolVersion")) {
            ProtocolVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "status")) {
            Status = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "commitSha")) {
            if (!member->IsNull()) {
                std::string parsedCommitSha{};
                parsedCommitSha = ReadString(*member);
                CommitSha = std::move(parsedCommitSha);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "sourceRepository")) {
            if (!member->IsNull()) {
                std::string parsedSourceRepository{};
                parsedSourceRepository = ReadString(*member);
                SourceRepository = std::move(parsedSourceRepository);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "sourceRef")) {
            if (!member->IsNull()) {
                std::string parsedSourceRef{};
                parsedSourceRef = ReadString(*member);
                SourceRef = std::move(parsedSourceRef);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "sourceWorkflowRef")) {
            if (!member->IsNull()) {
                std::string parsedSourceWorkflowRef{};
                parsedSourceWorkflowRef = ReadString(*member);
                SourceWorkflowRef = std::move(parsedSourceWorkflowRef);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "sourceEnvironment")) {
            if (!member->IsNull()) {
                std::string parsedSourceEnvironment{};
                parsedSourceEnvironment = ReadString(*member);
                SourceEnvironment = std::move(parsedSourceEnvironment);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "artifactSha256")) {
            if (!member->IsNull()) {
                std::string parsedArtifactSha256{};
                parsedArtifactSha256 = ReadString(*member);
                ArtifactSha256 = std::move(parsedArtifactSha256);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "artifactUrl")) {
            if (!member->IsNull()) {
                std::string parsedArtifactUrl{};
                parsedArtifactUrl = ReadString(*member);
                ArtifactUrl = std::move(parsedArtifactUrl);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "updatedAt")) {
            UpdatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "revokedAt")) {
            if (!member->IsNull()) {
                std::string parsedRevokedAt{};
                parsedRevokedAt = ReadString(*member);
                RevokedAt = std::move(parsedRevokedAt);
            }
        }
    }

    GameOfficialBuildResponse GameOfficialBuildResponse::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        GameOfficialBuildResponse value;
        value.Parse(document);
        return value;
    }

    void GameOfficialBuildResponse::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "build")) {
            if (member->IsObject())
                Build.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "credential")) {
            Credential = ReadString(*member);
        }
    }

    GameOfficialBuildRequestSupportedVersionsItem GameOfficialBuildRequestSupportedVersionsItem::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        GameOfficialBuildRequestSupportedVersionsItem value;
        value.Parse(document);
        return value;
    }

    void GameOfficialBuildRequestSupportedVersionsItem::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "gameVersion")) {
            GameVersion = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "uploadVersionHash")) {
            UploadVersionHash = ReadString(*member);
        }
    }

    GameOfficialBuildRequest GameOfficialBuildRequest::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        GameOfficialBuildRequest value;
        value.Parse(document);
        return value;
    }

    void GameOfficialBuildRequest::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "buildId")) {
            BuildId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "pluginVersion")) {
            PluginVersion = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "gameVersion")) {
            GameVersion = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "uploadVersionHash")) {
            UploadVersionHash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "supportedVersions")) {
            if (member->IsArray()) {
                SupportedVersions.clear();
                for (auto& item : member->GetArray()) {
                    GameOfficialBuildRequestSupportedVersionsItem parsedItem{};
                    if (item.IsObject())
                        parsedItem.Parse(item);
                    SupportedVersions.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "protocolVersion")) {
            ProtocolVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "commitSha")) {
            CommitSha = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "sourceRepository")) {
            SourceRepository = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "sourceRef")) {
            SourceRef = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "sourceWorkflowRef")) {
            SourceWorkflowRef = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "sourceEnvironment")) {
            if (!member->IsNull()) {
                std::string parsedSourceEnvironment{};
                parsedSourceEnvironment = ReadString(*member);
                SourceEnvironment = std::move(parsedSourceEnvironment);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "artifactSha256")) {
            if (!member->IsNull()) {
                std::string parsedArtifactSha256{};
                parsedArtifactSha256 = ReadString(*member);
                ArtifactSha256 = std::move(parsedArtifactSha256);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "artifactUrl")) {
            if (!member->IsNull()) {
                std::string parsedArtifactUrl{};
                parsedArtifactUrl = ReadString(*member);
                ArtifactUrl = std::move(parsedArtifactUrl);
            }
        }
    }

    GameUploadResponse GameUploadResponse::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        GameUploadResponse value;
        value.Parse(document);
        return value;
    }

    void GameUploadResponse::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "success")) {
            Success = ReadBool(*member);
        }
    }

    GameUploadRequest GameUploadRequest::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        GameUploadRequest value;
        value.Parse(document);
        return value;
    }

    void GameUploadRequest::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "data")) {
            Data = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "zr")) {
            Zr = ReadString(*member);
        }
    }

    LeaderboardResponseMap LeaderboardResponseMap::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardResponseMap value;
        value.Parse(document);
        return value;
    }

    void LeaderboardResponseMap::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hash")) {
            Hash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bsid")) {
            if (!member->IsNull()) {
                std::string parsedBsid{};
                parsedBsid = ReadString(*member);
                Bsid = std::move(parsedBsid);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "songName")) {
            SongName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songSubName")) {
            SongSubName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songAuthorName")) {
            SongAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "levelAuthorName")) {
            LevelAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bpm")) {
            Bpm = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "coverUrl")) {
            CoverUrl = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "verified")) {
            Verified = ReadBool(*member);
        }
    }

    LeaderboardResponseDifficulty LeaderboardResponseDifficulty::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardResponseDifficulty value;
        value.Parse(document);
        return value;
    }

    void LeaderboardResponseDifficulty::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "difficulty")) {
            Difficulty = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rawDifficulty")) {
            RawDifficulty = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "gameMode")) {
            GameMode = ReadString(*member);
        }
    }

    LeaderboardResponseRealm LeaderboardResponseRealm::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardResponseRealm value;
        value.Parse(document);
        return value;
    }

    void LeaderboardResponseRealm::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "realmId")) {
            RealmId = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "realmName")) {
            RealmName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "leaderboardStatus")) {
            LeaderboardStatus = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "positiveModifiers")) {
            PositiveModifiers = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "stars")) {
            Stars = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rankedAt")) {
            if (!member->IsNull()) {
                std::string parsedRankedAt{};
                parsedRankedAt = ReadString(*member);
                RankedAt = std::move(parsedRankedAt);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "qualifiedAt")) {
            if (!member->IsNull()) {
                std::string parsedQualifiedAt{};
                parsedQualifiedAt = ReadString(*member);
                QualifiedAt = std::move(parsedQualifiedAt);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "lovedAt")) {
            if (!member->IsNull()) {
                std::string parsedLovedAt{};
                parsedLovedAt = ReadString(*member);
                LovedAt = std::move(parsedLovedAt);
            }
        }
    }

    LeaderboardResponse LeaderboardResponse::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardResponse value;
        value.Parse(document);
        return value;
    }

    void LeaderboardResponse::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "map")) {
            if (member->IsObject())
                Map.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "difficulty")) {
            if (member->IsObject())
                Difficulty.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "maxScore")) {
            MaxScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalScores")) {
            TotalScores = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "dailyScores")) {
            DailyScores = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "realm")) {
            if (member->IsObject())
                Realm.Parse(*member);
        }
    }

    LeaderboardScoresResponseDataItemPlayer LeaderboardScoresResponseDataItemPlayer::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardScoresResponseDataItemPlayer value;
        value.Parse(document);
        return value;
    }

    void LeaderboardScoresResponseDataItemPlayer::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerNameInGame")) {
            PlayerNameInGame = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "country")) {
            Country = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "role")) {
            if (!member->IsNull()) {
                std::string parsedRole{};
                parsedRole = ReadString(*member);
                Role = std::move(parsedRole);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "avatar")) {
            Avatar = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "avatarVersion")) {
            AvatarVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "permissions")) {
            Permissions = ReadDouble(*member);
        }
    }

    LeaderboardScoresResponseDataItemDevice LeaderboardScoresResponseDataItemDevice::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardScoresResponseDataItemDevice value;
        value.Parse(document);
        return value;
    }

    void LeaderboardScoresResponseDataItemDevice::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "hmd")) {
            if (!member->IsNull()) {
                std::string parsedHMD{};
                parsedHMD = ReadString(*member);
                HMD = std::move(parsedHMD);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerLeft")) {
            if (!member->IsNull()) {
                std::string parsedControllerLeft{};
                parsedControllerLeft = ReadString(*member);
                ControllerLeft = std::move(parsedControllerLeft);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerRight")) {
            if (!member->IsNull()) {
                std::string parsedControllerRight{};
                parsedControllerRight = ReadString(*member);
                ControllerRight = std::move(parsedControllerRight);
            }
        }
    }

    LeaderboardScoresResponseDataItem LeaderboardScoresResponseDataItem::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardScoresResponseDataItem value;
        value.Parse(document);
        return value;
    }

    void LeaderboardScoresResponseDataItem::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rank")) {
            Rank = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "unmodifiedScore")) {
            UnmodifiedScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "modifiedScore")) {
            ModifiedScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "accuracy")) {
            Accuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "pp")) {
            PP = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "weight")) {
            Weight = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "mods")) {
            if (member->IsArray()) {
                Mods.clear();
                for (auto& item : member->GetArray()) {
                    std::string parsedItem{};
                    parsedItem = ReadString(item);
                    Mods.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "badCuts")) {
            BadCuts = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "missedNotes")) {
            MissedNotes = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "maxCombo")) {
            MaxCombo = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "fullCombo")) {
            FullCombo = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hasReplay")) {
            HasReplay = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "personalBest")) {
            PersonalBest = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "legacyHmdId")) {
            if (!member->IsNull()) {
                double parsedLegacyHMDId{};
                parsedLegacyHMDId = ReadDouble(*member);
                LegacyHMDId = std::move(parsedLegacyHMDId);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "version")) {
            if (!member->IsNull()) {
                std::string parsedVersion{};
                parsedVersion = ReadString(*member);
                Version = std::move(parsedVersion);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "playOutcome")) {
            PlayOutcome = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playOutcomeTime")) {
            if (!member->IsNull()) {
                double parsedPlayOutcomeTime{};
                parsedPlayOutcomeTime = ReadDouble(*member);
                PlayOutcomeTime = std::move(parsedPlayOutcomeTime);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "player")) {
            if (member->IsObject())
                Player.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "device")) {
            if (member->IsObject())
                Device.Parse(*member);
        }
    }

    LeaderboardScoresResponseMetadata LeaderboardScoresResponseMetadata::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardScoresResponseMetadata value;
        value.Parse(document);
        return value;
    }

    void LeaderboardScoresResponseMetadata::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "page")) {
            Page = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "itemsPerPage")) {
            ItemsPerPage = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalItems")) {
            TotalItems = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPages")) {
            TotalPages = ReadDouble(*member);
        }
    }

    LeaderboardScoresResponsePlayerScorePlayer LeaderboardScoresResponsePlayerScorePlayer::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardScoresResponsePlayerScorePlayer value;
        value.Parse(document);
        return value;
    }

    void LeaderboardScoresResponsePlayerScorePlayer::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerNameInGame")) {
            PlayerNameInGame = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "country")) {
            Country = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "role")) {
            if (!member->IsNull()) {
                std::string parsedRole{};
                parsedRole = ReadString(*member);
                Role = std::move(parsedRole);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "avatar")) {
            Avatar = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "avatarVersion")) {
            AvatarVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "permissions")) {
            Permissions = ReadDouble(*member);
        }
    }

    LeaderboardScoresResponsePlayerScoreDevice LeaderboardScoresResponsePlayerScoreDevice::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardScoresResponsePlayerScoreDevice value;
        value.Parse(document);
        return value;
    }

    void LeaderboardScoresResponsePlayerScoreDevice::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "hmd")) {
            if (!member->IsNull()) {
                std::string parsedHMD{};
                parsedHMD = ReadString(*member);
                HMD = std::move(parsedHMD);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerLeft")) {
            if (!member->IsNull()) {
                std::string parsedControllerLeft{};
                parsedControllerLeft = ReadString(*member);
                ControllerLeft = std::move(parsedControllerLeft);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerRight")) {
            if (!member->IsNull()) {
                std::string parsedControllerRight{};
                parsedControllerRight = ReadString(*member);
                ControllerRight = std::move(parsedControllerRight);
            }
        }
    }

    LeaderboardScoresResponsePlayerScore LeaderboardScoresResponsePlayerScore::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardScoresResponsePlayerScore value;
        value.Parse(document);
        return value;
    }

    void LeaderboardScoresResponsePlayerScore::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rank")) {
            Rank = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "unmodifiedScore")) {
            UnmodifiedScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "modifiedScore")) {
            ModifiedScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "accuracy")) {
            Accuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "pp")) {
            PP = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "weight")) {
            Weight = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "mods")) {
            if (member->IsArray()) {
                Mods.clear();
                for (auto& item : member->GetArray()) {
                    std::string parsedItem{};
                    parsedItem = ReadString(item);
                    Mods.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "badCuts")) {
            BadCuts = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "missedNotes")) {
            MissedNotes = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "maxCombo")) {
            MaxCombo = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "fullCombo")) {
            FullCombo = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hasReplay")) {
            HasReplay = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "personalBest")) {
            PersonalBest = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "legacyHmdId")) {
            if (!member->IsNull()) {
                double parsedLegacyHMDId{};
                parsedLegacyHMDId = ReadDouble(*member);
                LegacyHMDId = std::move(parsedLegacyHMDId);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "version")) {
            if (!member->IsNull()) {
                std::string parsedVersion{};
                parsedVersion = ReadString(*member);
                Version = std::move(parsedVersion);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "playOutcome")) {
            PlayOutcome = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playOutcomeTime")) {
            if (!member->IsNull()) {
                double parsedPlayOutcomeTime{};
                parsedPlayOutcomeTime = ReadDouble(*member);
                PlayOutcomeTime = std::move(parsedPlayOutcomeTime);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "player")) {
            if (member->IsObject())
                Player.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "device")) {
            if (member->IsObject())
                Device.Parse(*member);
        }
    }

    LeaderboardScoresResponse LeaderboardScoresResponse::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LeaderboardScoresResponse value;
        value.Parse(document);
        return value;
    }

    void LeaderboardScoresResponse::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "data")) {
            if (member->IsArray()) {
                Data.clear();
                for (auto& item : member->GetArray()) {
                    LeaderboardScoresResponseDataItem parsedItem{};
                    if (item.IsObject())
                        parsedItem.Parse(item);
                    Data.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "metadata")) {
            if (member->IsObject())
                Metadata.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerScore")) {
            if (member->IsObject())
                PlayerScore.Parse(*member);
        }
    }

    MapDetailsResponseLeaderboardsItemRealm MapDetailsResponseLeaderboardsItemRealm::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseLeaderboardsItemRealm value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseLeaderboardsItemRealm::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "realmId")) {
            RealmId = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "realmName")) {
            RealmName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "leaderboardStatus")) {
            LeaderboardStatus = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "positiveModifiers")) {
            PositiveModifiers = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "stars")) {
            Stars = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rankedAt")) {
            if (!member->IsNull()) {
                std::string parsedRankedAt{};
                parsedRankedAt = ReadString(*member);
                RankedAt = std::move(parsedRankedAt);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "qualifiedAt")) {
            if (!member->IsNull()) {
                std::string parsedQualifiedAt{};
                parsedQualifiedAt = ReadString(*member);
                QualifiedAt = std::move(parsedQualifiedAt);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "lovedAt")) {
            if (!member->IsNull()) {
                std::string parsedLovedAt{};
                parsedLovedAt = ReadString(*member);
                LovedAt = std::move(parsedLovedAt);
            }
        }
    }

    MapDetailsResponseLeaderboardsItem MapDetailsResponseLeaderboardsItem::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseLeaderboardsItem value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseLeaderboardsItem::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "difficulty")) {
            Difficulty = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "gameMode")) {
            GameMode = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rawDifficulty")) {
            RawDifficulty = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "maxScore")) {
            MaxScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalScores")) {
            TotalScores = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "dailyScores")) {
            DailyScores = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "realm")) {
            if (member->IsObject())
                Realm.Parse(*member);
        }
    }

    MapDetailsResponseRankRequestReplacedByMap MapDetailsResponseRankRequestReplacedByMap::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestReplacedByMap value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestReplacedByMap::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hash")) {
            Hash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bsid")) {
            if (!member->IsNull()) {
                std::string parsedBsid{};
                parsedBsid = ReadString(*member);
                Bsid = std::move(parsedBsid);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "songName")) {
            SongName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songSubName")) {
            SongSubName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songAuthorName")) {
            SongAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "levelAuthorName")) {
            LevelAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bpm")) {
            Bpm = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "coverUrl")) {
            CoverUrl = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "verified")) {
            Verified = ReadBool(*member);
        }
    }

    MapDetailsResponseRankRequestReplacedBy MapDetailsResponseRankRequestReplacedBy::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestReplacedBy value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestReplacedBy::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "map")) {
            if (member->IsObject())
                Map.Parse(*member);
        }
    }

    MapDetailsResponseRankRequestReplacedFromMap MapDetailsResponseRankRequestReplacedFromMap::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestReplacedFromMap value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestReplacedFromMap::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hash")) {
            Hash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bsid")) {
            if (!member->IsNull()) {
                std::string parsedBsid{};
                parsedBsid = ReadString(*member);
                Bsid = std::move(parsedBsid);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "songName")) {
            SongName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songSubName")) {
            SongSubName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songAuthorName")) {
            SongAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "levelAuthorName")) {
            LevelAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bpm")) {
            Bpm = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "coverUrl")) {
            CoverUrl = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "verified")) {
            Verified = ReadBool(*member);
        }
    }

    MapDetailsResponseRankRequestReplacedFrom MapDetailsResponseRankRequestReplacedFrom::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestReplacedFrom value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestReplacedFrom::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "map")) {
            if (member->IsObject())
                Map.Parse(*member);
        }
    }

    MapDetailsResponseRankRequestDifficultiesItemLeaderboardMap MapDetailsResponseRankRequestDifficultiesItemLeaderboardMap::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestDifficultiesItemLeaderboardMap value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestDifficultiesItemLeaderboardMap::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hash")) {
            Hash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bsid")) {
            if (!member->IsNull()) {
                std::string parsedBsid{};
                parsedBsid = ReadString(*member);
                Bsid = std::move(parsedBsid);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "songName")) {
            SongName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songSubName")) {
            SongSubName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songAuthorName")) {
            SongAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "levelAuthorName")) {
            LevelAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bpm")) {
            Bpm = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "coverUrl")) {
            CoverUrl = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "verified")) {
            Verified = ReadBool(*member);
        }
    }

    MapDetailsResponseRankRequestDifficultiesItemLeaderboardDifficulty MapDetailsResponseRankRequestDifficultiesItemLeaderboardDifficulty::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestDifficultiesItemLeaderboardDifficulty value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestDifficultiesItemLeaderboardDifficulty::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "difficulty")) {
            Difficulty = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rawDifficulty")) {
            RawDifficulty = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "gameMode")) {
            GameMode = ReadString(*member);
        }
    }

    MapDetailsResponseRankRequestDifficultiesItemLeaderboardRealm MapDetailsResponseRankRequestDifficultiesItemLeaderboardRealm::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestDifficultiesItemLeaderboardRealm value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestDifficultiesItemLeaderboardRealm::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "realmId")) {
            RealmId = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "realmName")) {
            RealmName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "leaderboardStatus")) {
            LeaderboardStatus = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "positiveModifiers")) {
            PositiveModifiers = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "stars")) {
            Stars = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rankedAt")) {
            if (!member->IsNull()) {
                std::string parsedRankedAt{};
                parsedRankedAt = ReadString(*member);
                RankedAt = std::move(parsedRankedAt);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "qualifiedAt")) {
            if (!member->IsNull()) {
                std::string parsedQualifiedAt{};
                parsedQualifiedAt = ReadString(*member);
                QualifiedAt = std::move(parsedQualifiedAt);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "lovedAt")) {
            if (!member->IsNull()) {
                std::string parsedLovedAt{};
                parsedLovedAt = ReadString(*member);
                LovedAt = std::move(parsedLovedAt);
            }
        }
    }

    MapDetailsResponseRankRequestDifficultiesItemLeaderboard MapDetailsResponseRankRequestDifficultiesItemLeaderboard::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestDifficultiesItemLeaderboard value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestDifficultiesItemLeaderboard::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "map")) {
            if (member->IsObject())
                Map.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "difficulty")) {
            if (member->IsObject())
                Difficulty.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "maxScore")) {
            MaxScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalScores")) {
            TotalScores = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "dailyScores")) {
            DailyScores = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "realm")) {
            if (member->IsObject())
                Realm.Parse(*member);
        }
    }

    MapDetailsResponseRankRequestDifficultiesItemRtVotes MapDetailsResponseRankRequestDifficultiesItemRtVotes::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestDifficultiesItemRtVotes value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestDifficultiesItemRtVotes::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "upvotes")) {
            Upvotes = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "downvotes")) {
            Downvotes = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "myVote")) {
            if (!member->IsNull()) {
                std::string parsedMyVote{};
                parsedMyVote = ReadString(*member);
                MyVote = std::move(parsedMyVote);
            }
        }
    }

    MapDetailsResponseRankRequestDifficultiesItemQatVotes MapDetailsResponseRankRequestDifficultiesItemQatVotes::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestDifficultiesItemQatVotes value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestDifficultiesItemQatVotes::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "upvotes")) {
            Upvotes = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "downvotes")) {
            Downvotes = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "neutrals")) {
            Neutrals = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "myVote")) {
            if (!member->IsNull()) {
                std::string parsedMyVote{};
                parsedMyVote = ReadString(*member);
                MyVote = std::move(parsedMyVote);
            }
        }
    }

    MapDetailsResponseRankRequestDifficultiesItemRtCommentsItemPlayer MapDetailsResponseRankRequestDifficultiesItemRtCommentsItemPlayer::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestDifficultiesItemRtCommentsItemPlayer value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestDifficultiesItemRtCommentsItemPlayer::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerNameInGame")) {
            PlayerNameInGame = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "country")) {
            Country = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "role")) {
            if (!member->IsNull()) {
                std::string parsedRole{};
                parsedRole = ReadString(*member);
                Role = std::move(parsedRole);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "avatar")) {
            Avatar = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "avatarVersion")) {
            AvatarVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "permissions")) {
            Permissions = ReadDouble(*member);
        }
    }

    MapDetailsResponseRankRequestDifficultiesItemRtCommentsItem MapDetailsResponseRankRequestDifficultiesItemRtCommentsItem::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestDifficultiesItemRtCommentsItem value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestDifficultiesItemRtCommentsItem::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "player")) {
            if (member->IsObject())
                Player.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "comment")) {
            Comment = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "edited")) {
            Edited = ReadBool(*member);
        }
    }

    MapDetailsResponseRankRequestDifficultiesItemQatCommentsItemPlayer MapDetailsResponseRankRequestDifficultiesItemQatCommentsItemPlayer::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestDifficultiesItemQatCommentsItemPlayer value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestDifficultiesItemQatCommentsItemPlayer::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerNameInGame")) {
            PlayerNameInGame = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "country")) {
            Country = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "role")) {
            if (!member->IsNull()) {
                std::string parsedRole{};
                parsedRole = ReadString(*member);
                Role = std::move(parsedRole);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "avatar")) {
            Avatar = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "avatarVersion")) {
            AvatarVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "permissions")) {
            Permissions = ReadDouble(*member);
        }
    }

    MapDetailsResponseRankRequestDifficultiesItemQatCommentsItem MapDetailsResponseRankRequestDifficultiesItemQatCommentsItem::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestDifficultiesItemQatCommentsItem value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestDifficultiesItemQatCommentsItem::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "player")) {
            if (member->IsObject())
                Player.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "comment")) {
            Comment = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "edited")) {
            Edited = ReadBool(*member);
        }
    }

    MapDetailsResponseRankRequestDifficultiesItem MapDetailsResponseRankRequestDifficultiesItem::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequestDifficultiesItem value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequestDifficultiesItem::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "description")) {
            Description = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "approvalStatus")) {
            ApprovalStatus = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "leaderboard")) {
            if (member->IsObject())
                Leaderboard.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rtVotes")) {
            if (member->IsObject())
                RtVotes.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "qatVotes")) {
            if (member->IsObject())
                QatVotes.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rtComments")) {
            if (member->IsArray()) {
                RtComments.clear();
                for (auto& item : member->GetArray()) {
                    MapDetailsResponseRankRequestDifficultiesItemRtCommentsItem parsedItem{};
                    if (item.IsObject())
                        parsedItem.Parse(item);
                    RtComments.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "qatComments")) {
            if (member->IsArray()) {
                QatComments.clear();
                for (auto& item : member->GetArray()) {
                    MapDetailsResponseRankRequestDifficultiesItemQatCommentsItem parsedItem{};
                    if (item.IsObject())
                        parsedItem.Parse(item);
                    QatComments.push_back(std::move(parsedItem));
                }
            }
        }
    }

    MapDetailsResponseRankRequest MapDetailsResponseRankRequest::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponseRankRequest value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponseRankRequest::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "description")) {
            Description = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "requestType")) {
            RequestType = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "approvalStatus")) {
            ApprovalStatus = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "replacedBy")) {
            if (member->IsObject())
                ReplacedBy.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "replacedFrom")) {
            if (member->IsObject())
                ReplacedFrom.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "weight")) {
            Weight = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "difficulties")) {
            if (member->IsArray()) {
                Difficulties.clear();
                for (auto& item : member->GetArray()) {
                    MapDetailsResponseRankRequestDifficultiesItem parsedItem{};
                    if (item.IsObject())
                        parsedItem.Parse(item);
                    Difficulties.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "commentsObfuscated")) {
            CommentsObfuscated = ReadBool(*member);
        }
    }

    MapDetailsResponse MapDetailsResponse::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        MapDetailsResponse value;
        value.Parse(document);
        return value;
    }

    void MapDetailsResponse::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hash")) {
            Hash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bsid")) {
            if (!member->IsNull()) {
                std::string parsedBsid{};
                parsedBsid = ReadString(*member);
                Bsid = std::move(parsedBsid);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "songName")) {
            SongName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songSubName")) {
            SongSubName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songAuthorName")) {
            SongAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "levelAuthorName")) {
            LevelAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bpm")) {
            Bpm = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "coverUrl")) {
            CoverUrl = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "verified")) {
            Verified = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalScores")) {
            TotalScores = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "dailyScores")) {
            DailyScores = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "leaderboards")) {
            if (member->IsArray()) {
                Leaderboards.clear();
                for (auto& item : member->GetArray()) {
                    MapDetailsResponseLeaderboardsItem parsedItem{};
                    if (item.IsObject())
                        parsedItem.Parse(item);
                    Leaderboards.push_back(std::move(parsedItem));
                }
            }
        }
    }

    PlayerListResponseDataItemStatsDevice PlayerListResponseDataItemStatsDevice::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerListResponseDataItemStatsDevice value;
        value.Parse(document);
        return value;
    }

    void PlayerListResponseDataItemStatsDevice::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "hmd")) {
            if (!member->IsNull()) {
                std::string parsedHMD{};
                parsedHMD = ReadString(*member);
                HMD = std::move(parsedHMD);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerLeft")) {
            if (!member->IsNull()) {
                std::string parsedControllerLeft{};
                parsedControllerLeft = ReadString(*member);
                ControllerLeft = std::move(parsedControllerLeft);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerRight")) {
            if (!member->IsNull()) {
                std::string parsedControllerRight{};
                parsedControllerRight = ReadString(*member);
                ControllerRight = std::move(parsedControllerRight);
            }
        }
    }

    PlayerListResponseDataItemStats PlayerListResponseDataItemStats::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerListResponseDataItemStats value;
        value.Parse(document);
        return value;
    }

    void PlayerListResponseDataItemStats::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "realmId")) {
            RealmId = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "realmName")) {
            RealmName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rank")) {
            Rank = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "countryRank")) {
            CountryRank = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPP")) {
            TotalPP = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalScore")) {
            TotalScore = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalRankedScore")) {
            TotalRankedScore = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPlayedLeaderboards")) {
            TotalPlayedLeaderboards = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPlayedRankedLeaderboards")) {
            TotalPlayedRankedLeaderboards = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalSubmittedPlays")) {
            TotalSubmittedPlays = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalReplayViews")) {
            TotalReplayViews = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "averageAccuracy")) {
            AverageAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "weightedAverageAccuracy")) {
            WeightedAverageAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "completionAccuracy")) {
            CompletionAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "device")) {
            if (member->IsObject())
                Device.Parse(*member);
        }
    }

    PlayerListResponseDataItem PlayerListResponseDataItem::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerListResponseDataItem value;
        value.Parse(document);
        return value;
    }

    void PlayerListResponseDataItem::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerNameInGame")) {
            PlayerNameInGame = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "country")) {
            Country = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "role")) {
            if (!member->IsNull()) {
                std::string parsedRole{};
                parsedRole = ReadString(*member);
                Role = std::move(parsedRole);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "avatar")) {
            Avatar = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "avatarVersion")) {
            AvatarVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "permissions")) {
            Permissions = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "banned")) {
            Banned = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "silenced")) {
            Silenced = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "inactive")) {
            Inactive = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "stats")) {
            if (member->IsObject())
                Stats.Parse(*member);
        }
    }

    PlayerListResponseMetadata PlayerListResponseMetadata::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerListResponseMetadata value;
        value.Parse(document);
        return value;
    }

    void PlayerListResponseMetadata::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "page")) {
            Page = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "itemsPerPage")) {
            ItemsPerPage = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalItems")) {
            TotalItems = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPages")) {
            TotalPages = ReadDouble(*member);
        }
    }

    PlayerListResponse PlayerListResponse::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerListResponse value;
        value.Parse(document);
        return value;
    }

    void PlayerListResponse::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "data")) {
            if (member->IsArray()) {
                Data.clear();
                for (auto& item : member->GetArray()) {
                    PlayerListResponseDataItem parsedItem{};
                    if (item.IsObject())
                        parsedItem.Parse(item);
                    Data.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "metadata")) {
            if (member->IsObject())
                Metadata.Parse(*member);
        }
    }

    PlayerProfileResponseStatsDevice PlayerProfileResponseStatsDevice::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponseStatsDevice value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponseStatsDevice::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "hmd")) {
            if (!member->IsNull()) {
                std::string parsedHMD{};
                parsedHMD = ReadString(*member);
                HMD = std::move(parsedHMD);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerLeft")) {
            if (!member->IsNull()) {
                std::string parsedControllerLeft{};
                parsedControllerLeft = ReadString(*member);
                ControllerLeft = std::move(parsedControllerLeft);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerRight")) {
            if (!member->IsNull()) {
                std::string parsedControllerRight{};
                parsedControllerRight = ReadString(*member);
                ControllerRight = std::move(parsedControllerRight);
            }
        }
    }

    PlayerProfileResponseStats PlayerProfileResponseStats::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponseStats value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponseStats::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "realmId")) {
            RealmId = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "realmName")) {
            RealmName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rank")) {
            Rank = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "countryRank")) {
            CountryRank = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPP")) {
            TotalPP = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalScore")) {
            TotalScore = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalRankedScore")) {
            TotalRankedScore = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPlayedLeaderboards")) {
            TotalPlayedLeaderboards = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPlayedRankedLeaderboards")) {
            TotalPlayedRankedLeaderboards = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalSubmittedPlays")) {
            TotalSubmittedPlays = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalReplayViews")) {
            TotalReplayViews = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "averageAccuracy")) {
            AverageAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "weightedAverageAccuracy")) {
            WeightedAverageAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "completionAccuracy")) {
            CompletionAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "device")) {
            if (member->IsObject())
                Device.Parse(*member);
        }
    }

    PlayerProfileResponseProfileCustomization PlayerProfileResponseProfileCustomization::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponseProfileCustomization value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponseProfileCustomization::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "accentColor")) {
            if (!member->IsNull()) {
                std::string parsedAccentColor{};
                parsedAccentColor = ReadString(*member);
                AccentColor = std::move(parsedAccentColor);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "accentForegroundColor")) {
            if (!member->IsNull()) {
                std::string parsedAccentForegroundColor{};
                parsedAccentForegroundColor = ReadString(*member);
                AccentForegroundColor = std::move(parsedAccentForegroundColor);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "supporterNameColorEnabled")) {
            SupporterNameColorEnabled = ReadBool(*member);
        }
    }

    PlayerProfileResponseBadgesItem PlayerProfileResponseBadgesItem::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponseBadgesItem value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponseBadgesItem::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "image")) {
            Image = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "description")) {
            Description = ReadString(*member);
        }
    }

    PlayerProfileResponsePinnedScoresItemScoreScorePlayer PlayerProfileResponsePinnedScoresItemScoreScorePlayer::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponsePinnedScoresItemScoreScorePlayer value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponsePinnedScoresItemScoreScorePlayer::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerNameInGame")) {
            PlayerNameInGame = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "country")) {
            Country = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "role")) {
            if (!member->IsNull()) {
                std::string parsedRole{};
                parsedRole = ReadString(*member);
                Role = std::move(parsedRole);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "avatar")) {
            Avatar = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "avatarVersion")) {
            AvatarVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "permissions")) {
            Permissions = ReadDouble(*member);
        }
    }

    PlayerProfileResponsePinnedScoresItemScoreScoreDevice PlayerProfileResponsePinnedScoresItemScoreScoreDevice::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponsePinnedScoresItemScoreScoreDevice value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponsePinnedScoresItemScoreScoreDevice::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "hmd")) {
            if (!member->IsNull()) {
                std::string parsedHMD{};
                parsedHMD = ReadString(*member);
                HMD = std::move(parsedHMD);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerLeft")) {
            if (!member->IsNull()) {
                std::string parsedControllerLeft{};
                parsedControllerLeft = ReadString(*member);
                ControllerLeft = std::move(parsedControllerLeft);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerRight")) {
            if (!member->IsNull()) {
                std::string parsedControllerRight{};
                parsedControllerRight = ReadString(*member);
                ControllerRight = std::move(parsedControllerRight);
            }
        }
    }

    PlayerProfileResponsePinnedScoresItemScoreScore PlayerProfileResponsePinnedScoresItemScoreScore::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponsePinnedScoresItemScoreScore value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponsePinnedScoresItemScoreScore::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rank")) {
            Rank = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "unmodifiedScore")) {
            UnmodifiedScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "modifiedScore")) {
            ModifiedScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "accuracy")) {
            Accuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "pp")) {
            PP = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "weight")) {
            Weight = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "mods")) {
            if (member->IsArray()) {
                Mods.clear();
                for (auto& item : member->GetArray()) {
                    std::string parsedItem{};
                    parsedItem = ReadString(item);
                    Mods.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "badCuts")) {
            BadCuts = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "missedNotes")) {
            MissedNotes = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "maxCombo")) {
            MaxCombo = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "fullCombo")) {
            FullCombo = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hasReplay")) {
            HasReplay = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "personalBest")) {
            PersonalBest = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "legacyHmdId")) {
            if (!member->IsNull()) {
                double parsedLegacyHMDId{};
                parsedLegacyHMDId = ReadDouble(*member);
                LegacyHMDId = std::move(parsedLegacyHMDId);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "version")) {
            if (!member->IsNull()) {
                std::string parsedVersion{};
                parsedVersion = ReadString(*member);
                Version = std::move(parsedVersion);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "playOutcome")) {
            PlayOutcome = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playOutcomeTime")) {
            if (!member->IsNull()) {
                double parsedPlayOutcomeTime{};
                parsedPlayOutcomeTime = ReadDouble(*member);
                PlayOutcomeTime = std::move(parsedPlayOutcomeTime);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "player")) {
            if (member->IsObject())
                Player.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "device")) {
            if (member->IsObject())
                Device.Parse(*member);
        }
    }

    PlayerProfileResponsePinnedScoresItemScoreLeaderboardMap PlayerProfileResponsePinnedScoresItemScoreLeaderboardMap::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponsePinnedScoresItemScoreLeaderboardMap value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponsePinnedScoresItemScoreLeaderboardMap::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hash")) {
            Hash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bsid")) {
            if (!member->IsNull()) {
                std::string parsedBsid{};
                parsedBsid = ReadString(*member);
                Bsid = std::move(parsedBsid);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "songName")) {
            SongName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songSubName")) {
            SongSubName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songAuthorName")) {
            SongAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "levelAuthorName")) {
            LevelAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bpm")) {
            Bpm = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "coverUrl")) {
            CoverUrl = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "verified")) {
            Verified = ReadBool(*member);
        }
    }

    PlayerProfileResponsePinnedScoresItemScoreLeaderboardDifficulty PlayerProfileResponsePinnedScoresItemScoreLeaderboardDifficulty::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponsePinnedScoresItemScoreLeaderboardDifficulty value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponsePinnedScoresItemScoreLeaderboardDifficulty::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "difficulty")) {
            Difficulty = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rawDifficulty")) {
            RawDifficulty = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "gameMode")) {
            GameMode = ReadString(*member);
        }
    }

    PlayerProfileResponsePinnedScoresItemScoreLeaderboardRealm PlayerProfileResponsePinnedScoresItemScoreLeaderboardRealm::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponsePinnedScoresItemScoreLeaderboardRealm value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponsePinnedScoresItemScoreLeaderboardRealm::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "realmId")) {
            RealmId = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "realmName")) {
            RealmName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "leaderboardStatus")) {
            LeaderboardStatus = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "positiveModifiers")) {
            PositiveModifiers = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "stars")) {
            Stars = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rankedAt")) {
            if (!member->IsNull()) {
                std::string parsedRankedAt{};
                parsedRankedAt = ReadString(*member);
                RankedAt = std::move(parsedRankedAt);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "qualifiedAt")) {
            if (!member->IsNull()) {
                std::string parsedQualifiedAt{};
                parsedQualifiedAt = ReadString(*member);
                QualifiedAt = std::move(parsedQualifiedAt);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "lovedAt")) {
            if (!member->IsNull()) {
                std::string parsedLovedAt{};
                parsedLovedAt = ReadString(*member);
                LovedAt = std::move(parsedLovedAt);
            }
        }
    }

    PlayerProfileResponsePinnedScoresItemScoreLeaderboard PlayerProfileResponsePinnedScoresItemScoreLeaderboard::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponsePinnedScoresItemScoreLeaderboard value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponsePinnedScoresItemScoreLeaderboard::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "map")) {
            if (member->IsObject())
                Map.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "difficulty")) {
            if (member->IsObject())
                Difficulty.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "maxScore")) {
            MaxScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalScores")) {
            TotalScores = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "dailyScores")) {
            DailyScores = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "realm")) {
            if (member->IsObject())
                Realm.Parse(*member);
        }
    }

    PlayerProfileResponsePinnedScoresItemScore PlayerProfileResponsePinnedScoresItemScore::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponsePinnedScoresItemScore value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponsePinnedScoresItemScore::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "score")) {
            if (member->IsObject())
                Score.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "leaderboard")) {
            if (member->IsObject())
                Leaderboard.Parse(*member);
        }
    }

    PlayerProfileResponsePinnedScoresItem PlayerProfileResponsePinnedScoresItem::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponsePinnedScoresItem value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponsePinnedScoresItem::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "score")) {
            if (member->IsObject())
                Score.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "comment")) {
            Comment = ReadString(*member);
        }
    }

    PlayerProfileResponse PlayerProfileResponse::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerProfileResponse value;
        value.Parse(document);
        return value;
    }

    void PlayerProfileResponse::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerNameInGame")) {
            PlayerNameInGame = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "country")) {
            Country = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "role")) {
            if (!member->IsNull()) {
                std::string parsedRole{};
                parsedRole = ReadString(*member);
                Role = std::move(parsedRole);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "avatar")) {
            Avatar = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "avatarVersion")) {
            AvatarVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "permissions")) {
            Permissions = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "banned")) {
            Banned = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "silenced")) {
            Silenced = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "inactive")) {
            Inactive = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "stats")) {
            if (member->IsObject())
                Stats.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bio")) {
            if (!member->IsNull()) {
                std::string parsedBio{};
                parsedBio = ReadString(*member);
                Bio = std::move(parsedBio);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "vanity")) {
            if (!member->IsNull()) {
                std::string parsedVanity{};
                parsedVanity = ReadString(*member);
                Vanity = std::move(parsedVanity);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "profileCustomization")) {
            if (member->IsObject())
                ProfileCustomization.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "lastSeenAt")) {
            LastSeenAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "badges")) {
            if (member->IsArray()) {
                Badges.clear();
                for (auto& item : member->GetArray()) {
                    PlayerProfileResponseBadgesItem parsedItem{};
                    if (item.IsObject())
                        parsedItem.Parse(item);
                    Badges.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "pinnedScores")) {
            if (member->IsArray()) {
                PinnedScores.clear();
                for (auto& item : member->GetArray()) {
                    PlayerProfileResponsePinnedScoresItem parsedItem{};
                    if (item.IsObject())
                        parsedItem.Parse(item);
                    PinnedScores.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "followers")) {
            Followers = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "following")) {
            Following = ReadDouble(*member);
        }
    }

    PlayerBasicProfileResponseStatsDevice PlayerBasicProfileResponseStatsDevice::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerBasicProfileResponseStatsDevice value;
        value.Parse(document);
        return value;
    }

    void PlayerBasicProfileResponseStatsDevice::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "hmd")) {
            if (!member->IsNull()) {
                std::string parsedHMD{};
                parsedHMD = ReadString(*member);
                HMD = std::move(parsedHMD);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerLeft")) {
            if (!member->IsNull()) {
                std::string parsedControllerLeft{};
                parsedControllerLeft = ReadString(*member);
                ControllerLeft = std::move(parsedControllerLeft);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "controllerRight")) {
            if (!member->IsNull()) {
                std::string parsedControllerRight{};
                parsedControllerRight = ReadString(*member);
                ControllerRight = std::move(parsedControllerRight);
            }
        }
    }

    PlayerBasicProfileResponseStats PlayerBasicProfileResponseStats::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerBasicProfileResponseStats value;
        value.Parse(document);
        return value;
    }

    void PlayerBasicProfileResponseStats::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "realmId")) {
            RealmId = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "realmName")) {
            RealmName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rank")) {
            Rank = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "countryRank")) {
            CountryRank = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPP")) {
            TotalPP = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalScore")) {
            TotalScore = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalRankedScore")) {
            TotalRankedScore = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPlayedLeaderboards")) {
            TotalPlayedLeaderboards = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPlayedRankedLeaderboards")) {
            TotalPlayedRankedLeaderboards = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalSubmittedPlays")) {
            TotalSubmittedPlays = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalReplayViews")) {
            TotalReplayViews = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "averageAccuracy")) {
            AverageAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "weightedAverageAccuracy")) {
            WeightedAverageAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "completionAccuracy")) {
            CompletionAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "device")) {
            if (member->IsObject())
                Device.Parse(*member);
        }
    }

    PlayerBasicProfileResponse PlayerBasicProfileResponse::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerBasicProfileResponse value;
        value.Parse(document);
        return value;
    }

    void PlayerBasicProfileResponse::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerNameInGame")) {
            PlayerNameInGame = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "country")) {
            Country = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "role")) {
            if (!member->IsNull()) {
                std::string parsedRole{};
                parsedRole = ReadString(*member);
                Role = std::move(parsedRole);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "avatar")) {
            Avatar = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "avatarVersion")) {
            AvatarVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "permissions")) {
            Permissions = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "banned")) {
            Banned = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "silenced")) {
            Silenced = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "inactive")) {
            Inactive = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "stats")) {
            if (member->IsObject())
                Stats.Parse(*member);
        }
    }

    PlayerHistoryEntry PlayerHistoryEntry::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        PlayerHistoryEntry value;
        value.Parse(document);
        return value;
    }

    void PlayerHistoryEntry::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "rank")) {
            Rank = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPP")) {
            TotalPP = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalScore")) {
            TotalScore = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalRankedScore")) {
            TotalRankedScore = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPlayedLeaderboards")) {
            TotalPlayedLeaderboards = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPlayedRankedLeaderboards")) {
            TotalPlayedRankedLeaderboards = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalSubmittedPlays")) {
            TotalSubmittedPlays = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalReplayViews")) {
            TotalReplayViews = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "averageAccuracy")) {
            AverageAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "weightedAverageAccuracy")) {
            WeightedAverageAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "completionAccuracy")) {
            CompletionAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "estimated")) {
            Estimated = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
    }

    GlobalPlayerHistoryEntry GlobalPlayerHistoryEntry::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        GlobalPlayerHistoryEntry value;
        value.Parse(document);
        return value;
    }

    void GlobalPlayerHistoryEntry::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "rank")) {
            Rank = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPP")) {
            TotalPP = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalScore")) {
            TotalScore = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalRankedScore")) {
            TotalRankedScore = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPlayedLeaderboards")) {
            TotalPlayedLeaderboards = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalPlayedRankedLeaderboards")) {
            TotalPlayedRankedLeaderboards = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalSubmittedPlays")) {
            TotalSubmittedPlays = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "totalReplayViews")) {
            TotalReplayViews = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "averageAccuracy")) {
            AverageAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "weightedAverageAccuracy")) {
            WeightedAverageAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "completionAccuracy")) {
            CompletionAccuracy = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "estimated")) {
            Estimated = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
    }

    RealmSummary RealmSummary::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        RealmSummary value;
        value.Parse(document);
        return value;
    }

    void RealmSummary::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "startDate")) {
            StartDate = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "endDate")) {
            if (!member->IsNull()) {
                std::string parsedEndDate{};
                parsedEndDate = ReadString(*member);
                EndDate = std::move(parsedEndDate);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "isAcceptingScores")) {
            IsAcceptingScores = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hasPP")) {
            HasPP = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hasHistory")) {
            HasHistory = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "decayFactor")) {
            DecayFactor = ReadDouble(*member);
        }
    }

    RealmDetailsResponse RealmDetailsResponse::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        RealmDetailsResponse value;
        value.Parse(document);
        return value;
    }

    void RealmDetailsResponse::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "startDate")) {
            StartDate = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "endDate")) {
            if (!member->IsNull()) {
                std::string parsedEndDate{};
                parsedEndDate = ReadString(*member);
                EndDate = std::move(parsedEndDate);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "isAcceptingScores")) {
            IsAcceptingScores = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hasPP")) {
            HasPP = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "hasHistory")) {
            HasHistory = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "decayFactor")) {
            DecayFactor = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "leaderboardCount")) {
            LeaderboardCount = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerCount")) {
            PlayerCount = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "scoreCount")) {
            ScoreCount = ReadDouble(*member);
        }
    }

    LivePlayerTournamentSummary LivePlayerTournamentSummary::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LivePlayerTournamentSummary value;
        value.Parse(document);
        return value;
    }

    void LivePlayerTournamentSummary::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "tournamentId")) {
            TournamentId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "status")) {
            Status = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "permissions")) {
            if (member->IsArray()) {
                Permissions.clear();
                for (auto& item : member->GetArray()) {
                    std::string parsedItem{};
                    parsedItem = ReadString(item);
                    Permissions.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "roomSummary")) {
            RoomSummary = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "updatedAt")) {
            UpdatedAt = ReadString(*member);
        }
    }

    LivePlayerRoomSummarySelectedSong LivePlayerRoomSummarySelectedSong::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LivePlayerRoomSummarySelectedSong value;
        value.Parse(document);
        return value;
    }

    void LivePlayerRoomSummarySelectedSong::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "tournamentId")) {
            TournamentId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "mapId")) {
            MapId = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "beatSaverKey")) {
            if (!member->IsNull()) {
                std::string parsedBeatSaverKey{};
                parsedBeatSaverKey = ReadString(*member);
                BeatSaverKey = std::move(parsedBeatSaverKey);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "mapHash")) {
            MapHash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "difficulty")) {
            Difficulty = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "characteristic")) {
            Characteristic = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "leaderboardId")) {
            if (!member->IsNull()) {
                double parsedLeaderboardId{};
                parsedLeaderboardId = ReadDouble(*member);
                LeaderboardId = std::move(parsedLeaderboardId);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "songName")) {
            SongName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songSubName")) {
            SongSubName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songAuthorName")) {
            SongAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "levelAuthorName")) {
            LevelAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bpm")) {
            Bpm = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "nps")) {
            Nps = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "durationSeconds")) {
            DurationSeconds = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "maxScore")) {
            MaxScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "downloadUrl")) {
            if (!member->IsNull()) {
                std::string parsedDownloadUrl{};
                parsedDownloadUrl = ReadString(*member);
                DownloadUrl = std::move(parsedDownloadUrl);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "coverUrl")) {
            if (!member->IsNull()) {
                std::string parsedCoverUrl{};
                parsedCoverUrl = ReadString(*member);
                CoverUrl = std::move(parsedCoverUrl);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "updatedAt")) {
            UpdatedAt = ReadString(*member);
        }
    }

    LivePlayerRoomSummary LivePlayerRoomSummary::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LivePlayerRoomSummary value;
        value.Parse(document);
        return value;
    }

    void LivePlayerRoomSummary::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "tournamentId")) {
            TournamentId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "matchId")) {
            MatchId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "inviteCode")) {
            InviteCode = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "state")) {
            State = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rosterMode")) {
            RosterMode = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerCount")) {
            PlayerCount = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "selectedSong")) {
            if (member->IsObject())
                SelectedSong.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "updatedAt")) {
            UpdatedAt = ReadString(*member);
        }
    }

    LivePlayerRoomDetailsSelectedSong LivePlayerRoomDetailsSelectedSong::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LivePlayerRoomDetailsSelectedSong value;
        value.Parse(document);
        return value;
    }

    void LivePlayerRoomDetailsSelectedSong::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "tournamentId")) {
            TournamentId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "mapId")) {
            MapId = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "beatSaverKey")) {
            if (!member->IsNull()) {
                std::string parsedBeatSaverKey{};
                parsedBeatSaverKey = ReadString(*member);
                BeatSaverKey = std::move(parsedBeatSaverKey);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "mapHash")) {
            MapHash = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "difficulty")) {
            Difficulty = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "characteristic")) {
            Characteristic = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "leaderboardId")) {
            if (!member->IsNull()) {
                double parsedLeaderboardId{};
                parsedLeaderboardId = ReadDouble(*member);
                LeaderboardId = std::move(parsedLeaderboardId);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "songName")) {
            SongName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songSubName")) {
            SongSubName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "songAuthorName")) {
            SongAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "levelAuthorName")) {
            LevelAuthorName = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "bpm")) {
            Bpm = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "nps")) {
            Nps = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "durationSeconds")) {
            DurationSeconds = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "maxScore")) {
            MaxScore = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "downloadUrl")) {
            if (!member->IsNull()) {
                std::string parsedDownloadUrl{};
                parsedDownloadUrl = ReadString(*member);
                DownloadUrl = std::move(parsedDownloadUrl);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "coverUrl")) {
            if (!member->IsNull()) {
                std::string parsedCoverUrl{};
                parsedCoverUrl = ReadString(*member);
                CoverUrl = std::move(parsedCoverUrl);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "updatedAt")) {
            UpdatedAt = ReadString(*member);
        }
    }

    LivePlayerRoomDetailsMembersItemPlayer LivePlayerRoomDetailsMembersItemPlayer::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LivePlayerRoomDetailsMembersItemPlayer value;
        value.Parse(document);
        return value;
    }

    void LivePlayerRoomDetailsMembersItemPlayer::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "id")) {
            Id = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "name")) {
            Name = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerNameInGame")) {
            PlayerNameInGame = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "country")) {
            Country = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "role")) {
            if (!member->IsNull()) {
                std::string parsedRole{};
                parsedRole = ReadString(*member);
                Role = std::move(parsedRole);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "avatar")) {
            Avatar = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "avatarVersion")) {
            AvatarVersion = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "permissions")) {
            Permissions = ReadDouble(*member);
        }
    }

    LivePlayerRoomDetailsMembersItem LivePlayerRoomDetailsMembersItem::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LivePlayerRoomDetailsMembersItem value;
        value.Parse(document);
        return value;
    }

    void LivePlayerRoomDetailsMembersItem::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "playerId")) {
            PlayerId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "player")) {
            if (member->IsObject())
                Player.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "teamId")) {
            if (!member->IsNull()) {
                double parsedTeamId{};
                parsedTeamId = ReadDouble(*member);
                TeamId = std::move(parsedTeamId);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "teamName")) {
            if (!member->IsNull()) {
                std::string parsedTeamName{};
                parsedTeamName = ReadString(*member);
                TeamName = std::move(parsedTeamName);
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "connected")) {
            Connected = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "isBot")) {
            IsBot = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "role")) {
            Role = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "active")) {
            Active = ReadBool(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playState")) {
            PlayState = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "downloadState")) {
            DownloadState = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "joinedAt")) {
            JoinedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "lastSeenAt")) {
            LastSeenAt = ReadString(*member);
        }
    }

    LivePlayerRoomDetails LivePlayerRoomDetails::FromJson(std::string_view json)
    {
        rapidjson::Document document = ParseDocument(json);
        LivePlayerRoomDetails value;
        value.Parse(document);
        return value;
    }

    void LivePlayerRoomDetails::Parse(const rapidjson::Value& value)
    {
        if (const rapidjson::Value* member = FindMember(value, "tournamentId")) {
            TournamentId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "matchId")) {
            MatchId = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "inviteCode")) {
            InviteCode = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "state")) {
            State = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "rosterMode")) {
            RosterMode = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "playerCount")) {
            PlayerCount = ReadDouble(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "selectedSong")) {
            if (member->IsObject())
                SelectedSong.Parse(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "members")) {
            if (member->IsArray()) {
                Members.clear();
                for (auto& item : member->GetArray()) {
                    LivePlayerRoomDetailsMembersItem parsedItem{};
                    if (item.IsObject())
                        parsedItem.Parse(item);
                    Members.push_back(std::move(parsedItem));
                }
            }
        }
        if (const rapidjson::Value* member = FindMember(value, "createdAt")) {
            CreatedAt = ReadString(*member);
        }
        if (const rapidjson::Value* member = FindMember(value, "updatedAt")) {
            UpdatedAt = ReadString(*member);
        }
    }

    std::string ToJson(const GameAuthenticateRequest& value)
    {
        rapidjson::Document document;
        document.SetObject();
        auto& allocator = document.GetAllocator();
        document.AddMember("at", value.At, allocator);
        document.AddMember("playerId", rapidjson::Value(value.PlayerId.c_str(), allocator), allocator);
        document.AddMember("nonce", rapidjson::Value(value.Nonce.c_str(), allocator), allocator);
        document.AddMember("friends", rapidjson::Value(value.Friends.c_str(), allocator), allocator);
        document.AddMember("name", rapidjson::Value(value.Name.c_str(), allocator), allocator);
        if (!value.ClientBuildId.empty())
            document.AddMember("clientBuildId", rapidjson::Value(value.ClientBuildId.c_str(), allocator), allocator);
        if (value.UploadProtocolVersion != 0)
            document.AddMember("uploadProtocolVersion", value.UploadProtocolVersion, allocator);
        if (!value.PluginVersion.empty())
            document.AddMember("pluginVersion", rapidjson::Value(value.PluginVersion.c_str(), allocator), allocator);
        if (!value.GameVersion.empty())
            document.AddMember("gameVersion", rapidjson::Value(value.GameVersion.c_str(), allocator), allocator);
        if (!value.UploadVersionHash.empty())
            document.AddMember("uploadVersionHash", rapidjson::Value(value.UploadVersionHash.c_str(), allocator), allocator);
        if (!value.ClientKind.empty())
            document.AddMember("clientKind", rapidjson::Value(value.ClientKind.c_str(), allocator), allocator);
        if (!value.ClientProof.empty())
            document.AddMember("clientProof", rapidjson::Value(value.ClientProof.c_str(), allocator), allocator);
        if (!value.DevUploadToken.empty())
            document.AddMember("devUploadToken", rapidjson::Value(value.DevUploadToken.c_str(), allocator), allocator);
        if (!value.ArtifactSha256.empty())
            document.AddMember("artifactSha256", rapidjson::Value(value.ArtifactSha256.c_str(), allocator), allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        return buffer.GetString();
    }

    SnoreSaberApiGeneratedClient::SnoreSaberApiGeneratedClient(std::string baseUrl)
        : _baseUrl(TrimBaseUrl(std::move(baseUrl)))
    {
    }

    GameAuthenticateResponse SnoreSaberApiGeneratedClient::AuthenticateGame(const GameAuthenticateRequest& body)
    {
        auto [statusCode, response] = WebUtils::PostJsonSync(_baseUrl + "/api/v2/game/auth", ToJson(body), TimeoutSeconds);
        // The SnoreSaber API can return either 200 OK or 201 Created for a successful session.
        EnsureSuccess(statusCode, response, {200, 201});
        return GameAuthenticateResponse::FromJson(response);
    }

    GameUploadResponse SnoreSaberApiGeneratedClient::UploadScore(std::string sessionKey, std::string sessionId, const std::string& data, const std::vector<char>& replay,
                                                                 std::optional<std::string> uploadVersionHash,
                                                                 std::optional<std::string> uploadSignature,
                                                                 std::optional<std::string> replaySha256,
                                                                 std::optional<std::string> uploadNonce,
                                                                 std::optional<std::string> uploadTimestamp,
                                                                 std::optional<std::string> clientBuildId,
                                                                 std::optional<std::string> uploadProtocol)
    {
        std::vector<std::string> headers = {
            "x-session-key: " + sessionKey,
            "x-session-id: " + sessionId,
        };
        if (uploadVersionHash.has_value())
            headers.push_back("x-upload-version-hash: " + uploadVersionHash.value());
        if (uploadSignature.has_value())
            headers.push_back("x-upload-signature: " + uploadSignature.value());
        if (replaySha256.has_value())
            headers.push_back("x-replay-sha256: " + replaySha256.value());
        if (uploadNonce.has_value())
            headers.push_back("x-upload-nonce: " + uploadNonce.value());
        if (uploadTimestamp.has_value())
            headers.push_back("x-upload-timestamp: " + uploadTimestamp.value());
        if (clientBuildId.has_value())
            headers.push_back("x-client-build-id: " + clientBuildId.value());
        if (uploadProtocol.has_value())
            headers.push_back("x-upload-protocol: " + uploadProtocol.value());

        auto [statusCode, response] = WebUtils::PostWithReplaySync(_baseUrl + "/api/v2/game/upload", replay, data, 30, headers);
        EnsureSuccess(statusCode, response, {200});
        return GameUploadResponse::FromJson(response);
    }

    LeaderboardResponse SnoreSaberApiGeneratedClient::GetLeaderboard(std::string hash, std::string mode, int difficulty, std::optional<int> realmId)
    {
        std::string url = _baseUrl + "/api/v2/leaderboards/hash/" + hash + "/" + mode + "/" + std::to_string(difficulty);
        if (realmId.has_value())
            AddQuery(url, "realmId", realmId.value());
        return GetJson<LeaderboardResponse>(url);
    }

    LeaderboardScoresResponse SnoreSaberApiGeneratedClient::GetLeaderboardScores(std::string hash, std::string mode, int difficulty, std::optional<int> page,
                                                                                 std::optional<int> limit, std::optional<Pivot> pivot,
                                                                                 std::optional<std::string> scope, std::optional<std::string> hideNA,
                                                                                 std::optional<int> realmId, std::optional<std::string> includePlayerScore,
                                                                                 RequestHeaders headers)
    {
        std::string url = _baseUrl + "/api/v2/leaderboards/hash/" + hash + "/" + mode + "/" + std::to_string(difficulty) + "/scores";
        if (page.has_value())
            AddQuery(url, "page", page.value());
        if (limit.has_value())
            AddQuery(url, "limit", limit.value());
        if (pivot.has_value())
            AddQuery(url, "pivot", PivotValue(pivot.value()));
        if (scope.has_value())
            AddQuery(url, "scope", scope.value());
        if (hideNA.has_value())
            AddQuery(url, "hideNA", hideNA.value());
        if (realmId.has_value())
            AddQuery(url, "realmId", realmId.value());
        if (includePlayerScore.has_value())
            AddQuery(url, "includePlayerScore", includePlayerScore.value());
        return GetJson<LeaderboardScoresResponse>(url, headers);
    }

    MapDetailsResponse SnoreSaberApiGeneratedClient::GetMapById(int id, std::optional<int> realmId)
    {
        std::string url = _baseUrl + "/api/v2/maps/" + std::to_string(id);
        if (realmId.has_value())
            AddQuery(url, "realmId", realmId.value());
        return GetJson<MapDetailsResponse>(url);
    }

    MapDetailsResponse SnoreSaberApiGeneratedClient::GetMapByHash(std::string hash, std::optional<int> realmId)
    {
        std::string url = _baseUrl + "/api/v2/maps/hash/" + hash;
        if (realmId.has_value())
            AddQuery(url, "realmId", realmId.value());
        return GetJson<MapDetailsResponse>(url);
    }

    PlayerListResponse SnoreSaberApiGeneratedClient::GetPlayers(std::optional<int> page, std::optional<int> limit, std::optional<std::string> countries,
                                                                std::optional<PlayerScope> scope, std::optional<int> realmId, std::optional<Pivot> pivot,
                                                                RequestHeaders headers)
    {
        std::string url = _baseUrl + "/api/v2/players";
        if (page.has_value())
            AddQuery(url, "page", page.value());
        if (limit.has_value())
            AddQuery(url, "limit", limit.value());
        if (countries.has_value())
            AddQuery(url, "countries", countries.value());
        if (scope.has_value())
            AddQuery(url, "scope", PlayerScopeValue(scope.value()));
        if (realmId.has_value())
            AddQuery(url, "realmId", realmId.value());
        if (pivot.has_value())
            AddQuery(url, "pivot", PivotValue(pivot.value()));
        return GetJson<PlayerListResponse>(url, headers);
    }

    PlayerProfileResponse SnoreSaberApiGeneratedClient::GetPlayer(std::string id, std::optional<int> realmId)
    {
        std::string url = _baseUrl + "/api/v2/players/" + id;
        if (realmId.has_value())
            AddQuery(url, "realmId", realmId.value());
        return GetJson<PlayerProfileResponse>(url);
    }

    PlayerBasicProfileResponse SnoreSaberApiGeneratedClient::GetPlayerBasic(std::string id, std::optional<int> realmId)
    {
        std::string url = _baseUrl + "/api/v2/players/" + id + "/basic";
        if (realmId.has_value())
            AddQuery(url, "realmId", realmId.value());
        return GetJson<PlayerBasicProfileResponse>(url);
    }

    std::vector<GlobalPlayerHistoryEntry> SnoreSaberApiGeneratedClient::GetGlobalPlayerHistory(std::string id)
    {
        auto [statusCode, response] = WebUtils::GetSync(_baseUrl + "/api/v2/players/" + id + "/global-history", TimeoutSeconds);
        EnsureSuccess(statusCode, response, {200});
        rapidjson::Document document = ParseDocument(response);
        std::vector<GlobalPlayerHistoryEntry> entries;
        if (!document.IsArray())
            return entries;

        for (auto& item : document.GetArray()) {
            GlobalPlayerHistoryEntry entry;
            entry.Parse(item);
            entries.push_back(std::move(entry));
        }
        return entries;
    }

    std::vector<LivePlayerTournamentSummary> SnoreSaberApiGeneratedClient::ListLivePlayerTournaments(RequestHeaders headers)
    {
        return GetJsonArray<LivePlayerTournamentSummary>(_baseUrl + "/api/v2/live/player/tournaments", headers);
    }

    std::vector<LivePlayerRoomSummary> SnoreSaberApiGeneratedClient::ListLivePlayerRooms(std::string tournamentId, RequestHeaders headers)
    {
        return GetJsonArray<LivePlayerRoomSummary>(_baseUrl + "/api/v2/live/player/tournaments/" + tournamentId + "/rooms", headers);
    }

    LivePlayerRoomDetails SnoreSaberApiGeneratedClient::GetLivePlayerRoom(std::string tournamentId, std::string matchId, RequestHeaders headers)
    {
        return GetJson<LivePlayerRoomDetails>(_baseUrl + "/api/v2/live/player/tournaments/" + tournamentId + "/rooms/" + matchId, headers);
    }

    LivePlayerRoomDetails SnoreSaberApiGeneratedClient::GetLivePlayerRoomByInviteCode(std::string inviteCode, RequestHeaders headers)
    {
        return GetJson<LivePlayerRoomDetails>(_baseUrl + "/api/v2/live/player/rooms/by-invite-code/" + inviteCode, headers);
    }

    std::vector<char> SnoreSaberApiGeneratedClient::DownloadReplay(int id)
    {
        std::vector<char> replay;
        long statusCode = WebUtils::DownloadReplaySync(_baseUrl + "/api/v2/scores/" + std::to_string(id) + "/replay", replay, TimeoutSeconds);
        EnsureSuccess(statusCode, "", {200, 206});
        return replay;
    }
} // namespace SnoreSaber::Core::Api::Generated
