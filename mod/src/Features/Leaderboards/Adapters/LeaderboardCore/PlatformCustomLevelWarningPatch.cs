using HarmonyLib;

namespace SnoreSaber.Features.Leaderboards.Adapters.LeaderboardCore {
    [HarmonyPatch(typeof(LoadingControl), nameof(LoadingControl.ShowText))]
    internal static class PlatformCustomLevelWarningPatch {
        private static bool Prefix(LoadingControl __instance, string text) => !SnoreSaberLeaderboardCoreViewController.ShouldSuppressPlatformCustomLevelWarning(__instance, text);
    }
}
