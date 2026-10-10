#pragma once

#include "Core/BeatSaver/BeatSaverModels.hpp"

#include <custom-types/shared/macros.hpp>

#include <string>

// blocking BeatSaver api client; call off the main thread
DECLARE_CLASS_CODEGEN(SnoreSaber::Core::BeatSaver, BeatSaverService, System::Object) {
    DECLARE_CTOR(ctor);
public:
    BeatSaverMap GetMapByHash(const std::string& hash);
    BeatSaverMap GetMapById(const std::string& id);
    void DownloadMapByHash(const std::string& hash, const BeatSaverVersion* version);
    const BeatSaverVersion* SelectVersion(const BeatSaverMap* map, const std::string& hash);
    const BeatSaverDifficulty* SelectDifficulty(const BeatSaverVersion* version, const std::string& difficulty);
    static std::string NormalizeHash(const std::string& hash);
    static std::string NormalizeDifficulty(const std::string& difficulty);
};
