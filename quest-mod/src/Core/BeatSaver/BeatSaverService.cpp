#include "Core/BeatSaver/BeatSaverService.hpp"

#include "Utils/WebUtils.hpp"
#include "logging.hpp"

#include <songcore/shared/SongCore.hpp>
#include <zip/shared/zip.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>
#include <stdexcept>
#include <thread>
#include <vector>

DEFINE_TYPE(SnoreSaber::Core::BeatSaver, BeatSaverService);

namespace SnoreSaber::Core::BeatSaver
{
    namespace
    {
        constexpr int DownloadAttemptCount = 3;
        constexpr int DownloadRetryDelayMs = 750;
        constexpr long DownloadTimeoutSeconds = 45;
        constexpr long DownloadStallTimeoutSeconds = 30;
        constexpr long RequestTimeoutSeconds = 30;

        struct BeatSaverDownloadException : std::runtime_error
        {
            bool retryable;

            BeatSaverDownloadException(const std::string& message, bool retryable)
                : std::runtime_error(message), retryable(retryable) {}
        };

        bool EqualsIgnoreCase(const std::string& left, const std::string& right)
        {
            return left.size() == right.size() &&
                   std::equal(left.begin(), left.end(), right.begin(), [](unsigned char a, unsigned char b) {
                       return std::tolower(a) == std::tolower(b);
                   });
        }

        std::string ToLower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
            return value;
        }

        std::string Trim(const std::string& value)
        {
            auto begin = value.find_first_not_of(" \t\r\n");
            if (begin == std::string::npos)
            {
                return std::string();
            }
            auto end = value.find_last_not_of(" \t\r\n");
            return value.substr(begin, end - begin + 1);
        }

        bool IsBeatSaverHash(const std::string& hash)
        {
            return hash.size() == 40 && std::all_of(hash.begin(), hash.end(), [](unsigned char c) { return std::isxdigit(c); });
        }

        std::string RandomSuffix()
        {
            std::random_device device;
            std::mt19937_64 engine(((uint64_t)device() << 32) ^ device());
            char suffix[33];
            snprintf(suffix, sizeof(suffix), "%016llx%016llx", (unsigned long long)engine(), (unsigned long long)engine());
            return std::string(suffix);
        }

        void TryDelete(const std::string& path)
        {
            std::error_code error;
            std::filesystem::remove(path, error);
            if (error)
            {
                INFO("Unable to delete BeatSaver map zip: {}", error.message());
            }
        }

        void TryDeleteDirectory(const std::string& path)
        {
            std::error_code error;
            if (!std::filesystem::exists(path, error))
            {
                return;
            }
            std::filesystem::remove_all(path, error);
            if (error)
            {
                INFO("Unable to delete BeatSaver map folder: {}", error.message());
            }
        }

        void TryRestoreDirectory(const std::string& backupPath, const std::string& destinationPath)
        {
            std::error_code error;
            if (backupPath.empty() || std::filesystem::exists(destinationPath, error) || !std::filesystem::exists(backupPath, error))
            {
                return;
            }

            std::filesystem::rename(backupPath, destinationPath, error);
            if (error)
            {
                INFO("Unable to restore previous BeatSaver map folder: {}", error.message());
            }
        }

        std::vector<std::string> BuildDownloadUrls(const std::string& songUrl, const std::string& lowerHash)
        {
            std::vector<std::string> urls;
            auto addUrl = [&urls](const std::string& url) {
                std::string trimmed = Trim(url);
                if (trimmed.empty())
                {
                    return;
                }
                if (std::any_of(urls.begin(), urls.end(), [&trimmed](const std::string& existing) { return EqualsIgnoreCase(existing, trimmed); }))
                {
                    return;
                }
                urls.push_back(std::move(trimmed));
            };

            addUrl(songUrl);
            addUrl("https://cdn.beatsaver.com/" + lowerHash + ".zip");
            return urls;
        }

        void DownloadZipToFile(const std::string& url, const std::string& zipPath)
        {
            auto [httpCode, curlError] = WebUtils::DownloadFileSync(url, zipPath, DownloadTimeoutSeconds, DownloadStallTimeoutSeconds);
            if (!curlError.empty())
            {
                // stalls, timeouts, and connection failures all surface as curl errors
                throw BeatSaverDownloadException(curlError, true);
            }
            if (httpCode < 200 || httpCode >= 300)
            {
                std::string message = httpCode > 0 ? "HTTP " + std::to_string(httpCode) : "download request failed";
                bool retryable = httpCode == 0 || httpCode == 408 || httpCode == 429 || httpCode >= 500;
                throw BeatSaverDownloadException(message, retryable);
            }
        }

        void EnsureDownloadedZip(const std::string& zipPath)
        {
            std::error_code error;
            if (!std::filesystem::is_regular_file(zipPath, error) || std::filesystem::file_size(zipPath, error) == 0)
            {
                throw BeatSaverDownloadException("downloaded zip was empty", true);
            }
        }

        void ExtractZip(const std::string& zipPath, const std::string& tempSongPath)
        {
            int openError = 0;
            zip_t* archive = zip_open(zipPath.c_str(), ZIP_RDONLY, &openError);
            if (!archive)
            {
                throw BeatSaverDownloadException("failed to open downloaded zip", true);
            }

            int extractedFiles = 0;
            std::string rootPath = std::filesystem::path(tempSongPath).lexically_normal().string();
            if (!rootPath.ends_with('/'))
            {
                rootPath += '/';
            }

            try
            {
                zip_int64_t entryCount = zip_get_num_entries(archive, 0);
                for (zip_int64_t index = 0; index < entryCount; ++index)
                {
                    const char* entryName = zip_get_name(archive, index, 0);
                    if (!entryName)
                    {
                        throw BeatSaverDownloadException("failed to read zip entry name", true);
                    }

                    std::string name(entryName);
                    std::string destinationPath = (std::filesystem::path(tempSongPath) / name).lexically_normal().string();
                    if (!destinationPath.starts_with(rootPath))
                    {
                        throw BeatSaverDownloadException("downloaded zip contained an unsafe path", false);
                    }

                    if (name.ends_with('/'))
                    {
                        std::filesystem::create_directories(destinationPath);
                        continue;
                    }

                    std::filesystem::create_directories(std::filesystem::path(destinationPath).parent_path());

                    zip_file_t* entry = zip_fopen_index(archive, index, 0);
                    if (!entry)
                    {
                        throw BeatSaverDownloadException("failed to read zip entry", true);
                    }

                    std::ofstream output(destinationPath, std::ios::binary | std::ios::trunc);
                    char buffer[64 * 1024];
                    zip_int64_t read;
                    while ((read = zip_fread(entry, buffer, sizeof(buffer))) > 0)
                    {
                        output.write(buffer, read);
                    }
                    zip_fclose(entry);

                    if (read < 0 || !output)
                    {
                        throw BeatSaverDownloadException("failed to extract zip entry", true);
                    }
                    extractedFiles++;
                }
            }
            catch (...)
            {
                zip_discard(archive);
                throw;
            }
            zip_discard(archive);

            if (extractedFiles == 0)
            {
                throw BeatSaverDownloadException("downloaded zip did not contain map files", true);
            }
        }

        bool ShouldRetry(const std::exception& exception)
        {
            auto downloadException = dynamic_cast<const BeatSaverDownloadException*>(&exception);
            if (downloadException)
            {
                return downloadException->retryable;
            }
            // filesystem errors are the quest analog of PC's retryable IOException
            return dynamic_cast<const std::filesystem::filesystem_error*>(&exception);
        }

        void DownloadAndExtractMap(const std::vector<std::string>& urls, const std::string& zipPath, const std::string& tempSongPath)
        {
            std::string lastError;

            for (const std::string& url : urls)
            {
                for (int attempt = 1; attempt <= DownloadAttemptCount; attempt++)
                {
                    TryDelete(zipPath);
                    TryDeleteDirectory(tempSongPath);

                    try
                    {
                        DownloadZipToFile(url, zipPath);
                        EnsureDownloadedZip(zipPath);
                        std::filesystem::create_directories(tempSongPath);
                        ExtractZip(zipPath, tempSongPath);
                        TryDelete(zipPath);
                        return;
                    }
                    catch (const std::exception& exception)
                    {
                        lastError = exception.what();
                        INFO("BeatSaver map download failed from {} (attempt {}/{}): {}", url, attempt, DownloadAttemptCount, lastError);
                        TryDelete(zipPath);
                        TryDeleteDirectory(tempSongPath);

                        if (attempt >= DownloadAttemptCount || !ShouldRetry(exception))
                        {
                            break;
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(DownloadRetryDelayMs * attempt));
                    }
                }
            }

            throw std::runtime_error("Failed to download BeatSaver map zip: " + lastError);
        }

        void ReplaceDirectory(const std::string& sourcePath, const std::string& destinationPath)
        {
            std::string backupPath;
            bool replaced = false;

            if (std::filesystem::exists(destinationPath))
            {
                backupPath = destinationPath + "." + RandomSuffix() + ".backup";
                std::filesystem::rename(destinationPath, backupPath);
            }

            try
            {
                std::filesystem::rename(sourcePath, destinationPath);
                replaced = true;
            }
            catch (...)
            {
                TryRestoreDirectory(backupPath, destinationPath);
                throw;
            }

            if (replaced && !backupPath.empty())
            {
                TryDeleteDirectory(backupPath);
            }
        }

        BeatSaverMap FetchMap(const std::string& url)
        {
            auto [httpCode, response] = WebUtils::GetSync(url, RequestTimeoutSeconds);
            if (httpCode < 200 || httpCode >= 300)
            {
                std::string message = httpCode > 0 ? "HTTP " + std::to_string(httpCode) : "request failed";
                throw std::runtime_error("BeatSaver request failed: " + message);
            }

            auto map = BeatSaverMap::TryParse(response);
            if (!map.has_value())
            {
                throw std::runtime_error("failed to parse BeatSaver map response");
            }
            return std::move(map.value());
        }
    } // namespace

    void BeatSaverService::ctor()
    {
        INVOKE_CTOR();
    }

    BeatSaverMap BeatSaverService::GetMapByHash(const std::string& hash)
    {
        std::string normalizedHash = NormalizeHash(hash);
        if (normalizedHash.empty())
        {
            throw std::invalid_argument("BeatSaver hash is required");
        }

        return FetchMap("https://api.beatsaver.com/maps/hash/" + ToLower(normalizedHash));
    }

    BeatSaverMap BeatSaverService::GetMapById(const std::string& id)
    {
        std::string trimmedId = Trim(id);
        if (trimmedId.empty())
        {
            throw std::invalid_argument("BeatSaver id is required");
        }

        return FetchMap("https://api.beatsaver.com/maps/id/" + trimmedId);
    }

    void BeatSaverService::DownloadMapByHash(const std::string& hash, const BeatSaverVersion* version)
    {
        std::string normalizedHash = NormalizeHash(hash);
        if (normalizedHash.empty())
        {
            throw std::invalid_argument("BeatSaver hash is required");
        }

        if (!IsBeatSaverHash(normalizedHash))
        {
            throw std::invalid_argument("BeatSaver hash must be a 40-character SHA1");
        }

        std::string lowerHash = ToLower(normalizedHash);
        std::string songUrl = !version || version->downloadUrl.empty()
            ? "https://cdn.beatsaver.com/" + lowerHash + ".zip"
            : version->downloadUrl;
        std::filesystem::path customSongsPath = SongCore::API::Loading::GetPreferredCustomLevelPath();
        std::string customSongPath = (customSongsPath / lowerHash).string();
        std::string tempRootPath = (customSongsPath / (lowerHash + "." + RandomSuffix() + ".download")).string();
        std::string tempSongPath = tempRootPath + "/song";
        std::string zipPath = tempRootPath + "/" + lowerHash + ".zip";

        try
        {
            std::filesystem::create_directories(customSongsPath);
            std::filesystem::create_directories(tempRootPath);

            DownloadAndExtractMap(BuildDownloadUrls(songUrl, lowerHash), zipPath, tempSongPath);
            ReplaceDirectory(tempSongPath, customSongPath);
        }
        catch (...)
        {
            TryDelete(zipPath);
            TryDeleteDirectory(tempRootPath);
            throw;
        }

        TryDelete(zipPath);
        TryDeleteDirectory(tempRootPath);
    }

    const BeatSaverVersion* BeatSaverService::SelectVersion(const BeatSaverMap* map, const std::string& hash)
    {
        if (!map || map->versions.empty())
        {
            return nullptr;
        }

        std::string normalizedHash = NormalizeHash(hash);
        for (const BeatSaverVersion& version : map->versions)
        {
            if (EqualsIgnoreCase(version.hash, normalizedHash))
            {
                return &version;
            }
        }
        return &map->versions.front();
    }

    const BeatSaverDifficulty* BeatSaverService::SelectDifficulty(const BeatSaverVersion* version, const std::string& difficulty)
    {
        if (!version || version->diffs.empty())
        {
            return nullptr;
        }

        std::string normalizedDifficulty = NormalizeDifficulty(difficulty);
        for (const BeatSaverDifficulty& diff : version->diffs)
        {
            if (EqualsIgnoreCase(NormalizeDifficulty(diff.difficulty), normalizedDifficulty))
            {
                return &diff;
            }
        }
        return &version->diffs.front();
    }

    std::string BeatSaverService::NormalizeHash(const std::string& hash)
    {
        return Trim(hash);
    }

    std::string BeatSaverService::NormalizeDifficulty(const std::string& difficulty)
    {
        std::string normalized = difficulty;
        std::string::size_type position = 0;
        while ((position = normalized.find('+', position)) != std::string::npos)
        {
            normalized.replace(position, 1, "Plus");
            position += 4;
        }
        return normalized;
    }
} // namespace SnoreSaber::Core::BeatSaver
