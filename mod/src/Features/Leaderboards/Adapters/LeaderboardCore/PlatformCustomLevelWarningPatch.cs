using HarmonyLib;

namespace SaberRank.Features.Leaderboards.Adapters.LeaderboardCore {
    [HarmonyPatch(typeof(LoadingControl), nameof(LoadingControl.ShowText))]
    internal static class PlatformCustomLevelWarningPatch {
        private static bool Prefix(LoadingControl __instance, string text) => !SaberRankLeaderboardCoreViewController.ShouldSuppressPlatformCustomLevelWarning(__instance, text);
    }
}
