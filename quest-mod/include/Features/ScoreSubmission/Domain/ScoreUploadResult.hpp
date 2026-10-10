#pragma once

#include "Core/Api/SnoreSaberApiError.hpp"

#include <optional>
#include <string>
#include <utility>

namespace SnoreSaber::Data::Private
{
    enum class ScoreUploadStatus
    {
        Packaging,
        Uploading,
        Success,
        Retrying,
        Error,
        Done,
    };

    struct ScoreUploadResult
    {
        ScoreUploadStatus status = ScoreUploadStatus::Error;
        bool success = false;
        std::string message;
        std::optional<SnoreSaber::Core::Api::SnoreSaberApiError> error;

        static ScoreUploadResult Success(std::string message = "Score uploaded!")
        {
            ScoreUploadResult result;
            result.status = ScoreUploadStatus::Success;
            result.success = true;
            result.message = std::move(message);
            return result;
        }

        static ScoreUploadResult Failure(std::string message, std::optional<SnoreSaber::Core::Api::SnoreSaberApiError> error = std::nullopt)
        {
            ScoreUploadResult result;
            result.status = ScoreUploadStatus::Error;
            result.success = false;
            result.message = std::move(message);
            result.error = std::move(error);
            return result;
        }
    };

    struct ScoreSubmissionStatus
    {
        ScoreUploadStatus status = ScoreUploadStatus::Error;
        std::string message;

        static ScoreSubmissionStatus Progress(ScoreUploadStatus status, std::string message)
        {
            ScoreSubmissionStatus result;
            result.status = status;
            result.message = std::move(message);
            return result;
        }

        static ScoreSubmissionStatus FromResult(const ScoreUploadResult& uploadResult)
        {
            return Progress(uploadResult.status, uploadResult.message);
        }
    };
}
