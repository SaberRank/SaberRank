#pragma once

#include "Utils/SafePtr.hpp"
#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <string>

namespace SnoreSaber::Features::Live::Compete::Domain
{
    struct CompeteSongSelection
    {
        // rooted so the Boehm GC keeps them alive while a room holds the selection
        SafePtr<GlobalNamespace::BeatmapLevel> beatmapLevel;
        SafeValueType<GlobalNamespace::BeatmapKey> beatmapKey;
        std::string name;
        std::string mapper;
        std::string difficulty;
        std::string characteristic;
        std::string coverSource;
        std::string duration;
        std::string bpm;
        std::string nps;
        std::string notes;
        std::string obstacles;
        std::string bombs;
        std::string njs;
        std::string jumpDistance;
        std::string stars;
        std::string mapHash;
        std::string downloadUrl;
    };
}
