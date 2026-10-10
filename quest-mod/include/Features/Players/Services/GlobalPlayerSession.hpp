#pragma once

#include "Features/Players/Domain/GlobalPlayerScope.hpp"

#include <beatsaber-hook/shared/utils/typedefs.h>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Players::Services, GlobalPlayerSession, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(int, _scope);
    DECLARE_INSTANCE_FIELD_PRIVATE(int, _page);
    DECLARE_INSTANCE_FIELD_PRIVATE(int, _requestId);
    DECLARE_CTOR(ctor);

  public:
    SnoreSaber::Data::GlobalPlayerScope GetScope() const;
    int GetPage() const;
    int BeginRequest();
    bool IsCurrentRequest(int requestId) const;
    bool SelectScope(SnoreSaber::Data::GlobalPlayerScope scope);
    void MovePage(bool down);
};
