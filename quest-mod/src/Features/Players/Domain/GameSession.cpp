#include "Features/Players/Domain/GameSession.hpp"

namespace SnoreSaber::Data
{
    bool GameSession::IsAuthenticated() const
    {
        return !sessionId.empty() && !sessionKey.empty();
    }

    bool GameSession::UsesUploadProtocolV2() const
    {
        return uploadTrust.has_value() && uploadTrust->IsUploadProtocolV2();
    }
}
