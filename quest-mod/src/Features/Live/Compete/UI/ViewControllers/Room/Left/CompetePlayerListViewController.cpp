#include "Features/Live/Compete/UI/ViewControllers/Room/Left/CompetePlayerListViewController.hpp"

#include "Features/Live/Compete/UI/Cells/CompetePlayerCell.hpp"
#include "assets.hpp"

#include <HMUI/VerticalScrollIndicator.hpp>
#include <UnityEngine/RectTransform.hpp>
#include <UnityEngine/Vector2.hpp>
#include <bsml/shared/BSML.hpp>
#include <bsml/shared/Helpers/getters.hpp>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::UI::ViewControllers::Room::Left, CompetePlayerListViewController);

namespace SnoreSaber::Features::Live::Compete::UI::ViewControllers::Room::Left
{
    namespace
    {
        constexpr float ScrollbarWidth = 8.0f;
        const Domain::CompeteTeam FallbackTeamOne = {"team1", "Team 1"};
        const Domain::CompeteTeam FallbackTeamTwo = {"team2", "Team 2"};

        void ReloadList(BSML::CustomCellListTableData* list)
        {
            if (!list || !list->tableView)
            {
                return;
            }

            list->tableView->ReloadData();
            list->tableView->ClearSelection();
        }
    }

    void CompetePlayerListViewController::ctor()
    {
        INVOKE_CTOR();
        _teamOne = FallbackTeamOne;
        _teamTwo = FallbackTeamTwo;
    }

    void CompetePlayerListViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            _parser = BSML::parse_and_construct(IncludedAssets::CompetePlayerListViewController_bsml, transform, this);
        }

        ApplyTeamNames();
        ApplyVisibility();
        ReloadPlayers();
    }

    void CompetePlayerListViewController::SetRoom(const Domain::CompeteRoom& room)
    {
        if (!_materials)
        {
            _materials = BSML::Helpers::GetDiContainer()->TryResolve<Core::Presentation::SnoreSaberUIMaterials*>();
        }

        players->Clear();
        teamOnePlayers->Clear();
        teamTwoPlayers->Clear();

        _teamMode = room.playerListMode == Domain::CompetePlayerListMode::Teams;
        if (room.teams.size() > 0)
        {
            _teamOne = room.teams[0];
        }
        else
        {
            _teamOne = FallbackTeamOne;
        }

        if (room.teams.size() > 1)
        {
            _teamTwo = room.teams[1];
        }
        else
        {
            _teamTwo = FallbackTeamTwo;
        }

        for (auto& player : room.players)
        {
            auto cell = Cells::CompetePlayerCell::Create(player, _materials, [this](const std::string& playerId, const std::string& name) { ShowProfile(playerId, name); });
            if (_teamMode)
            {
                if (player.teamId == _teamOne.id)
                {
                    teamOnePlayers->Add(cell);
                }
                else
                {
                    teamTwoPlayers->Add(cell);
                }

                continue;
            }

            players->Add(cell);
        }

        _hasPlayers = !room.players.empty();
        ApplyTeamNames();
        ApplyVisibility();
        ReloadPlayers();
    }

    void CompetePlayerListViewController::ShowProfile(const std::string& playerId, const std::string& name)
    {
        if (playerId.empty())
        {
            return;
        }

        // quest modal fetches and presents its own loading state; pc pre-seeds the
        // profile view with the clicked name before awaiting the fetch
        if (!profileModal)
        {
            profileModal = ::SnoreSaber::UI::Other::PlayerProfileModal::Create(transform);
        }

        profileModal->Show(playerId);
    }

    void CompetePlayerListViewController::ApplyTeamNames()
    {
        if (teamOneNameText)
        {
            teamOneNameText->text = _teamOne.name;
        }

        if (teamTwoNameText)
        {
            teamTwoNameText->text = _teamTwo.name;
        }
    }

    void CompetePlayerListViewController::ApplyVisibility()
    {
        if (playerList)
        {
            playerList->gameObject->SetActive(_hasPlayers && !_teamMode);
        }

        if (teamPlayersObject)
        {
            teamPlayersObject->SetActive(_hasPlayers && _teamMode);
        }

        if (emptyStateObject)
        {
            emptyStateObject->SetActive(!_hasPlayers);
        }
    }

    void CompetePlayerListViewController::ReloadPlayers()
    {
        ReloadList(playerList.unsafePtr());
        ReloadList(teamOnePlayerList.unsafePtr());
        ReloadList(teamTwoPlayerList.unsafePtr());
        MoveTeamOneScrollbarToLeft();
    }

    void CompetePlayerListViewController::MoveTeamOneScrollbarToLeft()
    {
        if (!teamOnePlayerList)
        {
            return;
        }

        auto indicator = teamOnePlayerList->GetComponentInChildren<HMUI::VerticalScrollIndicator*>(true);
        if (!indicator)
        {
            return;
        }

        auto parent = indicator->transform->get_parent();
        if (!parent)
        {
            return;
        }

        auto scrollbar = il2cpp_utils::try_cast<UnityEngine::RectTransform>(parent.unsafePtr()).value_or(nullptr);
        if (!scrollbar)
        {
            return;
        }

        scrollbar->anchorMin = UnityEngine::Vector2(0.0f, 0.0f);
        scrollbar->anchorMax = UnityEngine::Vector2(0.0f, 1.0f);
        scrollbar->offsetMin = UnityEngine::Vector2(-ScrollbarWidth, 0.0f);
        scrollbar->offsetMax = UnityEngine::Vector2::get_zero();
    }
}
