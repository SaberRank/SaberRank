#pragma once
#include <paper2_scotland2/shared/logger.hpp>

namespace SnoreSaber
{
    namespace Logging {
        constexpr auto Logger = Paper::ConstLoggerContext("SnoreSaber");
    }
}
#define INFO(...) ::SnoreSaber::Logging::Logger.info(__VA_ARGS__)
#define WARN(...) ::SnoreSaber::Logging::Logger.warn(__VA_ARGS__)
#define ERROR(...) ::SnoreSaber::Logging::Logger.error(__VA_ARGS__)
#define CRITICAL(...) ::SnoreSaber::Logging::Logger.critical(__VA_ARGS__)