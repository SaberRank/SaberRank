#include "Data/Private/Settings.hpp"

#include "static.hpp"
#include "logging.hpp"

#include <beatsaber-hook/shared/config/rapidjson-utils.hpp>
#include <beatsaber-hook/shared/rapidjson/include/rapidjson/document.h>
#include <beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h>
#include <beatsaber-hook/shared/rapidjson/include/rapidjson/writer.h>
#include <beatsaber-hook/shared/utils/utils-functions.h>

#include <utility>

namespace SnoreSaber::Data::Private {

    namespace Settings {
        constexpr int currentVersion = 5;

        int fileVersion;
        bool showLocalPlayerRank;
        bool showScorePP;
        bool showStatusText;
        bool saveLocalReplays;
        bool enableCountryLeaderboards;
        std::string locationFilterMode;
        bool hideNAScoresFromLeaderboard;
        bool hasClickedSnoreSaberLogo;
        bool hasOpenedReplayUI;
        bool leftHandedReplayUI;
        bool lockedReplayUIMode;
        bool replayOverrideHandedness;
        bool useRecordedPlayerSettings;
        bool shareHsvProfiles;
        bool publicLivePresenceOptOut;
        bool liveChatOverlayEnabled;
        bool liveChatOverlayGameplayEnabled;
        float liveChatOverlayScale;
        float liveChatOverlayTextScale;
        std::vector<SpectatorPoseRoot> spectatorPositions;


        void InitializeDefaults() {
            fileVersion = currentVersion;
            showLocalPlayerRank = true;
            showScorePP = true;
            showStatusText = true;
            saveLocalReplays = true;
            enableCountryLeaderboards = true;
            locationFilterMode = "Country";
            hideNAScoresFromLeaderboard = false;
            hasClickedSnoreSaberLogo = false;
            hasOpenedReplayUI = false;
            leftHandedReplayUI = false;
            lockedReplayUIMode = false;
            replayOverrideHandedness = false;
            useRecordedPlayerSettings = true;
            shareHsvProfiles = false;
            publicLivePresenceOptOut = false;
            liveChatOverlayEnabled = true;
            liveChatOverlayGameplayEnabled = true;
            liveChatOverlayScale = 1.15f;
            liveChatOverlayTextScale = 1.25f;
            InitializeDefaultSpectatorPositions();
        }

        void InitializeDefaultSpectatorPositions() {
            spectatorPositions = {
                SpectatorPoseRoot(SpectatorPose(0, 0, -2), "Main"),
                SpectatorPoseRoot(SpectatorPose(0, 4, 0), "Bird's Eye"),
                SpectatorPoseRoot(SpectatorPose(-3, 0, 0), "Left"),
                SpectatorPoseRoot(SpectatorPose(3, 0, 0), "Right"),
            };
        }

        const rapidjson::Value* FindSetting(const rapidjson::Value& object, const char* name) {
            if (!object.IsObject()) {
                return nullptr;
            }

            auto member = object.FindMember(name);
            return member != object.MemberEnd() ? &member->value : nullptr;
        }

        bool ReadIntSetting(const rapidjson::Document& doc, const char* name, int& value) {
            auto setting = FindSetting(doc, name);
            if (!setting) {
                return true;
            }
            if (!setting->IsInt()) {
                ERROR("Invalid int setting: {:s}", name);
                return false;
            }

            value = setting->GetInt();
            return true;
        }

        bool ReadBoolSetting(const rapidjson::Document& doc, const char* name, bool& value) {
            auto setting = FindSetting(doc, name);
            if (!setting) {
                return true;
            }
            if (!setting->IsBool()) {
                ERROR("Invalid bool setting: {:s}", name);
                return false;
            }

            value = setting->GetBool();
            return true;
        }

        bool ReadFloatSetting(const rapidjson::Document& doc, const char* name, float& value) {
            auto setting = FindSetting(doc, name);
            if (!setting) {
                return true;
            }
            if (!setting->IsNumber()) {
                ERROR("Invalid float setting: {:s}", name);
                return false;
            }

            value = setting->GetFloat();
            return true;
        }

        bool ReadStringSetting(const rapidjson::Document& doc, const char* name, std::string& value) {
            auto setting = FindSetting(doc, name);
            if (!setting) {
                return true;
            }
            if (!setting->IsString()) {
                ERROR("Invalid string setting: {:s}", name);
                return false;
            }

            value = setting->GetString();
            return true;
        }

        bool ReadSpectatorPositions(const rapidjson::Document& doc) {
            auto setting = FindSetting(doc, "spectatorPositions");
            if (!setting) {
                return true;
            }
            if (!setting->IsArray()) {
                ERROR("Invalid spectatorPositions setting");
                return false;
            }

            std::vector<SpectatorPoseRoot> parsedPositions;
            for (auto&& position : setting->GetArray()) {
                auto name = FindSetting(position, "name");
                auto pose = FindSetting(position, "spectatorPose");
                auto x = pose ? FindSetting(*pose, "x") : nullptr;
                auto y = pose ? FindSetting(*pose, "y") : nullptr;
                auto z = pose ? FindSetting(*pose, "z") : nullptr;

                if (!name || !name->IsString() ||
                    !x || !x->IsNumber() ||
                    !y || !y->IsNumber() ||
                    !z || !z->IsNumber()) {
                    ERROR("Invalid spectator position setting");
                    return false;
                }

                parsedPositions.emplace_back(SpectatorPose(x->GetFloat(), y->GetFloat(), z->GetFloat()), name->GetString());
            }

            if (parsedPositions.empty()) {
                ERROR("No spectator positions in settings");
                return false;
            }

            spectatorPositions = std::move(parsedPositions);
            return true;
        }

        void LoadSettings() {
            InitializeDefaults();

            if (!fileexists(Static::SETTINGS_PATH)) {
                SaveSettings();
                return;
            }

            rapidjson::Document doc;
            auto settings_file = readfile(Static::SETTINGS_PATH);
            doc.Parse(settings_file.data(), settings_file.size());

            if (doc.HasParseError() || !doc.IsObject()) {
                ERROR("Invalid settings file, restoring defaults");
                SaveSettings();
                return;
            }

            bool validSettings = true;
            validSettings &= ReadIntSetting(doc, "fileVersion", fileVersion);
            validSettings &= ReadBoolSetting(doc, "showLocalPlayerRank", showLocalPlayerRank);
            validSettings &= ReadBoolSetting(doc, "showScorePP", showScorePP);
            validSettings &= ReadBoolSetting(doc, "showStatusText", showStatusText);
            validSettings &= ReadBoolSetting(doc, "saveLocalReplays", saveLocalReplays);
            validSettings &= ReadBoolSetting(doc, "enableCountryLeaderboards", enableCountryLeaderboards);
            validSettings &= ReadStringSetting(doc, "locationFilterMode", locationFilterMode);
            validSettings &= ReadBoolSetting(doc, "hideNAScoresFromLeaderboard", hideNAScoresFromLeaderboard);
            validSettings &= ReadBoolSetting(doc, "hasClickedSnoreSaberLogo", hasClickedSnoreSaberLogo);
            validSettings &= ReadBoolSetting(doc, "hasOpenedReplayUI", hasOpenedReplayUI);
            validSettings &= ReadBoolSetting(doc, "leftHandedReplayUI", leftHandedReplayUI);
            validSettings &= ReadBoolSetting(doc, "lockedReplayUIMode", lockedReplayUIMode);
            validSettings &= ReadBoolSetting(doc, "replayOverrideHandedness", replayOverrideHandedness);
            validSettings &= ReadBoolSetting(doc, "useRecordedPlayerSettings", useRecordedPlayerSettings);
            validSettings &= ReadBoolSetting(doc, "shareHsvProfiles", shareHsvProfiles);
            validSettings &= ReadBoolSetting(doc, "publicLivePresenceOptOut", publicLivePresenceOptOut);
            validSettings &= ReadBoolSetting(doc, "liveChatOverlayEnabled", liveChatOverlayEnabled);
            validSettings &= ReadBoolSetting(doc, "liveChatOverlayGameplayEnabled", liveChatOverlayGameplayEnabled);
            validSettings &= ReadFloatSetting(doc, "liveChatOverlayScale", liveChatOverlayScale);
            validSettings &= ReadFloatSetting(doc, "liveChatOverlayTextScale", liveChatOverlayTextScale);
            validSettings &= ReadSpectatorPositions(doc);

            if (spectatorPositions.empty()) InitializeDefaultSpectatorPositions(); // make sure we always have some spectator positions

            if (!validSettings || fileVersion < currentVersion) {
                // add settings upgrade code here once needed
                SaveSettings();
            }
        }

        void SaveSettings() {
            INFO("saving settings");

            fileVersion = currentVersion;

            rapidjson::StringBuffer s;
            rapidjson::Writer<rapidjson::StringBuffer> writer(s);

            writer.StartObject();
            writer.String("fileVersion");
            writer.Int(fileVersion);
            writer.String("showLocalPlayerRank");
            writer.Bool(showLocalPlayerRank);
            writer.String("showScorePP");
            writer.Bool(showScorePP);
            writer.String("showStatusText");
            writer.Bool(showStatusText);
            writer.String("saveLocalReplays");
            writer.Bool(saveLocalReplays);
            writer.String("enableCountryLeaderboards");
            writer.Bool(enableCountryLeaderboards);
            writer.String("locationFilterMode");
            writer.String(locationFilterMode);
            writer.String("hideNAScoresFromLeaderboard");
            writer.Bool(hideNAScoresFromLeaderboard);
            writer.String("hasClickedSnoreSaberLogo");
            writer.Bool(hasClickedSnoreSaberLogo);
            writer.String("hasOpenedReplayUI");
            writer.Bool(hasOpenedReplayUI);
            writer.String("leftHandedReplayUI");
            writer.Bool(leftHandedReplayUI);
            writer.String("lockedReplayUIMode");
            writer.Bool(lockedReplayUIMode);
            writer.String("replayOverrideHandedness");
            writer.Bool(replayOverrideHandedness);
            writer.String("useRecordedPlayerSettings");
            writer.Bool(useRecordedPlayerSettings);
            writer.String("shareHsvProfiles");
            writer.Bool(shareHsvProfiles);
            writer.String("publicLivePresenceOptOut");
            writer.Bool(publicLivePresenceOptOut);
            writer.String("liveChatOverlayEnabled");
            writer.Bool(liveChatOverlayEnabled);
            writer.String("liveChatOverlayGameplayEnabled");
            writer.Bool(liveChatOverlayGameplayEnabled);
            writer.String("liveChatOverlayScale");
            writer.Double(liveChatOverlayScale);
            writer.String("liveChatOverlayTextScale");
            writer.Double(liveChatOverlayTextScale);
            writer.String("spectatorPositions");
            writer.StartArray();
            for (auto position : spectatorPositions) {
                writer.StartObject();
                writer.String("name");
                writer.String(position.name);

                writer.String("spectatorPose");
                writer.StartObject();
                writer.String("x");
                writer.Double(position.spectatorPose.x);
                writer.String("y");
                writer.Double(position.spectatorPose.y);
                writer.String("z");
                writer.Double(position.spectatorPose.z);
                writer.EndObject();

                writer.EndObject();
            }
            writer.EndArray();
            writer.EndObject();
            
            assert(writer.IsComplete());

            if (!writefile(Static::SETTINGS_PATH, s.GetString())) {
                ERROR("Failed to write settings");
            }
        }
    }

    SpectatorPose::SpectatorPose(float x, float y, float z) : x(x), y(y), z(z) { }
    SpectatorPoseRoot::SpectatorPoseRoot(SpectatorPose spectatorPose, std::string name) : name(name), spectatorPose(spectatorPose) { }
}
