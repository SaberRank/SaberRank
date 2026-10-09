// camera2 reflects this old full name and treats Prefix() == false as replay playback
namespace SnoreSaber.Core.ReplaySystem.HarmonyPatches {
    internal static class PatchHandleHMDUnmounted {
        internal static bool Prefix() => !SnoreSaber.Features.Replays.ReplayStateRegistry.IsPlaybackEnabled;
    }
}
