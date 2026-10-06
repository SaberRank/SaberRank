using SaberRank.Core;
using SaberRank.Features.Live.Ludus.Services;
using SaberRank.Features.Live.Protocol;

namespace SaberRank.Features.Live.Ludus.Packets.Handlers {
    internal sealed class RoomContextUpdatedEnvelopeHandler<TSession> : ILudusEnvelopeHandler<TSession>
        where TSession : ILudusSessionPacketContext {
        public LudusEnvelopeType Type => LudusEnvelopeType.RoomContextUpdated;

        public void Handle(TSession session, DecodedLudusEnvelope envelope) {
            session.ApplyClientContext(envelope);
            Plugin.Log.Info($"Ludus: Room context changed to {session.ClientType} {session.RoomContext} {session.CurrentLudusMatchId}");
        }
    }
}
