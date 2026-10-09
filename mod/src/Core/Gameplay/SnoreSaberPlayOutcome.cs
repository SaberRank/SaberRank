using System.Runtime.Serialization;

namespace SnoreSaber.Core.Gameplay {
    internal enum SnoreSaberPlayOutcome {
        [EnumMember(Value = "CLEAR")]
        Clear,
        [EnumMember(Value = "FAIL")]
        Fail,
        [EnumMember(Value = "QUIT")]
        Quit,
        [EnumMember(Value = "RESTART")]
        Restart
    }

    internal static class SnoreSaberPlayOutcomes {
        internal static SnoreSaberPlayOutcome FromLevelCompletionResults(LevelCompletionResults results) {
            if (results.levelEndStateType == LevelCompletionResults.LevelEndStateType.Failed) {
                return SnoreSaberPlayOutcome.Fail;
            }

            if (results.levelEndAction == LevelCompletionResults.LevelEndAction.Restart) {
                return SnoreSaberPlayOutcome.Restart;
            }

            if (results.levelEndAction == LevelCompletionResults.LevelEndAction.Quit) {
                return SnoreSaberPlayOutcome.Quit;
            }

            if (results.levelEndStateType == LevelCompletionResults.LevelEndStateType.Cleared) {
                return SnoreSaberPlayOutcome.Clear;
            }

            return SnoreSaberPlayOutcome.Quit;
        }
    }
}
