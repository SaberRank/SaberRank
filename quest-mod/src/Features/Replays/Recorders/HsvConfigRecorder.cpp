#include "Features/Replays/Recorders/HsvConfigRecorder.hpp"

#include "Features/Replays/Format/HsvReplayConfigCodec.hpp"
#include "logging.hpp"

#include <beatsaber-hook/shared/config/rapidjson-utils.hpp>
#include <beatsaber-hook/shared/utils/utils-functions.h>

#include <filesystem>
#include <string>

using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Recorders, HsvConfigRecorder);

namespace SnoreSaber::ReplaySystem::Recorders
{
    namespace
    {
        constexpr int MaxSelectorBytes = 8 * 1024;
        // quest HitScoreVisualizer stores its config-utils config here; "selectedConfig" holds
        // the full path of the active config file ("" means the built-in default)
        const std::string SelectorPath = "/sdcard/ModData/com.beatgames.beatsaber/Configs/HitScoreVisualizer.json";
        const std::string ConfigRoot = "/sdcard/ModData/com.beatgames.beatsaber/Mods/HitScoreVisualizer";

        std::string SelectedHsvConfigPath()
        {
            std::error_code error;
            if (!std::filesystem::is_regular_file(SelectorPath, error) || std::filesystem::file_size(SelectorPath, error) > MaxSelectorBytes)
            {
                return std::string();
            }

            std::string selectorJson = readfile(SelectorPath);
            rapidjson::Document document;
            document.Parse(selectorJson.data(), selectorJson.size());
            if (document.HasParseError() || !document.IsObject())
            {
                return std::string();
            }

            auto member = document.FindMember("selectedConfig");
            if (member == document.MemberEnd() || !member->value.IsString())
            {
                return std::string();
            }

            std::string selectedPath(member->value.GetString(), member->value.GetStringLength());
            if (selectedPath.empty())
            {
                return std::string();
            }

            // only read files inside the HSV config directory
            std::string fullConfigPath = std::filesystem::path(selectedPath).lexically_normal().string();
            if (!fullConfigPath.starts_with(ConfigRoot + "/"))
            {
                return std::string();
            }

            return fullConfigPath;
        }
    } // namespace

    void HsvConfigRecorder::ctor()
    {
        INVOKE_CTOR();
    }

    std::vector<char> HsvConfigRecorder::Export()
    {
        try
        {
            std::string selectedPath = SelectedHsvConfigPath();
            if (selectedPath.empty())
            {
                return {};
            }

            std::error_code error;
            if (!std::filesystem::is_regular_file(selectedPath, error) || std::filesystem::file_size(selectedPath, error) > HsvReplayConfigCodec::MaxJsonBytes)
            {
                return {};
            }

            std::string configJson = readfile(selectedPath);
            std::vector<char> payload;
            std::string failure;
            if (!HsvReplayConfigCodec::TryEncodeJson(configJson, payload, failure))
            {
                INFO("Skipping HSV replay config: {}", failure);
                return {};
            }

            return payload;
        }
        catch (const std::exception& exception)
        {
            INFO("Failed to record HSV config: {}", exception.what());
            return {};
        }
    }
} // namespace SnoreSaber::ReplaySystem::Recorders
