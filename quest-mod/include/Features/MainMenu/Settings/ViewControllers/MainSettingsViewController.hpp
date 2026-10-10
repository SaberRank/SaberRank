#pragma once

#include "Core/Configuration/SettingsService.hpp"
#include <HMUI/ViewController.hpp>
#include <TMPro/TextMeshProUGUI.hpp>

#include <beatsaber-hook/shared/utils/typedefs.h>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::UI::ViewControllers, MainSettingsViewController, HMUI::ViewController) {
                      DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Core::Configuration::SettingsService*, _settingsService);
                      DECLARE_INSTANCE_METHOD(bool, get_showScorePP);
                      DECLARE_INSTANCE_METHOD(void, set_showScorePP, bool value);
                      DECLARE_INSTANCE_METHOD(bool, get_showLocalPlayerRank);
                      DECLARE_INSTANCE_METHOD(void, set_showLocalPlayerRank, bool value);
                      DECLARE_INSTANCE_METHOD(bool, get_hideNAScores);
                      DECLARE_INSTANCE_METHOD(void, set_hideNAScores, bool value);
                      DECLARE_INSTANCE_METHOD(StringW, get_locationFilterMode);
                      DECLARE_INSTANCE_METHOD(void, set_locationFilterMode, StringW value);
                      DECLARE_INSTANCE_METHOD(bool, get_enableCountryLeaderboards);
                      DECLARE_INSTANCE_METHOD(void, set_enableCountryLeaderboards, bool value);
                      DECLARE_INSTANCE_METHOD(bool, get_saveLocalReplays);
                      DECLARE_INSTANCE_METHOD(void, set_saveLocalReplays, bool value);
                      DECLARE_INSTANCE_METHOD(bool, get_replayOverrideHandedness);
                      DECLARE_INSTANCE_METHOD(void, set_replayOverrideHandedness, bool value);
                      DECLARE_INSTANCE_METHOD(bool, get_useRecordedPlayerSettings);
                      DECLARE_INSTANCE_METHOD(void, set_useRecordedPlayerSettings, bool value);
                      DECLARE_INSTANCE_METHOD(bool, get_shareHsvProfiles);
                      DECLARE_INSTANCE_METHOD(void, set_shareHsvProfiles, bool value);
                      DECLARE_INSTANCE_METHOD(bool, get_publicLivePresenceEnabled);
                      DECLARE_INSTANCE_METHOD(void, set_publicLivePresenceEnabled, bool value);
                      DECLARE_INSTANCE_METHOD(bool, get_liveChatOverlayEnabled);
                      DECLARE_INSTANCE_METHOD(void, set_liveChatOverlayEnabled, bool value);
                      DECLARE_INSTANCE_METHOD(bool, get_liveChatOverlayGameplayEnabled);
                      DECLARE_INSTANCE_METHOD(void, set_liveChatOverlayGameplayEnabled, bool value);
                      DECLARE_INSTANCE_METHOD(float, get_liveChatOverlayScale);
                      DECLARE_INSTANCE_METHOD(void, set_liveChatOverlayScale, float value);
                      DECLARE_INSTANCE_METHOD(float, get_liveChatOverlayTextScale);
                      DECLARE_INSTANCE_METHOD(void, set_liveChatOverlayTextScale, float value);

                      DECLARE_INSTANCE_METHOD(void, SignOut);
                      DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, accountStatusText);

                      DECLARE_INSTANCE_FIELD_PRIVATE_DEFAULT(ListW<StringW>, locationFilterOptions, ListW<StringW>::New());

                      DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);

                      DECLARE_CTOR(ctor);

private:
                      SnoreSaber::Core::Configuration::SettingsService* GetSettingsService();
                      void RefreshAccountStatus();
};
