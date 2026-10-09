using SnoreSaber.Core.Timing;
using SnoreSaber.Features.Live.Compete.Packets.Handlers;
using SnoreSaber.Features.Live.Ludus.Services;
using SnoreSaber.Features.Live.Ludus.Packets;
using SnoreSaber.Features.Live.Ludus.Packets.Handlers;

namespace SnoreSaber.Features.Live.Compete.Packets {
    internal static class CompeteLudusPacketDispatcher {
        internal static LudusPacketDispatcher<ILudusSessionPacketContext> CreateDefault(ILudusServerCommandSession commandSession, LudusChatMessageBuffer chatMessages, SnoreSaberClock clock) {
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
