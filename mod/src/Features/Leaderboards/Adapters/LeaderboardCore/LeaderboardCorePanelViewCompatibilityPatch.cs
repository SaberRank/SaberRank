using HarmonyLib;
using IPA.Loader;
using LeaderboardCore.Models;
using System;
using System.Reflection;

namespace SnoreSaber.Features.Leaderboards.Adapters.LeaderboardCore {
    // LeaderboardCore 1.7.0 contains a PanelView_SetIsLoaded Harmony patch whose
    // TargetMethod throws on Beat Saber 1.40.x. SnoreSaber is intentionally
    // loaded BEFORE LeaderboardCore so we can patch that TargetMethod first.
    internal static class LeaderboardCorePanelViewCompatibilityPatch {
        private const string PatchTypeName = "LeaderboardCore.HarmonyPatches.PanelView_SetIsLoaded";
        private static readonly Hive.Versioning.Version MaxAffectedLeaderboardCoreVersion = new Hive.Versioning.Version("1.7.0");

        private static bool Prefix(ref MethodBase __result) {
            __result = null;
            return false;
        }

        internal static void Install(Harmony harmony) {
            try {
                PluginMetadata metadata = PluginManager.GetPluginFromId("LeaderboardCore");
                if (metadata == null || metadata.HVersion.CompareTo(MaxAffectedLeaderboardCoreVersion) > 0) {
                    return;
                }

                Assembly assembly = typeof(CustomLeaderboard).Assembly;
                Type patchType = assembly.GetType(PatchTypeName);
                MethodInfo targetMethod = patchType == null
                    ? null
                    : patchType.GetMethod(
                        "TargetMethod",
                        BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic
                    );

                if (targetMethod == null) {
                    return;
                }

                harmony.Patch(targetMethod, prefix: new HarmonyMethod(typeof(LeaderboardCorePanelViewCompatibilityPatch), nameof(Prefix)));
                SnoreSaber.Plugin.Log.Info("Installed LeaderboardCore 1.7.0 PanelView compatibility patch.");
            }
            catch (Exception ex) {
                SnoreSaber.Plugin.Log.Error($"Failed to install LeaderboardCore PanelView compatibility patch: {ex}");
            }
        }
    }
}
