#pragma once

#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Core, SnoreSaberRuntimeInfo, Il2CppObject) {
    DECLARE_CTOR(ctor);

  public:
    static void InitializeGameVersion();
    static std::string PluginVersion();
    static std::string GameVersion();
    static std::string BuildUploadVersionHash();

    std::string UploadVersionHash() const;
};
