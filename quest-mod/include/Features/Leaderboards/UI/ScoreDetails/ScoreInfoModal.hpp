#pragma once
#include "Features/Leaderboards/Domain/Score.hpp"
#include "Features/Leaderboards/Domain/ScoreMap.hpp"
#include "Features/Leaderboards/UI/ScoreDetails/ScoreDetailData.hpp"
#include "Features/Replays/ReplayLoader.hpp"
#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <HMUI/HoverHint.hpp>
#include <HMUI/ImageView.hpp>
#include <HMUI/ModalView.hpp>
#include "Features/Players/Profile/PlayerProfileModal.hpp"
#include <UnityEngine/MonoBehaviour.hpp>
#include <custom-types/shared/macros.hpp>
#include <bsml/shared/BSML-Lite.hpp>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::UI::Other, ScoreInfoModal, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ModalView>, modal);
    DECLARE_INSTANCE_FIELD(UnityW<PlayerProfileModal>, playerProfileModal);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ImageView>, prefixImage);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::HoverHint>, prefixHoverHint);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, player);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, deviceHmd);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, deviceControllerLeft);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, deviceControllerRight);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, score);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, pp);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, combo);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, fullCombo);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, badCuts);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, missedNotes);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, modifiers);
    DECLARE_INSTANCE_FIELD(UnityW<TMPro::TextMeshProUGUI>, timeSet);
    DECLARE_INSTANCE_FIELD(UnityW<BSML::ClickableImage>, replayImage);
    DECLARE_INSTANCE_FIELD(GlobalNamespace::BeatmapLevel*, currentBeatmapLevel);
    DECLARE_INSTANCE_FIELD(GlobalNamespace::BeatmapKey, currentBeatmapKey);

public:

    void Hide();
    void Show(SnoreSaber::Data::ScoreMap& score);

    static SnoreSaber::UI::Other::ScoreInfoModal * Create(UnityEngine::Transform * parent, SnoreSaber::ReplaySystem::ReplayLoader* replayLoader);

    void Setup(SnoreSaber::ReplaySystem::ReplayLoader* replayLoader);

private:
    int leaderboardId;
    SnoreSaber::Data::Score currentScore;
    std::string playerId;
    std::string replayFileName;
    bool replayEnabled;
    int replayRequestId;
    SnoreSaber::ReplaySystem::ReplayLoader* replayLoader;

    void ShowPlayerProfileModal();
    void PlayReplay();
    void SetReplayButtonState(bool enabled);
    void SetScoreInfo(SnoreSaber::UI::Other::ScoreDetailData const& score);
    void ApplyCrown(SnoreSaber::UI::Other::ScoreDetailData const& score);
};
