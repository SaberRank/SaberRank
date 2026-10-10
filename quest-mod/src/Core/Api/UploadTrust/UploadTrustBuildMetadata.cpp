#include "Core/Api/UploadTrust/UploadTrustBuildMetadata.hpp"

#ifndef SNORESABER_OFFICIAL_BUILD_ID
#define SNORESABER_OFFICIAL_BUILD_ID ""
#endif

#ifndef SNORESABER_OFFICIAL_BUILD_CREDENTIAL
#define SNORESABER_OFFICIAL_BUILD_CREDENTIAL ""
#endif

#ifndef SNORESABER_OFFICIAL_ARTIFACT_SHA256
#define SNORESABER_OFFICIAL_ARTIFACT_SHA256 ""
#endif

#ifndef SNORESABER_DEVELOPMENT_UPLOAD_TOKEN
#define SNORESABER_DEVELOPMENT_UPLOAD_TOKEN ""
#endif

#ifndef SNORESABER_DEVELOPMENT_AUTH_NONCE
#define SNORESABER_DEVELOPMENT_AUTH_NONCE ""
#endif

#ifndef SNORESABER_DEVELOPMENT_PLAYER_ID
#define SNORESABER_DEVELOPMENT_PLAYER_ID ""
#endif

#ifndef SNORESABER_DEVELOPMENT_PLAYER_NAME
#define SNORESABER_DEVELOPMENT_PLAYER_NAME ""
#endif

namespace SnoreSaber::Core::Api::UploadTrust
{
    bool UploadTrustBuildMetadata::IsOfficial() const
    {
        return !buildId.empty() && !credential.empty();
    }

    bool UploadTrustBuildMetadata::IsDevelopment() const
    {
        return !IsOfficial() && !developmentUploadToken.empty();
    }

    bool UploadTrustBuildMetadata::HasDevelopmentAuth() const
    {
        return !developmentAuthNonce.empty();
    }

    UploadTrustBuildMetadata UploadTrustBuildMetadata::FromBuildConfig()
    {
        return {
            SNORESABER_OFFICIAL_BUILD_ID,
            SNORESABER_OFFICIAL_BUILD_CREDENTIAL,
            SNORESABER_OFFICIAL_ARTIFACT_SHA256,
            SNORESABER_DEVELOPMENT_UPLOAD_TOKEN,
            SNORESABER_DEVELOPMENT_AUTH_NONCE,
            SNORESABER_DEVELOPMENT_PLAYER_ID,
            SNORESABER_DEVELOPMENT_PLAYER_NAME,
        };
    }
}
