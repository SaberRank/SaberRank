#include "Features/Live/Compete/Services/CompeteGameplayState.hpp"

#include <cctype>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::Services, CompeteGameplayState);

namespace SnoreSaber::Features::Live::Compete::Services
{
    namespace
    {
        bool EqualsIgnoreCase(const std::string& left, const std::string& right)
        {
            if (left.size() != right.size())
            {
                return false;
            }

            for (size_t i = 0; i < left.size(); i++)
            {
                if (std::tolower(static_cast<unsigned char>(left[i])) != std::tolower(static_cast<unsigned char>(right[i])))
                {
                    return false;
                }
            }
            return true;
        }
    }

    void CompeteGameplayState::ctor()
    {
        INVOKE_CTOR();
    }

    bool CompeteGameplayState::IsLiveGameplayActive() const
    {
        return _isLiveGameplayActive;
    }

    bool CompeteGameplayState::IsMapStartReady() const
    {
        return _isMapStartReady;
    }

    bool CompeteGameplayState::IsWaitingForMapStartReady() const
    {
        return _isLiveGameplayActive && !_isMapStartReady;
    }

    const std::string& CompeteGameplayState::TournamentId() const
    {
        return _tournamentId;
    }

    const std::string& CompeteGameplayState::MatchId() const
    {
        return _matchId;
    }

    const std::string& CompeteGameplayState::MapHash() const
    {
        return _mapHash;
    }

    void CompeteGameplayState::Begin(const std::string& tournamentId, const std::string& matchId, const std::string& mapHash)
    {
        bool wasActive = _isLiveGameplayActive;
        _isLiveGameplayActive = true;
        _tournamentId = tournamentId;
        _matchId = matchId;
        _mapHash = mapHash;
        _isMapStartReady = false;
        _hostStopRequested = false;
        _hostStopMapHash.clear();
        if (!wasActive && liveGameplayActiveChanged)
        {
            liveGameplayActiveChanged(true);
        }
    }

    void CompeteGameplayState::End()
    {
        bool wasActive = _isLiveGameplayActive;
        _isLiveGameplayActive = false;
        _isMapStartReady = false;
        _tournamentId.clear();
        _matchId.clear();
        _mapHash.clear();
        if (wasActive && liveGameplayActiveChanged)
        {
            liveGameplayActiveChanged(false);
        }
    }

    void CompeteGameplayState::MarkMapStartReady()
    {
        if (!_isLiveGameplayActive)
        {
            return;
        }

        _isMapStartReady = true;
    }

    bool CompeteGameplayState::IsCurrentMap(const std::string& matchId, const std::string& mapHash) const
    {
        if (!_isLiveGameplayActive)
        {
            return false;
        }

        if (!matchId.empty() && _matchId != matchId)
        {
            return false;
        }

        return mapHash.empty() || EqualsIgnoreCase(_mapHash, mapHash);
    }

    void CompeteGameplayState::MarkHostStopRequested()
    {
        if (!_isLiveGameplayActive)
        {
            return;
        }

        _hostStopRequested = true;
        _hostStopMapHash = _mapHash;
    }

    bool CompeteGameplayState::TryConsumeHostStop(const std::string& mapHash)
    {
        if (!_hostStopRequested)
        {
            return false;
        }

        bool matches = _hostStopMapHash.empty() || EqualsIgnoreCase(_hostStopMapHash, mapHash);
        _hostStopRequested = false;
        _hostStopMapHash.clear();
        return matches;
    }
}
