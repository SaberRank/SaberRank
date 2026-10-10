#include "Core/BeatSaver/BeatSaverModels.hpp"

#include <beatsaber-hook/shared/config/rapidjson-utils.hpp>

namespace SnoreSaber::Core::BeatSaver
{
    namespace
    {
        std::string GetString(const rapidjson::Value& object, const char* name)
        {
            auto member = object.FindMember(name);
            if (member == object.MemberEnd() || !member->value.IsString())
            {
                return std::string();
            }
            return std::string(member->value.GetString(), member->value.GetStringLength());
        }

        std::optional<float> GetFloat(const rapidjson::Value& object, const char* name)
        {
            auto member = object.FindMember(name);
            if (member == object.MemberEnd() || !member->value.IsNumber())
            {
                return std::nullopt;
            }
            return member->value.GetFloat();
        }

        std::optional<int> GetInt(const rapidjson::Value& object, const char* name)
        {
            auto member = object.FindMember(name);
            if (member == object.MemberEnd() || !member->value.IsInt())
            {
                return std::nullopt;
            }
            return member->value.GetInt();
        }

        BeatSaverDifficulty ParseDifficulty(const rapidjson::Value& object)
        {
            BeatSaverDifficulty difficulty;
            difficulty.difficulty = GetString(object, "difficulty");
            difficulty.characteristic = GetString(object, "characteristic");
            difficulty.nps = GetFloat(object, "nps");
            difficulty.notes = GetInt(object, "notes");
            difficulty.obstacles = GetInt(object, "obstacles");
            difficulty.bombs = GetInt(object, "bombs");
            difficulty.njs = GetFloat(object, "njs");
            difficulty.offset = GetFloat(object, "offset");
            return difficulty;
        }

        BeatSaverVersion ParseVersion(const rapidjson::Value& object)
        {
            BeatSaverVersion version;
            version.hash = GetString(object, "hash");
            version.downloadUrl = GetString(object, "downloadURL");
            version.coverUrl = GetString(object, "coverURL");

            auto diffs = object.FindMember("diffs");
            if (diffs != object.MemberEnd() && diffs->value.IsArray())
            {
                for (const auto& diff : diffs->value.GetArray())
                {
                    if (diff.IsObject())
                    {
                        version.diffs.push_back(ParseDifficulty(diff));
                    }
                }
            }
            return version;
        }
    } // namespace

    std::optional<BeatSaverMap> BeatSaverMap::TryParse(std::string_view json)
    {
        rapidjson::Document document;
        document.Parse(json.data(), json.size());
        if (document.HasParseError() || !document.IsObject())
        {
            return std::nullopt;
        }

        BeatSaverMap map;
        map.name = GetString(document, "name");

        auto metadata = document.FindMember("metadata");
        if (metadata != document.MemberEnd() && metadata->value.IsObject())
        {
            BeatSaverMapMetadata parsed;
            parsed.songName = GetString(metadata->value, "songName");
            parsed.songSubName = GetString(metadata->value, "songSubName");
            parsed.songAuthorName = GetString(metadata->value, "songAuthorName");
            parsed.levelAuthorName = GetString(metadata->value, "levelAuthorName");
            parsed.duration = GetFloat(metadata->value, "duration");
            parsed.bpm = GetFloat(metadata->value, "bpm");
            map.metadata = std::move(parsed);
        }

        auto uploader = document.FindMember("uploader");
        if (uploader != document.MemberEnd() && uploader->value.IsObject())
        {
            map.uploader = BeatSaverUploader {GetString(uploader->value, "name")};
        }

        auto versions = document.FindMember("versions");
        if (versions != document.MemberEnd() && versions->value.IsArray())
        {
            for (const auto& version : versions->value.GetArray())
            {
                if (version.IsObject())
                {
                    map.versions.push_back(ParseVersion(version));
                }
            }
        }

        return map;
    }
} // namespace SnoreSaber::Core::BeatSaver
