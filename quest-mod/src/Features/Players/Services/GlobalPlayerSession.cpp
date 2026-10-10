#include "Features/Players/Services/GlobalPlayerSession.hpp"

DEFINE_TYPE(SnoreSaber::Features::Players::Services, GlobalPlayerSession);

namespace SnoreSaber::Features::Players::Services
{
    void GlobalPlayerSession::ctor()
    {
        INVOKE_CTOR();
        _scope = static_cast<int>(SnoreSaber::Data::GlobalPlayerScope::Global);
        _page = 1;
        _requestId = 0;
    }

    SnoreSaber::Data::GlobalPlayerScope GlobalPlayerSession::GetScope() const
    {
        return static_cast<SnoreSaber::Data::GlobalPlayerScope>(_scope);
    }

    int GlobalPlayerSession::GetPage() const
    {
        return _page;
    }

    int GlobalPlayerSession::BeginRequest()
    {
        _requestId++;
        return _requestId;
    }

    bool GlobalPlayerSession::IsCurrentRequest(int requestId) const
    {
        return _requestId == requestId;
    }

    bool GlobalPlayerSession::SelectScope(SnoreSaber::Data::GlobalPlayerScope scope)
    {
        int scopeValue = static_cast<int>(scope);
        if (_scope == scopeValue)
        {
            return false;
        }

        _scope = scopeValue;
        _page = 1;
        return true;
    }

    void GlobalPlayerSession::MovePage(bool down)
    {
        if (down)
        {
            _page++;
            return;
        }

        if (_page > 1)
        {
            _page--;
        }
    }
}
