#pragma once
#include "Features/Replays/Recorders/MainRecorder.hpp"
#include <optional>
#include <vector>

namespace SnoreSaber::Services::ReplayService
{
    struct ReplaySerializationResult
    {
        std::vector<char> replay;
        float failTime = 0.0f;
    };

    extern std::vector<char> CurrentSerializedReplay;
    extern std::function<void(const std::vector<char>&)> ReplaySerialized;

    void OnSoftRestart();

    void NewPlayStarted(ReplaySystem::Recorders::MainRecorder* _recorder);
    void DiscardReplay();
    std::optional<ReplaySerializationResult> WriteSerializedReplay();
}
