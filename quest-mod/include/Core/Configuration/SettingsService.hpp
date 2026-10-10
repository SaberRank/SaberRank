#pragma once

#include <GlobalNamespace/BeatmapKey.hpp>
#include <System/zzzz__Object_def.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Core::Configuration, SettingsService, System::Object) {
    DECLARE_CTOR(ctor);

  public:
    void Load();
    void Save();

    std::string DataPath() const;
    std::string ConfigPath() const;
    std::string ReplayPath() const;
    std::string ReplayPathFor(const std::string& playerId, const std::string& songHash, GlobalNamespace::BeatmapKey beatmapKey) const;

    bool ShowScorePP() const;
    void SetShowScorePP(bool value);
    bool ShowLocalPlayerRank() const;
    void SetShowLocalPlayerRank(bool value);
    bool HideNAScoresFromLeaderboard() const;
    void SetHideNAScoresFromLeaderboard(bool value);
    std::string LocationFilterMode() const;
    void SetLocationFilterMode(std::string value);
    bool EnableCountryLeaderboards() const;
    void SetEnableCountryLeaderboards(bool value);
    bool SaveLocalReplays() const;
    void SetSaveLocalReplays(bool value);
    bool ReplayOverrideHandedness() const;
    void SetReplayOverrideHandedness(bool value);
    bool UseRecordedPlayerSettings() const;
    void SetUseRecordedPlayerSettings(bool value);
    bool ShareHsvProfiles() const;
    void SetShareHsvProfiles(bool value);
    bool PublicLivePresenceOptOut() const;
    void SetPublicLivePresenceOptOut(bool value);
    bool LiveChatOverlayEnabled() const;
    void SetLiveChatOverlayEnabled(bool value);
    bool LiveChatOverlayGameplayEnabled() const;
    void SetLiveChatOverlayGameplayEnabled(bool value);
    float LiveChatOverlayScale() const;
    void SetLiveChatOverlayScale(float value);
    float LiveChatOverlayTextScale() const;
    void SetLiveChatOverlayTextScale(float value);
};
