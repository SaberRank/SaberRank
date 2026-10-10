#pragma once

#include "Features/Players/Domain/Player.hpp"

#include <beatsaber-hook/shared/utils/typedefs.h>
#include <custom-types/shared/macros.hpp>
#include <functional>
#include <lapiz/shared/macros.hpp>
#include <optional>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Players::Services, PlayerProfileService, Il2CppObject) {
    DECLARE_CTOR(ctor);

  public:
    std::optional<SnoreSaber::Data::Player> GetPlayerInfo(const std::string& playerId, bool full);
    void GetPlayerInfoAsync(std::string playerId, bool full, std::function<void(std::optional<SnoreSaber::Data::Player>)> finished);
};
