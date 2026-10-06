// camera2 reflects this old full name and treats Prefix() == false as replay playback
namespace SaberRank.Core.ReplaySystem.HarmonyPatches {
    internal static class PatchHandleHMDUnmounted {
        internal static bool Prefix() => !SaberRank.Features.Replays.ReplayStateRegistry.IsPlaybackEnabled;
    }
}
