#include <string>
#include <vector>

namespace SnoreSaber::Data::Private {
    struct SpectatorPose {
        float x;
        float y;
        float z;

        SpectatorPose(float x, float y, float z);
    };

    struct SpectatorPoseRoot {
        std::string name;
        SpectatorPose spectatorPose;

        SpectatorPoseRoot(SpectatorPose spectatorPose, std::string name);
    };

    namespace Settings {
        extern int fileVersion;
        extern bool showLocalPlayerRank;
        extern bool showScorePP;
        extern bool showStatusText;
        extern bool saveLocalReplays;
        extern bool enableCountryLeaderboards;
        extern std::string locationFilterMode;
        extern bool hideNAScoresFromLeaderboard;
        extern bool hasClickedSnoreSaberLogo;
        extern bool hasOpenedReplayUI;
        extern bool leftHandedReplayUI;
        extern bool lockedReplayUIMode;
        extern bool replayOverrideHandedness;
        extern bool useRecordedPlayerSettings;
        extern bool shareHsvProfiles;
        extern bool publicLivePresenceOptOut;
        extern bool liveChatOverlayEnabled;
        extern bool liveChatOverlayGameplayEnabled;
        extern float liveChatOverlayScale;
        extern float liveChatOverlayTextScale;
        extern std::vector<SpectatorPoseRoot> spectatorPositions;

        void InitializeDefaults();
        void InitializeDefaultSpectatorPositions();
        void LoadSettings();
        void SaveSettings();
    }
}
