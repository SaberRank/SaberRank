#pragma once

#include <HMUI/ImageView.hpp>
#include <HMUI/HoverHint.hpp>
#include <HMUI/ModalView.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <UnityEngine/MonoBehaviour.hpp>
#include <UnityEngine/Transform.hpp>
#include <UnityEngine/UI/GridLayoutGroup.hpp>
#include <custom-types/shared/coroutine.hpp>
#include <custom-types/shared/macros.hpp>
#include <string_view>

#include <UnityEngine/Coroutine.hpp>

#include "Features/Players/Domain/Badge.hpp"
#include "Features/Players/Domain/Player.hpp"
#include "Features/Players/Profile/ProfileDetailData.hpp"

DECLARE_CLASS_CODEGEN(SnoreSaber::UI::Other, PlayerProfileModal, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ModalView>, modal);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ImageView>, pfpImage);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ImageView>, prefixImage);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::HoverHint>, prefixHoverHint);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::UI::GridLayoutGroup>, badgeParent);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, headerText);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, globalRanking);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, performancePoints);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, averageRankedAccuracy);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, totalScore);
    DECLARE_INSTANCE_FIELD(UnityEngine::Coroutine*, profileRoutine);
    DECLARE_INSTANCE_FIELD(ListW<UnityEngine::Coroutine*>, badgeRoutines);

public:

    std::string playerId;
    int profileRequestId = 0;
    static SnoreSaber::UI::Other::PlayerProfileModal * Create(UnityEngine::Transform * parent);
    void Show(std::string playerId);

    void Hide();
    void Setup();
    void set_player(std::u16string_view playername);
    void set_header(std::u16string_view header);
    void set_globalRanking(int globalRanking);
    void set_performancePoints(float performancePoints);
    void set_averageRankedAccuracy(float averageRankedAccuracy);
    void set_totalScore(long totalScore);
    void set_pfp(UnityEngine::Sprite* pfp);
    void ClearBadges();
    void AddBadge(SnoreSaber::Data::Badge& badge, int index);
    void AddBadge(SnoreSaber::UI::Other::ProfileBadgeData const& badge, int index);

    custom_types::Helpers::Coroutine FetchPlayerData(std::string playerId);
    void SetPlayerData(SnoreSaber::Data::Player& player);
    void SetProfileData(SnoreSaber::UI::Other::ProfileDetailData const& player);
    void stopProfileRoutine();
    void stopBadgeRoutines();

    void OpenPlayerUrl();

private:
    void ApplyCrown(SnoreSaber::UI::Other::ProfileCrownData const& crown);
    void ApplyProfileFont(SnoreSaber::UI::Other::ProfileDetailData const& player);
    void ResetProfileFont();

    bool isFurryFontApplied = false;
};
