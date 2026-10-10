#pragma once

#include "Features/Leaderboards/UI/ScoreDetails/ScoreInfoModal.hpp"
#include "Features/Replays/ReplayLoader.hpp"
#include <UnityEngine/MonoBehaviour.hpp>
#include <custom-types/shared/macros.hpp>

#include "Features/Leaderboards/Domain/ScoreMap.hpp"
#include <vector>

DECLARE_CLASS_CODEGEN(SnoreSaber::CustomTypes::Components, LeaderboardScoreInfoButtonHandler, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(UnityW<SnoreSaber::UI::Other::ScoreInfoModal>, scoreInfoModal);

    public:
    void Setup(SnoreSaber::ReplaySystem::ReplayLoader* replayLoader);
    void set_scoreCollection(std::vector<SnoreSaber::Data::ScoreMap> scores);
    void ShowScoreInfoModal(int buttonIdx);

    private:
    std::vector<SnoreSaber::Data::ScoreMap> scores;
};
