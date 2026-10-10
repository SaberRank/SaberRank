#include "Services/ReplayService.hpp"

#include "Features/Replays/Format/ReplayFile.hpp"
#include "Features/Replays/ReplayFileCodec.hpp"
#include "Features/Replays/Recorders/MainRecorder.hpp"
#include "logging.hpp"

using namespace SnoreSaber::Data::Private;

namespace SnoreSaber::Services::ReplayService
{
    vector<char> CurrentSerializedReplay;
    function<void(const vector<char>&)> ReplaySerialized;
    SafePtr<ReplaySystem::Recorders::MainRecorder> recorder;

    void OnSoftRestart() {
        CurrentSerializedReplay.clear();
        ReplaySerialized = nullptr;
        recorder = nullptr;
    }

    void NewPlayStarted(ReplaySystem::Recorders::MainRecorder* _recorder) {
        recorder = _recorder;
    }

    namespace {
        void ClearRecorder(ReplaySystem::Recorders::MainRecorder* expected) {
            if (!recorder || recorder.ptr() != expected)
            {
                return;
            }

            recorder = nullptr;
        }
    }

    void DiscardReplay()
    {
        if (!recorder)
        {
            return;
        }

        INFO("Discarding replay");
        auto current = recorder.ptr();
        current->StopRecording();
        ClearRecorder(current);
    }

    std::optional<ReplaySerializationResult> WriteSerializedReplay()
    {
        if (!recorder)
        {
            INFO("Skipping replay write because no recorder is active");
            CurrentSerializedReplay.clear();
            return std::nullopt;
        }

        auto current = recorder.ptr();
        current->StopRecording();
        auto replay = current->ExportCurrentReplay();
        if (!replay)
        {
            ERROR("Skipping replay write because export returned no replay");
            CurrentSerializedReplay.clear();
            ClearRecorder(current);
            return std::nullopt;
        }

        float failTime = replay->metadata->FailTime;
        try
        {
            CurrentSerializedReplay = SnoreSaber::ReplaySystem::ReplayFileCodec::Write(replay);
        }
        catch (...)
        {
            ClearRecorder(current);
            throw;
        }
        ClearRecorder(current);

        if (CurrentSerializedReplay.empty())
        {
            INFO("Replay serialization failed");
            return std::nullopt;
        }

        if (ReplaySerialized)
            ReplaySerialized(CurrentSerializedReplay);

        return ReplaySerializationResult {CurrentSerializedReplay, failTime};
    }
}
