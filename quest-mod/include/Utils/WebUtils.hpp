#pragma once
#include <beatsaber-hook/shared/config/rapidjson-utils.hpp>
#include <custom-types/shared/coroutine.hpp>

#include <HMUI/ImageView.hpp>

#include <functional>
#include <string>
#include <tuple>
#include <vector>

// matches libcurl's public typedef so curl.h stays out of this header
typedef void CURL;

namespace WebUtils
{
    extern std::string cookie;

    /// @brief applies the shared tls hardening (bundled mozilla ca store) to a curl easy handle
    void ApplyTransportOptions(CURL* curl);
    std::tuple<long, std::string> GetSync(std::string url, long timeout);
    std::tuple<long, std::string> GetSync(std::string url, long timeout, const std::vector<std::string>& requestHeaders);
    void GetAsync(std::string url, std::function<void(long, std::string)> finished, bool pureCppCallback = false);
    void GetAsync(std::string url, long timeout, std::function<void(long, std::string)> finished, bool pureCppCallback = false);

    std::tuple<long, std::string> PostJsonSync(std::string url, std::string jsonData, long timeout);

    std::tuple<long, std::string> PostWithReplaySync(std::string url, const std::vector<char> &replayData, std::string postData, long timeout, const std::vector<std::string>& requestHeaders);

    /// @brief gets texture @ url and applies it to out->set_sprite() after downloading
    custom_types::Helpers::Coroutine WaitForImageDownload(std::string url, HMUI::ImageView* out);
    /// @brief gets gif @ url and applies it's first frame to out->set_sprite() after downloading
    custom_types::Helpers::Coroutine WaitForGifDownload(std::string url, HMUI::ImageView* out);

    long DownloadReplaySync(std::string url, std::vector<char> &replayData, long timeOut);

    /// @brief downloads url straight to a file; aborts when no data flows for stallSeconds
    /// @return http code and curl error message (empty = transfer succeeded)
    std::tuple<long, std::string> DownloadFileSync(std::string url, const std::string& filePath, long timeout, long stallSeconds);

} // namespace WebUtils
