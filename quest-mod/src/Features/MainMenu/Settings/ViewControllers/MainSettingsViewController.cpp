#include "Features/MainMenu/Settings/ViewControllers/MainSettingsViewController.hpp"

#include "Features/Live/Ludus/Services/LudusSessionService.hpp"
#include "Features/Players/Services/DevicePairingService.hpp"
#include "Features/Players/Services/GameSessionService.hpp"

#include <bsml/shared/BSML.hpp>
#include <bsml/shared/Helpers/getters.hpp>
#include "assets.hpp"

#include <algorithm>

DEFINE_TYPE(SnoreSaber::UI::ViewControllers, MainSettingsViewController);

namespace SnoreSaber::UI::ViewControllers {
    bool MainSettingsViewController::get_showScorePP() {
        return GetSettingsService()->ShowScorePP();
    }

    void MainSettingsViewController::set_showScorePP(bool value) {
        GetSettingsService()->SetShowScorePP(value);
    }

    bool MainSettingsViewController::get_showLocalPlayerRank() {
        return GetSettingsService()->ShowLocalPlayerRank();
    }

    void MainSettingsViewController::set_showLocalPlayerRank(bool value) {
        GetSettingsService()->SetShowLocalPlayerRank(value);
    }

    bool MainSettingsViewController::get_hideNAScores() {
        return GetSettingsService()->HideNAScoresFromLeaderboard();
    }

    void MainSettingsViewController::set_hideNAScores(bool value) {
        GetSettingsService()->SetHideNAScoresFromLeaderboard(value);
    }

    StringW MainSettingsViewController::get_locationFilterMode() {
        return GetSettingsService()->LocationFilterMode();
    }

    void MainSettingsViewController::set_locationFilterMode(StringW value) {
        GetSettingsService()->SetLocationFilterMode((std::string)value);
    }

    bool MainSettingsViewController::get_enableCountryLeaderboards() {
        return GetSettingsService()->EnableCountryLeaderboards();
    }

    void MainSettingsViewController::set_enableCountryLeaderboards(bool value) {
        GetSettingsService()->SetEnableCountryLeaderboards(value);
    }

    bool MainSettingsViewController::get_saveLocalReplays() {
        return GetSettingsService()->SaveLocalReplays();
    }

    void MainSettingsViewController::set_saveLocalReplays(bool value) {
        GetSettingsService()->SetSaveLocalReplays(value);
    }

    bool MainSettingsViewController::get_replayOverrideHandedness() {
        return GetSettingsService()->ReplayOverrideHandedness();
    }

    void MainSettingsViewController::set_replayOverrideHandedness(bool value) {
        GetSettingsService()->SetReplayOverrideHandedness(value);
    }

    bool MainSettingsViewController::get_useRecordedPlayerSettings() {
        return GetSettingsService()->UseRecordedPlayerSettings();
    }

    void MainSettingsViewController::set_useRecordedPlayerSettings(bool value) {
        GetSettingsService()->SetUseRecordedPlayerSettings(value);
    }

    bool MainSettingsViewController::get_shareHsvProfiles() {
        return GetSettingsService()->ShareHsvProfiles();
    }

    void MainSettingsViewController::set_shareHsvProfiles(bool value) {
        GetSettingsService()->SetShareHsvProfiles(value);
    }

    bool MainSettingsViewController::get_publicLivePresenceEnabled() {
        return !GetSettingsService()->PublicLivePresenceOptOut();
    }

    void MainSettingsViewController::set_publicLivePresenceEnabled(bool value) {
        bool optOut = !value;
        auto settings = GetSettingsService();
        if (settings->PublicLivePresenceOptOut() == optOut) {
            return;
        }

        settings->SetPublicLivePresenceOptOut(optOut);
        // the ludus session lives in the app container; only bound once the live feature installs
        auto container = BSML::Helpers::GetDiContainer();
        auto ludusSession = container ? container->TryResolve<SnoreSaber::Features::Live::Ludus::Services::LudusSessionService*>() : nullptr;
        if (ludusSession) {
            ludusSession->ApplyPublicLivePresencePreference();
        }
    }

    bool MainSettingsViewController::get_liveChatOverlayEnabled() {
        return GetSettingsService()->LiveChatOverlayEnabled();
    }

    void MainSettingsViewController::set_liveChatOverlayEnabled(bool value) {
        GetSettingsService()->SetLiveChatOverlayEnabled(value);
    }

    bool MainSettingsViewController::get_liveChatOverlayGameplayEnabled() {
        return GetSettingsService()->LiveChatOverlayGameplayEnabled();
    }

    void MainSettingsViewController::set_liveChatOverlayGameplayEnabled(bool value) {
        GetSettingsService()->SetLiveChatOverlayGameplayEnabled(value);
    }

    float MainSettingsViewController::get_liveChatOverlayScale() {
        return GetSettingsService()->LiveChatOverlayScale();
    }

    void MainSettingsViewController::set_liveChatOverlayScale(float value) {
        GetSettingsService()->SetLiveChatOverlayScale(std::clamp(value, 0.85f, 1.75f));
    }

    float MainSettingsViewController::get_liveChatOverlayTextScale() {
        return GetSettingsService()->LiveChatOverlayTextScale();
    }

    void MainSettingsViewController::set_liveChatOverlayTextScale(float value) {
        GetSettingsService()->SetLiveChatOverlayTextScale(std::clamp(value, 0.9f, 1.8f));
    }

    void MainSettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation) {
            BSML::parse_and_construct(IncludedAssets::MainSettingsViewController_bsml, transform, this);
        }

        RefreshAccountStatus();
    }

    void MainSettingsViewController::SignOut()
    {
        SnoreSaber::Features::Players::Services::DevicePairing::SignOut();
        RefreshAccountStatus();
    }

    void MainSettingsViewController::RefreshAccountStatus()
    {
        if (!accountStatusText) {
            return;
        }

        auto container = BSML::Helpers::GetDiContainer();
        auto gameSession = container ? container->TryResolve<SnoreSaber::Features::Players::Services::GameSessionService*>() : nullptr;
        std::string playerId = gameSession ? gameSession->GetLocalPlayerId() : "";
        if (playerId.empty()) {
            accountStatusText->text = "<color=#fc8181>Not signed in</color>";
            return;
        }

        StringW playerName = gameSession->GetLocalPlayerName();
        std::string name = playerName ? static_cast<std::string>(playerName) : "";
        accountStatusText->text = name.empty()
            ? "<color=#89fc81>Signed in</color>"
            : "<color=#89fc81>Signed in as " + name + "</color>";
    }

    void MainSettingsViewController::ctor() {
        INVOKE_CTOR();
        locationFilterOptions->Add(StringW("Country"));
        locationFilterOptions->Add(StringW("Region"));
    }

    SnoreSaber::Core::Configuration::SettingsService* MainSettingsViewController::GetSettingsService()
    {
        if (!_settingsService)
        {
            _settingsService = BSML::Helpers::GetDiContainer()->Resolve<SnoreSaber::Core::Configuration::SettingsService*>();
        }

        return _settingsService;
    }
}
