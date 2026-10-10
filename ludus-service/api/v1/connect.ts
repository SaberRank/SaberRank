import { createHmac, randomUUID, timingSafeEqual } from 'node:crypto';
import { createServer } from 'node:http';

import { create, fromBinary, toBinary } from '@bufbuild/protobuf';
import { Redis } from '@upstash/redis';
import { WebSocketServer, WebSocket } from 'ws';

import { LudusClientType, LudusDownloadState, LudusPlayState, LudusReadyState } from '../../proto/v1/common_pb.js';
import { LudusEnvelopeSchema, type LudusEnvelope as ProtoLudusEnvelope } from '../../proto/v1/ludus_pb.js';
import { LudusRoomContextType } from '../../proto/v1/room_actions_pb.js';
import { LiveMatchRoomStateSchema, RoomSnapshotSchema } from '../../proto/v1/room_state_pb.js';
import { ConnectAcceptedSchema, ErrorResponseSchema, HeartbeatAckSchema } from '../../proto/v1/session_pb.js';

const protocolVersion = 1;
const heartbeatIntervalMs = 10_000;
const publicEventsChannel = 'snoresaber:ludus:public:v1';
const redisUrl = process.env.UPSTASH_REDIS_REST_URL || process.env.LUDUS_REDIS_REST_URL || process.env.KV_REST_API_URL;
const redisToken = process.env.UPSTASH_REDIS_REST_TOKEN || process.env.LUDUS_REDIS_REST_TOKEN || process.env.KV_REST_API_TOKEN;
const pubsub = redisUrl && redisToken ? new Redis({ url: redisUrl, token: redisToken }) : null;
const ludusSecret = process.env.LUDUS_SESSION_SECRET || '';
const snoreApiUrl = (process.env.SNORE_API_URL || 'https://snoresaber.vercel.app').replace(/\/$/, '');
const connections = new Map<string, { socket: WebSocket; playerId: string; clientType: LudusClientType; roomContext: LudusRoomContextType; publicOptOut: boolean; currentRoomId: string; latestPresence?: { playState: number; currentRoomId: string; currentMapHash: string } }>();
const publicPresence = new Map<string, { playState: number; currentRoomId: string; currentMapHash: string }>();
let subscriberReady: Promise<void> | null = null;

const server = createServer((_request, response) => {
   response.writeHead(426, { 'content-type': 'text/plain' });
   response.end('WebSocket upgrade required');
});
const websocketServer = new WebSocketServer({ server, maxPayload: 256 * 1024, perMessageDeflate: false });

function envelope(body: ProtoLudusEnvelope['body'], connectionId = '', sequence = 0n) {
   return toBinary(LudusEnvelopeSchema, create(LudusEnvelopeSchema, {
      protocolVersion,
      messageId: randomUUID(),
      connectionId,
      sequence,
      clientTimeUnixMs: BigInt(Date.now()),
      serverTimeUnixMs: BigInt(Date.now()),
      body
   }));
}

function send(socket: WebSocket, body: ProtoLudusEnvelope['body'], connectionId = '', sequence = 0n) {
   if (socket.readyState === WebSocket.OPEN) socket.send(envelope(body, connectionId, sequence));
}

function sendError(socket: WebSocket, code: string, message: string, retryable = false) {
   send(socket, { case: 'error', value: create(ErrorResponseSchema, { code, message, retryable }) });
}

function base64url(value: string) {
   return Buffer.from(value, 'base64url').toString('utf8');
}

function verifyBrowserToken(token: string) {
   const [claimsPart, signaturePart, extra] = token.split('.');
   const secret = process.env.LUDUS_SESSION_SECRET || '';
   if (!claimsPart || !signaturePart || extra || secret.length < 32) return null;
   const expected = createHmac('sha256', secret).update(claimsPart).digest();
   let received: Buffer;
   try { received = Buffer.from(signaturePart, 'base64url'); } catch { return null; }
   if (received.length !== expected.length || !timingSafeEqual(received, expected)) return null;
   try {
      const claims = JSON.parse(base64url(claimsPart));
      if (claims.aud !== 'snoresaber-ludus' || claims.exp <= Math.floor(Date.now() / 1000) || !claims.playerId || claims.sub !== claims.playerId) return null;
      return { playerId: String(claims.playerId), clientType: LudusClientType.WEBSITE, roomContext: LudusRoomContextType.PUBLIC_PRESENCE, publicOptOut: false };
   } catch { return null; }
}

async function authenticateConnect(request: any) {
   if (request.authToken) return verifyBrowserToken(request.authToken);
   if (ludusSecret.length < 32 || !request.sessionId || !request.sessionKey || !request.playerId) return null;
   const body = JSON.stringify({ sessionId: request.sessionId, sessionKey: request.sessionKey, playerId: request.playerId });
   const timestamp = String(Date.now());
   const signature = createHmac('sha256', ludusSecret).update(`${timestamp}.${body}`).digest('base64url');
   const response = await fetch(`${snoreApiUrl}/api/v2/live/ludus/verify`, {
      method: 'POST',
      headers: { 'content-type': 'application/json', 'x-ludus-timestamp': timestamp, 'x-ludus-signature': signature },
      body,
      signal: AbortSignal.timeout(5000)
   });
   if (!response.ok) return null;
   const identity = await response.json() as { playerId?: string };
   if (!identity.playerId || String(identity.playerId) !== String(request.playerId)) return null;
   return {
      playerId: String(identity.playerId),
      clientType: request.clientType || LudusClientType.PLAYER,
      roomContext: request.initialRoomContext || LudusRoomContextType.CORE,
      publicOptOut: Boolean(request.publicLivePresenceOptOut)
   };
}

async function ensureSubscriber() {
   const redis = pubsub;
   if (!redis) {
      throw new Error('Ludus shared Redis is not connected');
   }
   if (!subscriberReady) {
      const subscription = redis.subscribe<string>(publicEventsChannel);
      subscriberReady = new Promise((resolve, reject) => {
         const readyTimeout = setTimeout(() => reject(new Error('Ludus shared Redis subscription timed out')), 5000);
         subscription.on('subscribe', () => { clearTimeout(readyTimeout); resolve(); });
         subscription.on('message', (messageEvent: any) => {
            try {
               const raw = messageEvent && typeof messageEvent === 'object' && 'message' in messageEvent ? messageEvent.message : messageEvent;
               const event = (typeof raw === 'string' ? JSON.parse(raw) : raw) as { kind: 'presence' | 'replay'; sender: string; playerId: string; matchId?: string; bytes?: string; state?: { playState: number; currentRoomId: string; currentMapHash: string }; online?: boolean };
               if (event.kind === 'presence') {
                  if (event.online && event.state) publicPresence.set(event.playerId, event.state);
                  else publicPresence.delete(event.playerId);
                  for (const [id, entry] of connections) {
                     if (id !== event.sender && !entry.publicOptOut && entry.roomContext === LudusRoomContextType.PUBLIC_PRESENCE && entry.socket.readyState === WebSocket.OPEN) {
                        send(entry.socket, { case: 'roomSnapshot', value: create(RoomSnapshotSchema, { rooms: getPublicRooms() }) }, id);
                     }
                  }
               } else if (event.kind === 'replay' && event.bytes) {
                  const packet = Buffer.from(event.bytes, 'base64');
                  for (const [id, entry] of connections) {
                     if (id === event.sender || entry.publicOptOut || entry.roomContext !== LudusRoomContextType.PUBLIC_PRESENCE) continue;
                     if (entry.currentRoomId && event.matchId && entry.currentRoomId !== event.matchId) continue;
                     if (entry.socket.readyState === WebSocket.OPEN) entry.socket.send(packet);
                  }
               }
            } catch { /* malformed Pub/Sub data is ignored */ }
         });
         subscription.on('error', (error) => { clearTimeout(readyTimeout); reject(error); });
      });
   }
   return subscriberReady;
}

async function publishPublicEvent(sender: string, playerId: string, bytes: Uint8Array, matchId?: string) {
   if (!pubsub) throw new Error('Ludus shared Redis is not connected');
   await pubsub.publish(publicEventsChannel, JSON.stringify({ kind: 'replay', sender, playerId, bytes: Buffer.from(bytes).toString('base64'), matchId }));
}

function getPublicRooms() {
   return [...publicPresence.entries()].map(([playerId, state]) => create(LiveMatchRoomStateSchema, {
      matchId: `player:${playerId}`,
      roomId: state.currentRoomId,
      loadedSong: Boolean(state.currentMapHash),
      loadedSongHash: state.currentMapHash,
      loadedSongName: '',
      playerIds: [playerId],
      playerStates: [{
         playerId,
         playState: state.playState,
         downloadState: state.currentMapHash ? LudusDownloadState.DOWNLOADED : LudusDownloadState.NONE,
         readyState: LudusReadyState.NOT_READY,
         isBot: false,
         errorMessage: ''
      }],
      viewerCount: 0,
      viewers: []
   }));
}

async function publishPresence(sender: string, playerId: string, state: { playState: number; currentRoomId: string; currentMapHash: string }, online: boolean) {
   if (!pubsub) throw new Error('Ludus shared Redis is not connected');
   await pubsub.publish(publicEventsChannel, JSON.stringify({ kind: 'presence', sender, playerId, state, online }));
}

websocketServer.on('connection', (socket) => {
   let connectionId = '';
   let currentSequence = 0n;
   let connectedPlayerId = '';
   socket.on('message', async (raw, isBinary) => {
      if (!isBinary) {
         sendError(socket, 'BINARY_REQUIRED', 'Ludus packets must use the binary protobuf protocol');
         return;
      }
      try {
         const incoming = fromBinary(LudusEnvelopeSchema, new Uint8Array(raw as Buffer));
         const body = incoming.body;
         if (body.case === 'connectRequest') {
            if (connectionId) return sendError(socket, 'ALREADY_CONNECTED', 'This socket is already connected');
            const identity = await authenticateConnect(body.value);
            if (!identity) {
               sendError(socket, 'UNAUTHORIZED', 'The SnoreSaber game session or website token is invalid or expired');
               socket.close(4401, 'Unauthorized');
               return;
            }
            try { await ensureSubscriber(); }
            catch {
               sendError(socket, 'LIVE_NOT_CONFIGURED', 'Live service storage is not connected yet', true);
               socket.close(1013, 'Live service is not configured');
               return;
            }
            connectionId = randomUUID();
            connectedPlayerId = identity.playerId;
            const clientType = identity.clientType;
            connections.set(connectionId, { socket, playerId: identity.playerId, clientType, roomContext: identity.roomContext, publicOptOut: identity.publicOptOut, currentRoomId: '' });
            send(socket, { case: 'connectAccepted', value: create(ConnectAcceptedSchema, {
               connectionId,
               protocolVersion,
               heartbeatIntervalMs,
               sendQueueSize: 256,
               maxPacketSizeBytes: 256 * 1024,
               roomContext: identity.roomContext,
               clientType
            }) }, connectionId, ++currentSequence);
            if (identity.roomContext === LudusRoomContextType.PUBLIC_PRESENCE) {
               try {
                  const playerIds = await pubsub!.smembers('snoresaber:ludus:online:v1');
                  const keys = playerIds.map((id) => `snoresaber:ludus:player:v1:${id}`);
                  const states = keys.length ? await pubsub!.mget(...keys) : [];
                  for (let index = 0; index < playerIds.length; index++) {
                     const serialized = states[index];
                     if (serialized == null) {
                        await pubsub!.srem('snoresaber:ludus:online:v1', String(playerIds[index]));
                        continue;
                     }
                     publicPresence.set(String(playerIds[index]), typeof serialized === 'string' ? JSON.parse(serialized) : serialized as any);
                  }
               } catch { /* the stream can still connect and receive fresh events */ }
               send(socket, { case: 'roomSnapshot', value: create(RoomSnapshotSchema, { rooms: getPublicRooms() }) }, connectionId, ++currentSequence);
            }
            return;
         }
         if (!connectionId) {
            sendError(socket, 'CONNECT_REQUIRED', 'Connect before sending Ludus packets');
            socket.close(4401, 'Connect required');
            return;
         }
         if (body.case === 'heartbeat') {
            const connection = connections.get(connectionId);
            if (connection && connection.clientType === LudusClientType.PLAYER && connection.latestPresence && !connection.publicOptOut) {
               await pubsub!.set(`snoresaber:ludus:player:v1:${connectedPlayerId}`, JSON.stringify(connection.latestPresence), { ex: 35 });
               await pubsub!.sadd('snoresaber:ludus:online:v1', connectedPlayerId);
            }
            send(socket, { case: 'heartbeatAck', value: create(HeartbeatAckSchema, { highestSeenSequence: currentSequence }) }, connectionId, ++currentSequence);
            return;
         }
         if (body.case === 'followRoomRequest') {
            const connection = connections.get(connectionId);
            if (connection) connection.currentRoomId = body.value.matchId;
            return;
         }
         if (body.case === 'presenceUpdate') {
            const connection = connections.get(connectionId);
            if (!connection) return;
            const update = body.value;
            connection.currentRoomId = update.currentRoomId || connection.currentRoomId;
            if (!connection.publicOptOut) {
               const state = { playState: update.playState, currentRoomId: connection.currentRoomId, currentMapHash: update.currentMapHash || '' };
               connection.latestPresence = state;
               await pubsub!.set(`snoresaber:ludus:player:v1:${connectedPlayerId}`, JSON.stringify(state), { ex: 35 });
               await pubsub!.sadd('snoresaber:ludus:online:v1', connectedPlayerId);
               publicPresence.set(connectedPlayerId, state);
               await publishPresence(connectionId, connectedPlayerId, state, true);
            }
            return;
         }
         if (body.case === 'replayPacket') {
            const connection = connections.get(connectionId);
            if (connection && !connection.publicOptOut) {
               await publishPublicEvent(connectionId, connectedPlayerId, new Uint8Array(raw as Buffer), body.value.matchId);
            }
            return;
         }
         if (body.case === 'setClientTypeRequest') {
            const connection = connections.get(connectionId);
            if (connection) connection.clientType = body.value.clientType;
         }
      } catch {
         sendError(socket, 'INVALID_PACKET', 'The Ludus packet could not be decoded');
      }
   });
   socket.on('close', () => {
      const connection = connectionId ? connections.get(connectionId) : null;
      if (connectionId) connections.delete(connectionId);
      if (connection && connection.clientType === LudusClientType.PLAYER && !connection.publicOptOut && ![...connections.values()].some((item) => item.playerId === connection.playerId)) {
         publicPresence.delete(connection.playerId);
         void pubsub?.del(`snoresaber:ludus:player:v1:${connection.playerId}`);
         void pubsub?.srem('snoresaber:ludus:online:v1', connection.playerId);
         void publishPresence(connectionId, connection.playerId, { playState: LudusPlayState.IN_MENUS, currentRoomId: '', currentMapHash: '' }, false);
      }
   });
   socket.on('error', () => {
      if (connectionId) connections.delete(connectionId);
   });
});

export default server;
