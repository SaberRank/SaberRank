#include "Features/Players/Services/GlobalPlayerQueryService.hpp"

#include "Core/Api/SnoreSaberApiClient.hpp"
#include "Features/Players/Domain/PlayerListQuery.hpp"
#include "logging.hpp"

DEFINE_TYPE(SnoreSaber::Features::Players::Services, GlobalPlayerQueryService);

namespace SnoreSaber::Features::Players::Services
{
    namespace
    {
        SnoreSaber::Data::PlayerQueryScope QueryScopeFor(SnoreSaber::Data::GlobalPlayerScope scope)
        {
            switch (scope)
            {
                case SnoreSaber::Data::GlobalPlayerScope::AroundPlayer:
                    return SnoreSaber::Data::PlayerQueryScope::AroundPlayer;
                case SnoreSaber::Data::GlobalPlayerScope::Friends:
                    return SnoreSaber::Data::PlayerQueryScope::Friends;
                case SnoreSaber::Data::GlobalPlayerScope::Country:
                    return SnoreSaber::Data::PlayerQueryScope::Country;
                case SnoreSaber::Data::GlobalPlayerScope::Region:
                    return SnoreSaber::Data::PlayerQueryScope::Region;
                default:
                    return SnoreSaber::Data::PlayerQueryScope::Global;
            }
        }

        SnoreSaber::Data::PlayerListQuery BuildQuery(SnoreSaber::Data::GlobalPlayerScope scope, int page)
        {
            SnoreSaber::Data::PlayerListQuery query;
            query.page = page;
            query.limit = scope == SnoreSaber::Data::GlobalPlayerScope::AroundPlayer ? 6 : 5;
            query.scope = QueryScopeFor(scope);
            return query;
        }
    }

    void GlobalPlayerQueryService::ctor(GameSessionService* gameSessionService)
    {
        INVOKE_CTOR();
        _gameSessionService = gameSessionService;
    }

    SnoreSaber::Data::GlobalPlayerPage GlobalPlayerQueryService::GetPlayerPage(SnoreSaber::Data::GlobalPlayerScope scope, int page)
    {
        SnoreSaber::Data::GlobalPlayerPage playerPage;
        playerPage.scope = scope;
        playerPage.page = page;

        try
        {
            SnoreSaber::Core::Api::SnoreSaberApiClient apiClient;
            SnoreSaber::Data::PlayerListQuery query = BuildQuery(scope, page);
            std::optional<SnoreSaber::Data::GameSession> session = _gameSessionService->GetGameSession();
            playerPage.players = apiClient.GetPlayers(query, session.has_value() ? &session.value() : nullptr).items;
        }
        catch (const std::exception& exception)
        {
            ERROR("Failed to load SnoreSaber global players: {:s}", exception.what());
        }

        return playerPage;
    }
}
