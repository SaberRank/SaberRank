#include "Core/SnoreSaberRuntimeInfo.hpp"

#include "Utils/md5.h"

#include <UnityEngine/Application.hpp>

DEFINE_TYPE(SnoreSaber::Core, SnoreSaberRuntimeInfo);

namespace SnoreSaber::Core
{
    namespace
    {
        std::string gameVersion;

        std::string NormalizeGameVersion(std::string value)
        {
            size_t packageSuffix = value.find('_');
            if (packageSuffix != std::string::npos)
            {
                value.resize(packageSuffix);
            }

            return value;
        }
    }

    void SnoreSaberRuntimeInfo::ctor()
    {
        INVOKE_CTOR();
    }

    void SnoreSaberRuntimeInfo::InitializeGameVersion()
    {
        gameVersion = NormalizeGameVersion((std::string)UnityEngine::Application::get_version());
    }

    std::string SnoreSaberRuntimeInfo::PluginVersion()
    {
        return VERSION;
    }

    std::string SnoreSaberRuntimeInfo::GameVersion()
    {
        return gameVersion;
    }

    std::string SnoreSaberRuntimeInfo::BuildUploadVersionHash()
    {
        return md5(std::string("Quest") + PluginVersion() + GameVersion());
    }

    std::string SnoreSaberRuntimeInfo::UploadVersionHash() const
    {
        return BuildUploadVersionHash();
    }
}
