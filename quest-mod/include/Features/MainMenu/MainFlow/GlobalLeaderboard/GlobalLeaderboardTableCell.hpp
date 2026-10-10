#pragma once

#include "Features/MainMenu/MainFlow/GlobalLeaderboard/GlobalLeaderboardTableData.hpp"
#include "Features/Players/Domain/Player.hpp"
#include "Features/Players/Profile/PlayerProfileModal.hpp"
#include <HMUI/ImageView.hpp>
#include <HMUI/TableCell.hpp>
#include <HMUI/TableView.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <UnityEngine/Coroutine.hpp>
#include <UnityEngine/MonoBehaviour.hpp>
#include <beatsaber-hook/shared/config/rapidjson-utils.hpp>
#include <bsml/shared/BSML/Components/Backgroundable.hpp>
#include <custom-types/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::CustomTypes::Components, GlobalLeaderboardTableCell, HMUI::TableCell) {
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, rank);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, pp);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, country);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, weekly);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, name);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ImageView>, flag);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ImageView>, profile);
    DECLARE_INSTANCE_FIELD(UnityW<BSML::Backgroundable>, bg);
    DECLARE_INSTANCE_FIELD(UnityEngine::Coroutine*, profileRoutine);
    DECLARE_INSTANCE_FIELD(UnityEngine::Coroutine*, flagRoutine);
    DECLARE_INSTANCE_FIELD(SnoreSaber::UI::Other::PlayerProfileModal*, playerProfileModal);

    DECLARE_CTOR(ctor);

public:
    std::string playerId;
    static GlobalLeaderboardTableCell * CreateCell();
    void Refresh(SnoreSaber::Data::Player& player, SnoreSaber::CustomTypes::Components::GlobalLeaderboardTableData::LeaderboardType leaderboardType);
    void stopProfileRoutine();
    void stopFlagRoutine();
    void OpenPlayerProfileModal();
};