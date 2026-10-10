#include "Features/Leaderboards/UI/ScoreDetails/LeaderboardScoreInfoButtonHandler.hpp"
#include "Utils/UIUtils.hpp"

#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Transform.hpp>

#include <utility>

#include "Sprites.hpp"
#include "logging.hpp"

DEFINE_TYPE(SnoreSaber::CustomTypes::Components, LeaderboardScoreInfoButtonHandler);

using namespace UnityEngine;
using namespace UnityEngine::UI;

namespace SnoreSaber::CustomTypes::Components
{
    void LeaderboardScoreInfoButtonHandler::Setup(SnoreSaber::ReplaySystem::ReplayLoader* replayLoader)
    {
        scoreInfoModal = SnoreSaber::UI::Other::ScoreInfoModal::Create(transform, replayLoader);
    }

    void LeaderboardScoreInfoButtonHandler::set_scoreCollection(std::vector<SnoreSaber::Data::ScoreMap> _scores)
    {
        scores = std::move(_scores);
    }

    void LeaderboardScoreInfoButtonHandler::ShowScoreInfoModal(int buttonIdx)
    {
        if (scoreInfoModal && buttonIdx < scores.size())
        {
            scoreInfoModal->Show(scores.at(buttonIdx));
        }
    }
}
