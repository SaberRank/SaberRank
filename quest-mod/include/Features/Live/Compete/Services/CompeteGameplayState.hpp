#pragma once

#include <custom-types/shared/macros.hpp>

#include <functional>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::Services, CompeteGameplayState, System::Object) {
    DECLARE_CTOR(ctor);

public:
    // fired on the thread that calls Begin/End (gameplay launch is main thread)
    std::function<void(bool)> liveGameplayActiveChanged;

    bool IsLiveGameplayActive() const;
    bool IsMapStartReady() const;
    bool IsWaitingForMapStartReady() const;
    const std::string& TournamentId() const;
    const std::string& MatchId() const;
    const std::string& MapHash() const;

    void Begin(const std::string& tournamentId, const std::string& matchId, const std::string& mapHash);
    void End();
    void MarkMapStartReady();
    bool IsCurrentMap(const std::string& matchId, const std::string& mapHash) const;
    void MarkHostStopRequested();
    bool TryConsumeHostStop(const std::string& mapHash);

private:
    bool _isLiveGameplayActive = false;
    bool _isMapStartReady = false;
    std::string _tournamentId;
    std::string _matchId;
    std::string _mapHash;
    bool _hostStopRequested = false;
    std::string _hostStopMapHash;
};
