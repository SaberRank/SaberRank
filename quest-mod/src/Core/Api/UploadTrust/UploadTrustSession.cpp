#include "Core/Api/UploadTrust/UploadTrustSession.hpp"

namespace SnoreSaber::Core::Api::UploadTrust
{
    bool UploadTrustSession::IsUploadProtocolV2() const
    {
        return (!requiresBuildId || !buildId.empty()) && !buildCredential.empty() && !uploadVersionHash.empty();
    }
}
