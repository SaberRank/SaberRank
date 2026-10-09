using HarmonyLib;
using LeaderboardCore.Models;
using SnoreSaber.Features.Leaderboards.Domain;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace SnoreSaber.Features.Leaderboards.Adapters.LeaderboardCore {
    // When ScoreSaber is installed, LeaderboardCore can restore ScoreSaber as the
    // last-used custom leaderboard. SnoreSaber should own its own custom-song
    // leaderboard instead, so reset the selection before LeaderboardCore handles
    // its normal activation logic.
    internal static class LeaderboardCoreSnoreSaberPriorityPatch {
        private const string NavigationButtonsType = "LeaderboardCore.UI.ViewControllers.LeaderboardNavigationButtonsController";

        private static MethodBase TargetMethod() {
            Type type = typeof(CustomLeaderboard).Assembly.GetType(NavigationButtonsType);
            return type == null ? null : AccessTools.Method(type, "OnLeaderboardLoaded");
        }

        private static bool Prepare() => TargetMethod() != null;

        private static void Prefix(object __instance) {
            try {
                var selectedLevel = Traverse.Create(__instance).Field("selectedLevelKey").GetValue<BeatmapKey?>();
                if (!selectedLevel.HasValue || !SnoreSaberBeatmapKey.IsSupportedLevelId(selectedLevel.Value.levelId)) {
                    return;
                }

                var leaderboards = Traverse.Create(__instance)
                    .Field("customLeaderboardsById")
                    .GetValue<Dictionary<string, CustomLeaderboard>>();

                if (leaderboards == null) {
                    return;
                }

                KeyValuePair<string, CustomLeaderboard>? snoreSaberEntry = leaderboards
                    .FirstOrDefault(x => x.Value is SnoreSaberCustomLeaderboard);

                if (snoreSaberEntry == null || snoreSaberEntry.Value.Value == null) {
                    return;
                }

                string snoreSaberId = snoreSaberEntry.Value.Key;
                object pluginConfig = Traverse.Create(__instance).Field("pluginConfig").GetValue<object>();
                if (pluginConfig != null) {
                    Traverse config = Traverse.Create(pluginConfig);
                    try {
                        config.Property("LastLeaderboard").SetValue(snoreSaberId);
                    } catch {
                        config.Field("LastLeaderboard").SetValue(snoreSaberId);
                    }
                }

                // Force LeaderboardCore back to its neutral state. Its original
                // OnLeaderboardLoaded then sees LastLeaderboard and calls
                // SwitchToLastLeaderboard(), replacing ScoreSaber's panel.
                Traverse.Create(__instance).Field("currentIndex").SetValue(0);
                Plugin.Log.Debug($"Prioritizing SnoreSaber custom leaderboard '{snoreSaberId}'.");
            }
            catch (Exception ex) {
                Plugin.Log.Warn($"Failed to prioritize SnoreSaber leaderboard: {ex.Message}");
            }
        }

        internal static void Install(Harmony harmony) {
            try {
                MethodBase target = TargetMethod();
                if (target == null) {
                    return;
                }

                harmony.Patch(target, prefix: new HarmonyMethod(typeof(LeaderboardCoreSnoreSaberPriorityPatch), nameof(Prefix)));
                Plugin.Log.Info("Installed LeaderboardCore SnoreSaber priority patch.");
            }
            catch (Exception ex) {
                Plugin.Log.Error($"Failed to install SnoreSaber LeaderboardCore priority patch: {ex}");
            }
        }
    }
}
