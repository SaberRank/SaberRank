#include "Features/Live/Ludus/Services/LudusInstalledMods.hpp"

#include "logging.hpp"

#include <scotland2/shared/loader.hpp>

#include <algorithm>
#include <cctype>
#include <set>
#include <string>

namespace SnoreSaber::Features::Live::Ludus::Services::LudusInstalledMods
{
    namespace
    {
        std::string Normalize(std::string value)
        {
            value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char c) { return !std::isspace(c); }));
            value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), value.end());
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
            return value;
        }
    }

    std::vector<::SnoreSaber::Live::V1::LiveMod> List()
    {
        try
        {
            // pc lists ipa EnabledPlugins; scotland2's early mods + mods are the analog (libs excluded)
            std::set<std::string> ids;
            for (const auto& mod : modloader::get_loaded())
            {
                if (mod.phase != modloader::LoadPhase::EarlyMods && mod.phase != modloader::LoadPhase::Mods)
                {
                    continue;
                }

                auto id = Normalize(mod.info.id);
                if (!id.empty())
                {
                    ids.insert(std::move(id));
                }
            }

            std::vector<::SnoreSaber::Live::V1::LiveMod> mods;
            mods.reserve(ids.size());
            for (const auto& id : ids)
            {
                mods.push_back(::SnoreSaber::Live::V1::LiveMod{.Id = id});
            }

            return mods;
        }
        catch (const std::exception& ex)
        {
            WARN("Failed to collect installed mods for Ludus: {}", ex.what());
            return {};
        }
    }
}
