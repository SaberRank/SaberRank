#include "Features/MainMenu/MainFlow/GlobalLeaderboard/GlobalLeaderboardTableData.hpp"
#include "Features/MainMenu/MainFlow/GlobalLeaderboard/GlobalLeaderboardTableCell.hpp"
#include "Features/Players/Domain/GlobalPlayerScope.hpp"
#include "Features/Players/Services/GlobalPlayerQueryService.hpp"
#include "Features/Players/Services/GlobalPlayerSession.hpp"
#include <HMUI/ScrollView.hpp>
#include <HMUI/Touchable.hpp>
#include <System/Action_1.hpp>
#include <UnityEngine/Networking/DownloadHandler.hpp>
#include <UnityEngine/Networking/UnityWebRequest.hpp>
#include <UnityEngine/RectOffset.hpp>
#include <UnityEngine/WaitForSeconds.hpp>
#include <bsml/shared/Helpers/getters.hpp>
#include <bsml/shared/BSML/SharedCoroutineStarter.hpp>
#include "Utils/AsyncUtils.hpp"
#include "Utils/SafePtr.hpp"
#include "logging.hpp"
#include "static.hpp"

#include "Features/Players/Domain/Player.hpp"
#include "Features/MainMenu/MainFlow/GlobalLeaderboard/GlobalViewController.hpp"
#include "Utils/StrippedMethods.hpp"

#include <utility>

DEFINE_TYPE(SnoreSaber::CustomTypes::Components, GlobalLeaderboardTableData);

using namespace SnoreSaber::CustomTypes::Components;
using namespace UnityEngine::UI;
using namespace UnityEngine::Networking;
using namespace TMPro;
using namespace SnoreSaber;

#define BeginCoroutine(method) BSML::SharedCoroutineStarter::StartCoroutine(custom_types::Helpers::CoroutineHelper::New(method))

std::vector<Data::Player> playerCollection;

namespace
{
    Data::GlobalPlayerScope GlobalScopeFor(GlobalLeaderboardTableData::LeaderboardType leaderboardType)
    {
        switch (leaderboardType)
        {
            case GlobalLeaderboardTableData::LeaderboardType::AroundYou:
                return Data::GlobalPlayerScope::AroundPlayer;
            case GlobalLeaderboardTableData::LeaderboardType::Friends:
                return Data::GlobalPlayerScope::Friends;
            case GlobalLeaderboardTableData::LeaderboardType::Country:
                return Data::GlobalPlayerScope::Country;
            default:
                return Data::GlobalPlayerScope::Global;
        }
    }

    SnoreSaber::Features::Players::Services::GlobalPlayerSession* GetGlobalPlayerSession()
    {
        return BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Features::Players::Services::GlobalPlayerSession*>();
    }

    bool IsCurrentRequest(GlobalLeaderboardTableData* self, SnoreSaber::Features::Players::Services::GlobalPlayerSession* session, int requestId)
    {
        return session ? session->IsCurrentRequest(requestId) : self->activeRequestId == requestId;
    }
}

namespace SnoreSaber::CustomTypes::Components
{
    void GlobalLeaderboardTableData::ctor()
    {
        page = 1;
        activeRequestId = 0;
        reuseIdentifier = "CustomPlayerCellList";
        leaderboardType = Global;
    }

    float GlobalLeaderboardTableData::CellSize()
    {
        return 12.0f;
    }

    int GlobalLeaderboardTableData::NumberOfCells()
    {
        // if we have less than 50 players in the playerCollection for SOME reason, this will make sure that if we reach the end of the list it won't overextend
        int size = playerCollection.size();
        return size < 5 ? size : 5;
    }

    void GlobalLeaderboardTableData::set_LeaderboardType(LeaderboardType type)
    {
        leaderboardType = type;
        auto session = GetGlobalPlayerSession();
        if (session)
        {
            session->SelectScope(GlobalScopeFor(type));
            page = session->GetPage();
        }
        else
        {
            page = 1;
        }
        StartRefresh();
    }

    void GlobalLeaderboardTableData::DownButtonWasPressed()
    {
        auto session = GetGlobalPlayerSession();
        if (session)
        {
            session->MovePage(true);
            page = session->GetPage();
        }
        else
        {
            page++;
        }
        StartRefresh();
    }

    void GlobalLeaderboardTableData::UpButtonWasPressed()
    {
        auto session = GetGlobalPlayerSession();
        int currentPage = session ? session->GetPage() : page;
        if (currentPage <= 1)
        {
            return;
        }

        if (session)
        {
            session->MovePage(false);
            page = session->GetPage();
        }
        else
        {
            page--;
        }
        StartRefresh();
    }

    void GlobalLeaderboardTableData::StartRefresh()
    {
        StartCoroutine(custom_types::Helpers::CoroutineHelper::New(Refresh()));
    }

    /// really just here because of the way it reloads twice by doing ReloadData and then RefreshCells(true, true), now it's combined
    void ReloadTableViewData(HMUI::TableView* self)
    {
        if (!self->_isInitialized)
        {
            self->LazyInit();
        }
        auto visibleCells = self->_visibleCells;
        int length = visibleCells->Count;
        for (int i = 0; i < length; i++)
        {
            auto tableCell = visibleCells->Item[i];
            tableCell->gameObject->SetActive(false);
            self->AddCellToReusableCells(tableCell);
        }
        self->_visibleCells->Clear();
        if (self->dataSource)
        {
            self->UpdateCachedData();
            self->_cellSize = self->dataSource->CellSize(-1);
        }
        else
        {
            self->_numberOfCells = 0;
            self->_cellSize = 1.0f;
        }

        self->scrollView->_fixedCellSize = self->cellSize + self->spacing;
        self->RefreshContentSize();
        if (!self->gameObject->activeInHierarchy)
        {
            self->_refreshCellsOnEnable = true;
        }
        else
        {
            self->RefreshCells(true, true);
        }

        if (self->didReloadDataEvent)
        {
            self->didReloadDataEvent->Invoke(self);
        }
    }

    custom_types::Helpers::Coroutine GlobalLeaderboardTableData::Refresh()
    {
        auto session = GetGlobalPlayerSession();
        Data::GlobalPlayerScope scope = session ? session->GetScope() : GlobalScopeFor(leaderboardType);
        int requestPage = session ? session->GetPage() : page;
        int localRequestId = 0;
        if (session)
        {
            localRequestId = session->BeginRequest();
        }
        else
        {
            activeRequestId = activeRequestId + 1;
            localRequestId = activeRequestId;
        }
        page = requestPage;

        isLoading = true;
        playerCollection.clear();
        ReloadTableViewData(tableView);
        auto ourGlobalViewController = globalViewController.cast<SnoreSaber::UI::ViewControllers::GlobalViewController>();
        ourGlobalViewController->set_loading(true);

        auto queryService = BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Features::Players::Services::GlobalPlayerQueryService*>();
        if (!queryService)
        {
            isLoading = false;
            ourGlobalViewController->set_loading(false);
            co_return;
        }

        SafePtrUnity<GlobalLeaderboardTableData> self(this);
        SafePtr<SnoreSaber::Features::Players::Services::GlobalPlayerQueryService> queryServiceSafe(queryService);
        SafePtr<SnoreSaber::Features::Players::Services::GlobalPlayerSession> sessionSafe;
        bool hasSession = session;
        if (hasSession)
        {
            sessionSafe = session;
        }

        SnoreSaber::Utils::Async::RunThenMain(
            [queryServiceSafe, scope, requestPage] {
                return queryServiceSafe->GetPlayerPage(scope, requestPage).players;
            },
            [self, sessionSafe, hasSession, localRequestId](std::vector<Data::Player> players) {
                auto session = hasSession ? sessionSafe.ptr() : nullptr;
                if (!self || !IsCurrentRequest(self.ptr(), session, localRequestId))
                {
                    return;
                }

                playerCollection = std::move(players);
                self->initialized = true;
                ReloadTableViewData(self->tableView.ptr());
                self->isLoading = false;

                auto viewController = self->globalViewController.cast<SnoreSaber::UI::ViewControllers::GlobalViewController>();
                viewController->set_loading(false);
            });

        co_return;
    }

    HMUI::TableCell* GlobalLeaderboardTableData::CellForIdx(HMUI::TableView* tableView, int idx)
    {
        auto cell = tableView->DequeueReusableCellForIdentifier(reuseIdentifier);

        UnityW<GlobalLeaderboardTableCell> playerCell = cell ? cell.cast<GlobalLeaderboardTableCell>() : nullptr;

        if (!playerCell)
        {
            playerCell = GlobalLeaderboardTableCell::CreateCell();
            playerCell->playerProfileModal = playerProfileModal;
            // playerCell->transform->SetParent(tableView->transform->GetChild(0)->GetChild(0), false);
        }

        playerCell->reuseIdentifier = reuseIdentifier;
        if (initialized)
        {
            playerCell->Refresh(playerCollection[idx], leaderboardType);
        }
        return playerCell;
    }
} // namespace SnoreSaber::CustomTypes::Components
