using System.Net.WebSockets;

namespace SaberRank_RankedPlay_Sockets.Models {
    public class ConnectedPlayer {
        public string PlayerId { get; set; } = "";
        public string PlayerName { get; set; } = "";
        public string Tier { get; set; } = "";
        public int TierDivision { get; set; }
        public WebSocket Socket { get; set; } = null!;
    }
}
