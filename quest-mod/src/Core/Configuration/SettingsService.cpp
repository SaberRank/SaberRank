#include "Core/Configuration/SettingsService.hpp"

#include "Data/Private/Settings.hpp"
#include "Services/FileService.hpp"
#include "static.hpp"

#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/BeatmapDifficultySerializedMethods.hpp>

#include <utility>

DEFINE_TYPE(SnoreSaber::Core::Configuration, SettingsService);

namespace SnoreSaber::Core::Configuration
{
    void SettingsService::ctor()
    {
        INVOKE_CTOR();
    }

    void SettingsService::Load()
    {
        Data::Private::Settings::LoadSettings();
    }

    void SettingsService::Save()
    {
        Data::Private::Settings::SaveSettings();
    }

    std::string SettingsService::DataPath() const
    {
        return Static::DATA_DIR;
    }

    std::string SettingsService::ConfigPath() const
    {
        return Static::DATA_DIR;
    }

    std::string SettingsService::ReplayPath() const
    {
        return Static::REPLAY_DIR;
    }

    std::string SettingsService::ReplayPathFor(const std::string& playerId, const std::string& songHash, GlobalNamespace::BeatmapKey beatmapKey) const
    {
        std::string difficulty = GlobalNamespace::BeatmapDifficultySerializedMethods::SerializedName(beatmapKey.difficulty);
        std::string characteristic = beatmapKey.beatmapCharacteristic ? (std::string)beatmapKey.beatmapCharacteristic->serializedName : "Unknown";
        return ReplayPath() + "/" +
               Services::FileService::SanitizeReplayFileNameComponent(playerId, 80) + "-" +
               Services::FileService::SanitizeReplayFileNameComponent(songHash, 80) + "-" +
               Services::FileService::SanitizeReplayFileNameComponent(difficulty, 40) + "-" +
               Services::FileService::SanitizeReplayFileNameComponent(characteristic, 80) + ".dat";
    }

    bool SettingsService::ShowScorePP() const
    {
        return Data::Private::Settings::showScorePP;
    }

    void SettingsService::SetShowScorePP(bool value)
    {
        Data::Private::Settings::showScorePP = value;
    }

    bool SettingsService::ShowLocalPlayerRank() const
    {
        return Data::Private::Settings::showLocalPlayerRank;
    }

    void SettingsService::SetShowLocalPlayerRank(bool value)
    {
        Data::Private::Settings::showLocalPlayerRank = value;
    }

    bool SettingsService::HideNAScoresFromLeaderboard() const
    {
        return Data::Private::Settings::hideNAScoresFromLeaderboard;
    }

    void SettingsService::SetHideNAScoresFromLeaderboard(bool value)
    {
        Data::Private::Settings::hideNAScoresFromLeaderboard = value;
    }

    std::string SettingsService::LocationFilterMode() const
    {
        return Data::Private::Settings::locationFilterMode;
    }

    void SettingsService::SetLocationFilterMode(std::string value)
    {
        Data::Private::Settings::locationFilterMode = std::move(value);
    }

    bool SettingsService::EnableCountryLeaderboards() const
    {
        return Data::Private::Settings::enableCountryLeaderboards;
    }

    void SettingsService::SetEnableCountryLeaderboards(bool value)
    {
        Data::Private::Settings::enableCountryLeaderboards = value;
    }

    bool SettingsService::SaveLocalReplays() const
    {
        return Data::Private::Settings::saveLocalReplays;
    }

    void SettingsService::SetSaveLocalReplays(bool value)
    {
        Data::Private::Settings::saveLocalReplays = value;
    }

    bool SettingsService::ReplayOverrideHandedness() const
    {
        return Data::Private::Settings::replayOverrideHandedness;
    }

    void SettingsService::SetReplayOverrideHandedness(bool value)
    {
        Data::Private::Settings::replayOverrideHandedness = value;
    }

    bool SettingsService::UseRecordedPlayerSettings() const
    {
        return Data::Private::Settings::useRecordedPlayerSettings;
    }

    void SettingsService::SetUseRecordedPlayerSettings(bool value)
    {
        Data::Private::Settings::useRecordedPlayerSettings = value;
    }

    bool SettingsService::ShareHsvProfiles() const
    {
        return Data::Private::Settings::shareHsvProfiles;
    }

    void SettingsService::SetShareHsvProfiles(bool value)
    {
        Data::Private::Settings::shareHsvProfiles = value;
    }

    bool SettingsService::PublicLivePresenceOptOut() const
    {
        return Data::Private::Settings::publicLivePresenceOptOut;
    }

    void SettingsService::SetPublicLivePresenceOptOut(bool value)
    {
        Data::Private::Settings::publicLivePresenceOptOut = value;
    }

    bool SettingsService::LiveChatOverlayEnabled() const
    {
        return Data::Private::Settings::liveChatOverlayEnabled;
    }

    void SettingsService::SetLiveChatOverlayEnabled(bool value)
    {
        Data::Private::Settings::liveChatOverlayEnabled = value;
    }

    bool SettingsService::LiveChatOverlayGameplayEnabled() const
    {
        return Data::Private::Settings::liveChatOverlayGameplayEnabled;
    }

    void SettingsService::SetLiveChatOverlayGameplayEnabled(bool value)
    {
        Data::Private::Settings::liveChatOverlayGameplayEnabled = value;
    }

    float SettingsService::LiveChatOverlayScale() const
    {
        return Data::Private::Settings::liveChatOverlayScale;
    }

    void SettingsService::SetLiveChatOverlayScale(float value)
    {
        Data::Private::Settings::liveChatOverlayScale = value;
    }

    float SettingsService::LiveChatOverlayTextScale() const
    {
        return Data::Private::Settings::liveChatOverlayTextScale;
    }

    void SettingsService::SetLiveChatOverlayTextScale(float value)
    {
        Data::Private::Settings::liveChatOverlayTextScale = value;
    }
}
