#pragma once

#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <vector>

DECLARE_CLASS_CODEGEN(SnoreSaber::ReplaySystem::Recorders, HsvConfigRecorder, System::Object) {
    DECLARE_CTOR(ctor);
public:
    std::vector<char> Export();
};
