#pragma once

#include "Features/Players/Domain/GlobalPlayerPage.hpp"
#include "Features/Players/Services/GameSessionService.hpp"

#include <beatsaber-hook/shared/utils/typedefs.h>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Players::Services, GlobalPlayerQueryService, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Players::Services::GameSessionService*, _gameSessionService);
    DECLARE_CTOR(ctor, SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService);

  public:
    SnoreSaber::Data::GlobalPlayerPage GetPlayerPage(SnoreSaber::Data::GlobalPlayerScope scope, int page);
};
