#pragma once

#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::ScoreSubmission::Services, ScoreSubmissionRegistry, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(bool, _enabled);
    DECLARE_CTOR(ctor);

  public:
    bool IsEnabled() const;
    void SetEnabled(bool enabled);
};
