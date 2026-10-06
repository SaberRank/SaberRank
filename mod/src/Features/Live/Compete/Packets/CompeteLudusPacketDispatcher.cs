using SaberRank.Core.Timing;
using SaberRank.Features.Live.Compete.Packets.Handlers;
using SaberRank.Features.Live.Ludus.Services;
using SaberRank.Features.Live.Ludus.Packets;
using SaberRank.Features.Live.Ludus.Packets.Handlers;

namespace SaberRank.Features.Live.Compete.Packets {
    internal static class CompeteLudusPacketDispatcher {
        internal static LudusPacketDispatcher<ILudusSessionPacketContext> CreateDefault(ILudusServerCommandSession commandSession, LudusChatMessageBuffer chatMessages, SaberRankClock clock) {
            LudusServerCommandDispatcher commandDispatcher = LudusServerCommandDispatcher.CreateDefault();
            return new LudusPacketDispatcher<ILudusSessionPacketContext>(new ILudusEnvelopeHandler<ILudusSessionPacketContext>[] {
                new ConnectAcceptedEnvelopeHandler<ILudusSessionPacketContext>(),
                new RoomContextUpdatedEnvelopeHandler<ILudusSessionPacketContext>(),
                new ReconnectRequestedEnvelopeHandler<ILudusSessionPacketContext>(),
                new RoomSnapshotEnvelopeHandler(commandSession),
                new ServerCommandEnvelopeHandler(commandDispatcher, commandSession),
                new ChatMessageEnvelopeHandler<ILudusSessionPacketContext>(chatMessages),
                new ChatSnapshotEnvelopeHandler<ILudusSessionPacketContext>(chatMessages),
                new ErrorEnvelopeHandler<ILudusSessionPacketContext>()
            }, clock);
        }
    }
}
