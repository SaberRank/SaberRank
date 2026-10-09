using System;

namespace SnoreSaber.Core.Presentation {

    internal static class PlayerPresentation {
        private const string Denyah = "76561198064659288";
        private const string CyanSnow = "76561198019856958";
        private const string Williums = "76561198182060577";
        private const string Umbranox = "76561198283584459";
        private const string Woops = "76561198077062414";
        private const string Jones = "76561198066901156";
        private const string Rain = "76561198066644109";

        internal static string GetLoginSuccessText(string playerId) {
            if (playerId == Denyah) {
                return "Wagwan piffting wots ur bbm pin?";
            }

            return "Successfully signed into SnoreSaber!";
        }

        internal static bool UsesFurryFont(string playerId) {
            return playerId == CyanSnow;
        }

        internal static bool UsesWilliumsPanel(string playerId) {
            return playerId == Williums;
        }

        internal static bool UsesDenyahPanel(string playerId) {
            return playerId == Denyah;
        }

        internal static Tuple<string, string> GetCrownDetails(string playerId) {
            switch (playerId) {
                case Woops:
                    return new Tuple<string, string>("SnoreSaber.Resources.crown-bronze.png", "Beat Saber Invitational 3rd place");
                case Jones:
                    return new Tuple<string, string>("SnoreSaber.Resources.crown-silver.png", "Beat Saber Invitational 2nd place");
                case Umbranox:
                    return new Tuple<string, string>("SnoreSaber.Resources.crown-umby.png", "Owner of SnoreSaber");
                case Rain:
                    return new Tuple<string, string>("SnoreSaber.Resources.crown-rain.png", "Owner of Umbranox's heart");
            }
            return new Tuple<string, string>("", "");
        }
    }
}
