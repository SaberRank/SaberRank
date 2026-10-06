using System.Runtime.Serialization;

namespace SaberRank.Core.Gameplay {
    internal enum SaberRankPlayOutcome {
        [EnumMember(Value = "CLEAR")]
        Clear,
        [EnumMember(Value = "FAIL")]
        Fail,
        [EnumMember(Value = "QUIT")]
        Quit,
        [EnumMember(Value = "RESTART")]
        Restart
    }

    internal static class SaberRankPlayOutcomes {
        internal static SaberRankPlayOutcome FromLevelCompletionResults(LevelCompletionResults results) {
            if (results.levelEndStateType == LevelCompletionResults.LevelEndStateType.Failed) {
                return SaberRankPlayOutcome.Fail;
            }

            if (results.levelEndAction == LevelCompletionResults.LevelEndAction.Restart) {
                return SaberRankPlayOutcome.Restart;
            }

            if (results.levelEndAction == LevelCompletionResults.LevelEndAction.Quit) {
                return SaberRankPlayOutcome.Quit;
            }

            if (results.levelEndStateType == LevelCompletionResults.LevelEndStateType.Cleared) {
                return SaberRankPlayOutcome.Clear;
            }

            return SaberRankPlayOutcome.Quit;
        }
    }
}
