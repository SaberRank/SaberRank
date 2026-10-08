import { createHmac, randomBytes, timingSafeEqual } from 'node:crypto';
import { defineHandler } from 'nitro';
import { db } from '../../utils/db';

import { CURATED_BEATSAVER_MAP_KEYS } from "../../beatsaver-curated";
const SECRET = process.env.SESSION_SECRET || 'snoresaber-development-secret-change-me';
const INGEST_KEY = process.env.SNORE_INGEST_KEY || '';
const STEAM_API_KEY = process.env.STEAM_API_KEY || '';
const NOW = () => new Date().toISOString();

// SnoreSaber's initial curated map set. These are BeatSaver map keys, not
// SnoreSaber's internal numeric map IDs. Keep this list as the source of truth
// for the public map catalog until more maps are intentionally added.
const PUBLIC_BEATSAVER_MAP_KEYS = new Set(['25198', '4fdd2', '52dfb', '4e692', '4d977', '51e10']);

// Development/demo data is deliberately kept as a fallback. Once DATABASE_URL is
// configured, every read/write below uses PostgreSQL instead of these arrays.
const players: any[] = [
  player('76561198000000001', 'YawningSylveon', 'CA', 1, 16284.32),
  player('76561198000000002', 'Lunaa', 'US', 2, 16112.07),
  player('76561198000000003', 'Kyouki', 'JP', 3, 15998.44),
  player('76561198000000004', 'Rho', 'US', 4, 15781.2),
  player('76561198000000005', 'Astra', 'GB', 5, 15662.11)
];

const maps: any[] = [
  map(1, 'A1B2C3D4E5F6', 'Imprinting', '', 'CreepyBlock', 'Sotarks', 174, 382144, 9.42),
  map(2, 'B2C3D4E5F6A1', 'B.B.K.K.B.K.K.', '', 'nora2r', 'nora2r', 170, 301552, 9.18),
  map(3, 'C3D4E5F6A1B2', 'Kimi no Bouken', '', 'Sotarks', 'Sotarks', 168, 298771, 8.91),
  map(4, 'D4E5F6A1B2C3', 'Ghost', '', 'Camellia', 'Rustic', 150, 286905, 8.74),
  map(5, 'E5F6A1B2C3D4', 'Machine Gun', '', 'Kobaryo', 'Sotarks', 200, 274663, 8.62)
];

const scores: any[] = [
  score(1001, players[0], maps[0], 98.42, 999876, 512.4),
  score(1002, players[1], maps[2], 97.15, 987112, 498.2),
  score(1003, players[2], maps[3], 96.88, 972441, 487.1),
  score(1004, players[3], maps[1], 97.63, 981234, 493.8),
  score(1005, players[4], maps[4], 95.21, 951337, 471.2)
];

function player(id: string, name: string, country: string, rank: number, pp: number) {
  return {
    id, name, playerNameInGame: name, country, role: null,
    avatar: `https://ui-avatars.com/api/?name=${encodeURIComponent(name)}&background=16131f&color=ff79bd&bold=true`,
    avatarVersion: 1, permissions: 0, banned: false, silenced: false, inactive: false,
    stats: {
      realmId: 1, realmName: 'SnoreSaber', rank, countryRank: rank, rankChange: 0,
      totalPP: pp, plusOnePP: pp + 1, totalScore: '0', totalRankedScore: '0',
      totalPlayedLeaderboards: 5, totalPlayedRankedLeaderboards: 5, totalSubmittedPlays: 1,
      totalReplayViews: 0, averageAccuracy: 97.1, weightedAverageAccuracy: 97.1,
      completionAccuracy: 97.1, device: { hmd: 'Quest 2', controllerLeft: 'Touch', controllerRight: 'Touch' }
    }, bio: null, vanity: null,
    profileCustomization: { backgroundImage: null, backgroundImageVersion: null, accentColor: '#f06ab7', accentForegroundColor: '#160d16', accentForegroundActiveColor: '#ffffff', supporterNameColorEnabled: false, badgeOrder: null, badgeComments: null, statOrder: null, enabledStatIds: null, chartMetricIds: null, sectionOrder: null },
    createdAt: NOW(), lastSeenAt: NOW(), badges: [], relationships: { following: [], mutuals: [] }
  };
}

function realm(stars: number, status = 'RANKED') {
  return { realmId: 1, realmName: 'SnoreSaber', leaderboardStatus: status, positiveModifiers: false, stars, rankedAt: status === 'RANKED' ? NOW() : null, qualifiedAt: null, lovedAt: null };
}

function map(id: number, hash: string, songName: string, sub: string, author: string, mapper: string, bpm: number, totalScores: number, stars: number) {
  return {
    id, hash, bsid: null, songName, songSubName: sub, songAuthorName: author, levelAuthorName: mapper,
    bpm, coverUrl: '/assets/snoresaber-icon.png', verified: true, totalScores, dailyScores: Math.floor(totalScores / 90), createdAt: NOW(),
    leaderboards: [leaderboard(id * 10, id, 9, 'Standard', 'ExpertPlus', stars, totalScores)]
  };
}

function leaderboard(id: number, mapId: number, difficulty: number, gameMode: string, rawDifficulty: string, stars: number, totalScores: number, status = 'RANKED') {
  return { id, difficulty, gameMode, rawDifficulty, maxScore: 1000000, totalScores, dailyScores: Math.floor(totalScores / 90), createdAt: NOW(), realm: realm(stars, status) };
}

function score(id: number, p: any, m: any, accuracy: number, modifiedScore: number, pp: number) {
  return {
    id, rank: p.stats.rank, unmodifiedScore: modifiedScore, modifiedScore, accuracy, pp, weight: 1,
    mods: [], badCuts: 0, missedNotes: 0, maxCombo: 999, fullCombo: true, hasReplay: false, personalBest: true,
    replayViewCount: 0, legacyHmdId: 2, version: '1.40.8', playOutcome: 'CLEAR', playOutcomeTime: null, createdAt: NOW(), hasHistory: false,
    player: { id: p.id, name: p.name, playerNameInGame: p.name, country: p.country, role: null, avatar: p.avatar, avatarVersion: 1, permissions: 0 },
    device: { hmd: 'Quest 2', controllerLeft: 'Touch', controllerRight: 'Touch' },
    leaderboard: leaderboard(m.id * 10, m.id, 9, 'Standard', 'ExpertPlus', m.leaderboards[0].realm.stars, m.totalScores)
  };
}

function metadata(total: number, page: number, limit: number) {
  return { page, itemsPerPage: limit, totalItems: total, totalPages: Math.max(1, Math.ceil(total / limit)) };
}

function json(data: unknown, status = 200, headers: Record<string, string> = {}) {
  return new Response(JSON.stringify(data), { status, headers: { 'content-type': 'application/json; charset=utf-8', ...headers } });
}

function tokenFor(playerId: string) {
  const body = Buffer.from(JSON.stringify({ sub: playerId, iat: Date.now() })).toString('base64url');
  const sig = createHmac('sha256', SECRET).update(body).digest('base64url');
  return `${body}.${sig}`;
}

function playerIdFromToken(token?: string | null) {
  if (!token) return null;
  const [body, sig] = token.split('.');
  if (!body || !sig) return null;
  const expected = createHmac('sha256', SECRET).update(body).digest('base64url');
  try {
    if (!timingSafeEqual(Buffer.from(sig), Buffer.from(expected))) return null;
    const payload = JSON.parse(Buffer.from(body, 'base64url').toString());
    return typeof payload.sub === 'string' ? payload.sub : null;
  } catch {
    return null;
  }
}

function tokenFromCookie(raw: string | undefined) {
  const found = (raw || '').split(';').map((x) => x.trim()).find((x) => x.startsWith('token='));
  return found ? decodeURIComponent(found.slice(6)) : null;
}

async function authPlayerId(request: Request, sql: any) {
  const id = playerIdFromToken(tokenFromCookie(request.headers.get('cookie') || undefined));
  if (!id) return null;
  if (sql) {
    const rows: any[] = await sql`SELECT id FROM players WHERE id=${id} LIMIT 1`;
    return rows[0]?.id ?? null;
  }
  return players.some((p) => p.id === id) ? id : null;
}

function hasIngestAuth(request: Request) {
  if (!INGEST_KEY) return false;
  const header = request.headers.get('authorization') || '';
  const token = header.startsWith('Bearer ') ? header.slice(7) : request.headers.get('x-snore-ingest-key');
  return token === INGEST_KEY;
}

async function verifySteam(req: Request) {
  const url = new URL(req.url);
  const params = new URLSearchParams();
  url.searchParams.forEach((v, k) => { if (k.startsWith('openid.')) params.set(k, v); });
  params.set('openid.mode', 'check_authentication');
  const response = await fetch('https://steamcommunity.com/openid/login', {
    method: 'POST', headers: { 'content-type': 'application/x-www-form-urlencoded' }, body: params
  });
  const text = await response.text();
  if (!text.includes('is_valid:true')) return null;
  const claimed = url.searchParams.get('openid.claimed_id') || '';
  return claimed.match(/([0-9]{17})$/)?.[1] ?? null;
}

async function fetchSteamProfile(steamId: string) {
  try {
    if (STEAM_API_KEY) {
      const response = await fetch(`https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=${encodeURIComponent(STEAM_API_KEY)}&steamids=${steamId}`, {
        headers: { accept: 'application/json' },
        cache: 'no-store'
      });
      if (response.ok) {
        const data: any = await response.json();
        const profile = data?.response?.players?.[0];
        if (profile) {
          return {
            name: typeof profile.personaname === 'string' && profile.personaname.trim() ? profile.personaname.trim() : null,
            avatar: typeof profile.avatarfull === 'string' ? profile.avatarfull : null
          };
        }
      }
    }

    const response = await fetch(`https://steamcommunity.com/profiles/${steamId}?xml=1`, {
      headers: { 'user-agent': 'SnoreSaber/3.0 SteamLogin' },
      cache: 'no-store'
    });
    if (!response.ok) return null;
    const xml = await response.text();
    const read = (tag: string) => {
      const match = xml.match(new RegExp(`<${tag}>([\\s\\S]*?)</${tag}>`, 'i'));
      return match ? match[1].replace(/<!\[CDATA\[|\]\]>/g, '').trim() : null;
    };
    return { name: read('steamID'), avatar: read('avatarFull') };
  } catch {
    return null;
  }
}


async function getFollowRelationship(sql: any, viewerId: string, targetId: string) {
  if (!sql || !viewerId || !targetId || viewerId === targetId) return { following: false, followsViewer: false, mutual: false };
  const rows: any[] = await sql`
    SELECT
      EXISTS(SELECT 1 FROM player_follows WHERE follower_id=${viewerId} AND following_id=${targetId}) AS following,
      EXISTS(SELECT 1 FROM player_follows WHERE follower_id=${targetId} AND following_id=${viewerId}) AS follows_viewer
  `;
  const following = Boolean(rows[0]?.following);
  const followsViewer = Boolean(rows[0]?.follows_viewer);
  return { following, followsViewer, mutual: following && followsViewer };
}

async function relationshipSummary(sql: any, playerId: string, viewerId?: string | null) {
  const empty = { followers: 0, following: 0, platformFriends: 0, recentFollowers: [], recentFollowing: [], viewerRelationship: { following: false, followsViewer: false, mutual: false } };
  if (!sql) return empty;
  // Profiles must remain readable even if the optional social migration has not
  // been applied yet. The follow endpoints will become active once the table exists.
  try {
    const exists: any[] = await sql`SELECT to_regclass('public.player_follows') AS table_name`;
    if (!exists[0]?.table_name) return empty;
  } catch {
    return empty;
  }
  try {
    const counts: any[] = await sql`
    SELECT
      (SELECT COUNT(*)::int FROM player_follows WHERE following_id=${playerId}) AS followers,
      (SELECT COUNT(*)::int FROM player_follows WHERE follower_id=${playerId}) AS following
  `;
  const recentFollowers: any[] = await sql`
    SELECT p.id,p.name,p.country,p.avatar
    FROM player_follows f JOIN players p ON p.id=f.follower_id
    WHERE f.following_id=${playerId}
    ORDER BY f.created_at DESC LIMIT 5
  `;
  const recentFollowing: any[] = await sql`
    SELECT p.id,p.name,p.country,p.avatar
    FROM player_follows f JOIN players p ON p.id=f.following_id
    WHERE f.follower_id=${playerId}
    ORDER BY f.created_at DESC LIMIT 5
  `;
  const rel = viewerId ? await getFollowRelationship(sql, viewerId, playerId) : { following: false, followsViewer: false, mutual: false };
  const mapRelationship = (r: any) => ({ id: String(r.id), name: r.name, playerNameInGame: r.name, country: r.country || 'XX', role: r.role ?? null, avatar: r.avatar || '', avatarVersion: 1, permissions: Number(r.permissions || 0) });
    return {
      followers: Number(counts[0]?.followers || 0),
      following: Number(counts[0]?.following || 0),
      platformFriends: 0,
      recentFollowers: recentFollowers.map(mapRelationship),
      recentFollowing: recentFollowing.map(mapRelationship),
      viewerRelationship: rel
    };
  } catch {
    return empty;
  }
}

async function userRelationships(sql: any, playerId: string) {
  if (!sql) return { following: [], mutuals: [] };
  const rows: any[] = await sql`
    SELECT f.following_id AS id, p.name,p.country,p.avatar,p.role,p.permissions,
      EXISTS(SELECT 1 FROM player_follows back WHERE back.follower_id=f.following_id AND back.following_id=${playerId}) AS follows_back
    FROM player_follows f JOIN players p ON p.id=f.following_id
    WHERE f.follower_id=${playerId}
    ORDER BY f.created_at DESC
  `;
  const following = rows.filter((r) => !r.follows_back).map((r) => ({ id: String(r.id), relation: 'follow', name: r.name, playerNameInGame: r.name, country: r.country || 'XX', role: r.role ?? null, avatar: r.avatar || '', avatarVersion: 1, permissions: Number(r.permissions || 0) }));
  const mutuals = rows.filter((r) => r.follows_back).map((r) => ({ id: String(r.id), relation: 'follow', name: r.name, playerNameInGame: r.name, country: r.country || 'XX', role: r.role ?? null, avatar: r.avatar || '', avatarVersion: 1, permissions: Number(r.permissions || 0) }));
  return { following, mutuals };
}

async function isAdmin(sql: any, playerId: string | null) {
  if (!sql || !playerId) return false;
  const rows: any[] = await sql`
    SELECT permissions, role
    FROM players
    WHERE id=${playerId}
    LIMIT 1
  `;
  const permissions = Number(rows[0]?.permissions || 0);
  const role = String(rows[0]?.role || '').trim().toLowerCase();
  return (permissions & 16) !== 0 || role === 'admin' || role === 'administrator' || role === 'snoresaber admin';
}

async function ensureLeaderboardAdminColumns(sql: any) {
  await sql`ALTER TABLE leaderboards ADD COLUMN IF NOT EXISTS stars DOUBLE PRECISION NOT NULL DEFAULT 0`;
  await sql`ALTER TABLE leaderboards ADD COLUMN IF NOT EXISTS status TEXT NOT NULL DEFAULT 'UNRANKED'`;
  await sql`ALTER TABLE leaderboards ADD COLUMN IF NOT EXISTS ranked_at TIMESTAMPTZ`;
}

const PERMISSION_VALUES: Record<string, number> = {
  RT: 1,
  QAT: 2,
  QATHead: 4,
  NAT: 8,
  ADMIN: 16,
  PANDA: 32,
  SUPPORTER: 64,
  PPFARMER: 128,
  DEV: 256,
  CCT: 1024,
  CCTHead: 2048,
  CAT: 4096,
  RTR: 8192,
  EXTERNAL_DEV: 16384,
  TOURNAMENT_ORGANIZER: 32768
};

async function resolvePlayerId(sql: any, requestedId: string) {
  if (!sql) {
    const normalized = String(requestedId).toLowerCase();
    const match = players.find((p) =>
      String(p.id).toLowerCase() === normalized ||
      String(p.steamId ?? '').toLowerCase() === normalized ||
      String(p.vanity ?? '').toLowerCase() === normalized ||
      String(p.name ?? '').toLowerCase() === normalized
    );
    return match?.id ?? requestedId;
  }
  const value = String(requestedId);
  const rows: any[] = await sql`
    SELECT id FROM players
    WHERE id=${value}
       OR steam_id=${value}
       OR lower(vanity)=lower(${value})
       OR lower(name)=lower(${value})
    LIMIT 1`;
  return rows[0]?.id ?? null;
}

const BEATSAVER_API = process.env.BEATSAVER_API_URL || 'https://api.beatsaver.com';

function beatSaverDifficultyValue(value: unknown) {
  switch (String(value || '').toLowerCase()) {
    case 'easy': return 1;
    case 'normal': return 3;
    case 'hard': return 5;
    case 'expert': return 7;
    case 'expertplus':
    case 'expert+': return 9;
    default: return 0;
  }
}

function beatSaverGameMode(value: unknown) {
  const text = String(value || 'Standard');
  if (text === 'Standard') return 'Standard';
  if (/one saber/i.test(text)) return 'OneSaber';
  if (/90.?degree/i.test(text)) return '90Degree';
  if (/360.?degree/i.test(text)) return '360Degree';
  return text.replace(/[^a-zA-Z0-9]+/g, '');
}

async function syncBeatSaverMaps(sql: any, options: { maxPages?: number; forceBootstrap?: boolean } = {}) {
  if (!sql) return { synced: 0, source: 'fallback', complete: true };

  // BeatSaver's /maps/latest endpoint is cursor-paginated. We persist the cursor in
  // Neon so the initial import can walk ALL historical maps over multiple serverless
  // invocations, while normal syncs only fetch maps uploaded since the last sync.
  const pageSize = 100;
  const maxPages = Math.max(1, Math.min(50, Number(options.maxPages || 50)));

  await sql`
    CREATE TABLE IF NOT EXISTS beatsaver_sync_state (
      id INTEGER PRIMARY KEY CHECK (id=1),
      last_sync_at TIMESTAMPTZ NOT NULL DEFAULT now(),
      maps_synced INTEGER NOT NULL DEFAULT 0
    )`;
  await sql`ALTER TABLE beatsaver_sync_state ADD COLUMN IF NOT EXISTS bootstrap_before TIMESTAMPTZ`;
  await sql`ALTER TABLE beatsaver_sync_state ADD COLUMN IF NOT EXISTS bootstrap_complete BOOLEAN NOT NULL DEFAULT false`;
  await sql`ALTER TABLE beatsaver_sync_state ADD COLUMN IF NOT EXISTS newest_uploaded_at TIMESTAMPTZ`;

  const stateRows: any[] = await sql`SELECT * FROM beatsaver_sync_state WHERE id=1 LIMIT 1`;
  let state = stateRows[0] || null;
  if (!state) {
    const inserted: any[] = await sql`
      INSERT INTO beatsaver_sync_state (id, last_sync_at, maps_synced, bootstrap_complete)
      VALUES (1, now(), 0, false)
      RETURNING *`;
    state = inserted[0];
  }

  // If an admin explicitly asks for a bootstrap, restart/continue the historical walk.
  if (options.forceBootstrap && state.bootstrap_complete) {
    await sql`
      UPDATE beatsaver_sync_state
      SET bootstrap_before=NULL, bootstrap_complete=false, newest_uploaded_at=NULL
      WHERE id=1`;
    state = { ...state, bootstrap_before: null, bootstrap_complete: false, newest_uploaded_at: null };
  }

  const bootstrapping = !Boolean(state.bootstrap_complete);
  let before: string | null = state.bootstrap_before ? new Date(state.bootstrap_before).toISOString() : null;
  const cutoff = state.newest_uploaded_at ? new Date(state.newest_uploaded_at) : null;
  let synced = 0;
  let fetched = 0;
  let pages = 0;
  let complete = !bootstrapping;
  let newestSeen: Date | null = cutoff;

  while (pages < maxPages) {
    const params = new URLSearchParams({ pageSize: String(pageSize) });
    if (before) params.set('before', before);

    const response = await fetch(`${BEATSAVER_API}/maps/latest?${params.toString()}`, {
      headers: { 'accept': 'application/json', 'user-agent': 'SnoreSaber/2.0 BeatSaver sync' },
      cache: 'no-store'
    });
    if (!response.ok) throw new Error(`BeatSaver returned HTTP ${response.status}`);
    const payload: any = await response.json();
    const docs = Array.isArray(payload?.docs) ? payload.docs : [];
    if (docs.length === 0) {
      complete = true;
      break;
    }

    pages++;
    fetched += docs.length;

    for (const map of docs) {
      const uploaded = map.uploaded ? new Date(map.uploaded) : null;
      if (uploaded && !Number.isNaN(uploaded.getTime())) {
        if (!newestSeen || uploaded > newestSeen) newestSeen = uploaded;
      }

      // During incremental sync, the latest page can contain a few maps we already
      // have because of cursor/timestamp boundaries. Re-processing them is harmless,
      // but once the whole page is at/before our saved cutoff we are done.
      if (!bootstrapping && cutoff && uploaded && uploaded < cutoff) continue;

      const version = Array.isArray(map.versions)
        ? (map.versions.find((v: any) => String(v.state || '').toLowerCase() === 'published') || map.versions[0])
        : null;
      const hash = String(version?.hash || '').trim();
      const bsid = String(map.id || version?.key || '').trim();
      if (!hash || !bsid) continue;

      const metadata = map.metadata || {};
      const coverUrl = String(version?.coverURL || map.coverURL || `https://eu.cdn.beatsaver.com/${hash}.jpg`).trim();
      // BeatSaver's ranked/qualified flags are external ranking metadata.
      // SnoreSaber owns ranking state, so new difficulties are unranked and
      // existing local ranking state is preserved on conflict.
      const inserted: any[] = await sql`
        INSERT INTO maps (hash, bsid, song_name, song_sub_name, song_author_name, level_author_name, bpm, cover_url, verified, created_at)
        VALUES (${hash}, ${bsid}, ${String(metadata.songName || map.name || 'Unknown')}, ${String(metadata.songSubName || '')}, ${String(metadata.songAuthorName || '')}, ${String(metadata.levelAuthorName || map.uploader?.name || '')}, ${Number(metadata.bpm || 0)}, ${coverUrl}, ${Boolean(map.verified || map.uploader?.verifiedMapper)}, COALESCE(${map.uploaded ? new Date(map.uploaded).toISOString() : null}::timestamptz, now()))
        ON CONFLICT (hash) DO UPDATE SET
          bsid=EXCLUDED.bsid, song_name=EXCLUDED.song_name, song_sub_name=EXCLUDED.song_sub_name,
          song_author_name=EXCLUDED.song_author_name, level_author_name=EXCLUDED.level_author_name,
          bpm=EXCLUDED.bpm, cover_url=EXCLUDED.cover_url, verified=EXCLUDED.verified
        RETURNING id`;
      const mapId = Number(inserted[0]?.id);
      if (!mapId) continue;

      const diffs = Array.isArray(version?.diffs) ? version.diffs : [];
      for (const diff of diffs) {
        const difficulty = beatSaverDifficultyValue(diff.difficulty);
        if (!difficulty) continue;
        const gameMode = beatSaverGameMode(diff.characteristic);
        const rawDifficulty = String(diff.difficulty || 'ExpertPlus');
        await sql`
          INSERT INTO leaderboards (map_id, difficulty, game_mode, raw_difficulty, max_score, stars, status, ranked_at)
          VALUES (${mapId}, ${difficulty}, ${gameMode}, ${rawDifficulty}, ${Number(diff.maxScore || 1000000)}, 0, 'UNRANKED', NULL)
          ON CONFLICT (map_id, difficulty, game_mode) DO UPDATE SET
            raw_difficulty=EXCLUDED.raw_difficulty, max_score=EXCLUDED.max_score`;
      }
      synced++;
    }

    const oldestUploaded = docs[docs.length - 1]?.uploaded;
    if (!oldestUploaded) {
      complete = true;
      break;
    }
    const nextBefore = new Date(oldestUploaded);
    if (Number.isNaN(nextBefore.getTime())) {
      complete = true;
      break;
    }
    const nextBeforeValue = nextBefore.toISOString();
    if (nextBeforeValue === before) {
      complete = true;
      break;
    }

    if (!bootstrapping && cutoff) {
      // We have reached the previous newest upload boundary. Keep the boundary
      // inclusive for one page so maps sharing the same timestamp are not missed.
      const allAtOrBeforeCutoff = docs.every((doc: any) => {
        const d = doc.uploaded ? new Date(doc.uploaded) : null;
        return !d || Number.isNaN(d.getTime()) || d <= cutoff;
      });
      if (allAtOrBeforeCutoff) {
        complete = true;
        break;
      }
    }

    before = nextBeforeValue;
    if (docs.length < pageSize) {
      complete = true;
      break;
    }
  }

  if (bootstrapping) {
    if (complete) {
      await sql`
        UPDATE beatsaver_sync_state
        SET last_sync_at=now(), maps_synced=${synced}, bootstrap_before=NULL,
            bootstrap_complete=true, newest_uploaded_at=${newestSeen ? newestSeen.toISOString() : null}
        WHERE id=1`;
    } else {
      await sql`
        UPDATE beatsaver_sync_state
        SET last_sync_at=now(), maps_synced=${synced}, bootstrap_before=${before}
        WHERE id=1`;
    }
  } else {
    await sql`
      UPDATE beatsaver_sync_state
      SET last_sync_at=now(), maps_synced=${synced}, newest_uploaded_at=${newestSeen ? newestSeen.toISOString() : (state.newest_uploaded_at || null)}
      WHERE id=1`;
  }

  return {
    synced,
    fetched,
    pages,
    source: 'beatsaver',
    bootstrapping,
    complete: bootstrapping ? complete : true,
    message: bootstrapping
      ? (complete ? 'Initial BeatSaver import is complete; future syncs only fetch new maps.' : 'Initial BeatSaver import is continuing from the saved cursor.')
      : 'Incremental BeatSaver sync complete; only new maps were checked.'
  };
}

async function ensureModerationTables(sql: any) {
  if (!sql) return;
  await sql`
    CREATE TABLE IF NOT EXISTS badges (
      id BIGSERIAL PRIMARY KEY,
      image TEXT NOT NULL,
      description TEXT NOT NULL,
      image_url TEXT NOT NULL DEFAULT '',
      created_at TIMESTAMPTZ NOT NULL DEFAULT now()
    )
  `;
  await sql`
    CREATE TABLE IF NOT EXISTS player_badges (
      player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
      badge_id BIGINT NOT NULL REFERENCES badges(id) ON DELETE CASCADE,
      description_override TEXT,
      added_at TIMESTAMPTZ NOT NULL DEFAULT now(),
      PRIMARY KEY (player_id, badge_id)
    )
  `;
  await sql`
    CREATE TABLE IF NOT EXISTS profile_reports (
      id BIGSERIAL PRIMARY KEY,
      reporter_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
      target_player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
      reason TEXT NOT NULL,
      details TEXT NOT NULL DEFAULT '',
      status TEXT NOT NULL DEFAULT 'OPEN',
      created_at TIMESTAMPTZ NOT NULL DEFAULT now()
    )
  `;
  await sql`
    CREATE TABLE IF NOT EXISTS account_merges (
      id BIGSERIAL PRIMARY KEY,
      target_player_id TEXT NOT NULL,
      source_player_id TEXT NOT NULL,
      reason TEXT NOT NULL,
      merged_by TEXT NOT NULL,
      created_at TIMESTAMPTZ NOT NULL DEFAULT now()
    )
  `;
  await sql`
    CREATE TABLE IF NOT EXISTS rank_requests (
      id BIGSERIAL PRIMARY KEY,
      map_id BIGINT NOT NULL REFERENCES maps(id) ON DELETE CASCADE,
      description TEXT NOT NULL DEFAULT '',
      request_type TEXT NOT NULL DEFAULT 'RANK',
      approval_status TEXT NOT NULL DEFAULT 'PENDING',
      weight DOUBLE PRECISION NOT NULL DEFAULT 1,
      created_by TEXT REFERENCES players(id) ON DELETE SET NULL,
      created_at TIMESTAMPTZ NOT NULL DEFAULT now()
    )
  `;
  await sql`
    CREATE TABLE IF NOT EXISTS rank_request_difficulties (
      id BIGSERIAL PRIMARY KEY,
      request_id BIGINT NOT NULL REFERENCES rank_requests(id) ON DELETE CASCADE,
      leaderboard_id BIGINT NOT NULL REFERENCES leaderboards(id) ON DELETE CASCADE,
      description TEXT NOT NULL DEFAULT '',
      approval_status TEXT NOT NULL DEFAULT 'PENDING',
      UNIQUE(request_id, leaderboard_id)
    )
  `;
  await sql`
    CREATE TABLE IF NOT EXISTS rank_request_votes (
      difficulty_id BIGINT NOT NULL REFERENCES rank_request_difficulties(id) ON DELETE CASCADE,
      player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
      group_name TEXT NOT NULL,
      vote TEXT NOT NULL,
      created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
      PRIMARY KEY(difficulty_id, player_id, group_name)
    )
  `;
  await sql`
    CREATE TABLE IF NOT EXISTS rank_request_comments (
      id BIGSERIAL PRIMARY KEY,
      difficulty_id BIGINT NOT NULL REFERENCES rank_request_difficulties(id) ON DELETE CASCADE,
      player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
      group_name TEXT NOT NULL,
      comment TEXT NOT NULL,
      edited BOOLEAN NOT NULL DEFAULT false,
      created_at TIMESTAMPTZ NOT NULL DEFAULT now()
    )
  `;
  await sql`ALTER TABLE players ADD COLUMN IF NOT EXISTS vanity_changed_at TIMESTAMPTZ`;
  const countRows: any[] = await sql`SELECT COUNT(*)::int AS count FROM badges`;
  await sql`
    CREATE TABLE IF NOT EXISTS profile_customizations (
      player_id TEXT PRIMARY KEY REFERENCES players(id) ON DELETE CASCADE,
      background_image TEXT,
      background_image_version BIGINT NOT NULL DEFAULT 1,
      accent_color TEXT,
      accent_foreground_color TEXT,
      accent_foreground_active_color TEXT,
      supporter_name_color_enabled BOOLEAN NOT NULL DEFAULT true,
      badge_order BIGINT[],
      badge_comments JSONB,
      stat_order TEXT[],
      enabled_stat_ids TEXT[],
      chart_metric_ids TEXT[],
      section_order TEXT[],
      updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
    )
  `;
  await sql`
    CREATE TABLE IF NOT EXISTS pinned_scores (
      player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
      score_id BIGINT NOT NULL REFERENCES scores(id) ON DELETE CASCADE,
      position INTEGER NOT NULL,
      comment TEXT NOT NULL DEFAULT '',
      PRIMARY KEY (player_id, score_id)
    )
  `;
  await sql`UPDATE badges SET description='SnoreSaber Tester', image='tester.png', image_url='/assets/badges/tester.png' WHERE description='Early Supporter' AND NOT EXISTS (SELECT 1 FROM badges WHERE description='SnoreSaber Tester')`;
  const defaultBadges = [
    ['staff.png', 'SnoreSaber Staff', '/assets/badges/staff.png'],
    ['tester.png', 'SnoreSaber Tester', '/assets/badges/tester.png'],
    ['verified.png', 'Verified Player', '/assets/badges/verified.png'],
    ['mapper.png', 'Map Contributor', '/assets/badges/mapper.png'],
    ['tournament.png', 'Tournament Staff', '/assets/badges/tournament.png'],
    ['developer.png', 'SnoreSaber Developer', '/assets/badges/developer.png']
  ];
  for (const [image, description, imageUrl] of defaultBadges) {
    const existing:any[] = await sql`SELECT id FROM badges WHERE description=${description} LIMIT 1`;
    if (existing[0]) {
      await sql`UPDATE badges SET image=${image}, image_url=${imageUrl} WHERE id=${existing[0].id}`;
    } else {
      await sql`INSERT INTO badges (image, description, image_url) VALUES (${image}, ${description}, ${imageUrl})`;
    }
  }
}

async function getPlayerBadges(sql: any, playerId: string) {
  if (!sql) return [];
  await ensureModerationTables(sql);
  const rows: any[] = await sql`
    SELECT b.id, b.image, b.description, COALESCE(NULLIF(b.image_url,''), b.image) AS image_url,
           pb.description_override
    FROM player_badges pb
    JOIN badges b ON b.id=pb.badge_id
    WHERE pb.player_id=${playerId}
    ORDER BY pb.added_at ASC, b.id ASC
  `;

  const playerRows: any[] = await sql`SELECT role, permissions FROM players WHERE id=${playerId} LIMIT 1`;
  const playerRole = String(playerRows[0]?.role || '').trim().toLowerCase();
  const playerPermissions = Number(playerRows[0]?.permissions || 0);
  const automaticallyGrantedDescriptions: string[] = [];

  // These badges are role badges, so they are always shown for the matching role
  // even when the player has never been manually assigned the badge.
  if (playerRole.includes('tester')) automaticallyGrantedDescriptions.push('SnoreSaber Tester');
  if (playerRole.includes('developer') || (playerPermissions & 256) !== 0 || (playerPermissions & 16384) !== 0) {
    automaticallyGrantedDescriptions.push('SnoreSaber Developer');
  }

  if (automaticallyGrantedDescriptions.length > 0) {
    const autoRows: any[] = await sql`
      SELECT id, image, description, COALESCE(NULLIF(image_url,''), image) AS image_url
      FROM badges
      WHERE description = ANY(${automaticallyGrantedDescriptions})
      ORDER BY id ASC
    `;
    const existingIds = new Set(rows.map((r) => Number(r.id)));
    for (const row of autoRows) {
      if (!existingIds.has(Number(row.id))) {
        rows.push({ ...row, description_override: null });
      }
    }
  }

  const badgeAsset = (description: string, image: string) => {
    const key = String(description || '').toLowerCase();
    if (key === 'snoresaber developer') return '/assets/badges/developer.png';
    if (key === 'snoresaber tester') return '/assets/badges/tester.png';
    if (key === 'snoresaber staff') return '/assets/badges/staff.png';
    if (key === 'verified player') return '/assets/badges/verified.png';
    if (key === 'map contributor') return '/assets/badges/mapper.png';
    if (key === 'tournament staff') return '/assets/badges/tournament.png';
    return String(image || '');
  };
  return rows.map((r) => ({
    id: Number(r.id),
    image: badgeAsset(r.description, r.image_url || r.image),
    description: r.description_override || r.description
  }));
}


async function getProfileCustomization(sql: any, playerId: string) {
  if (!sql) return { backgroundImage:null, backgroundImageVersion:null, accentColor:'#f06ab7', accentForegroundColor:'#160d16', accentForegroundActiveColor:'#ffffff', supporterNameColorEnabled:true, badgeOrder:null, badgeComments:null, statOrder:null, enabledStatIds:null, chartMetricIds:null, sectionOrder:null };
  await ensureModerationTables(sql);
  const rows:any[] = await sql`SELECT * FROM profile_customizations WHERE player_id=${playerId} LIMIT 1`;
  if (!rows[0]) return { backgroundImage:null, backgroundImageVersion:null, accentColor:'#f06ab7', accentForegroundColor:'#160d16', accentForegroundActiveColor:'#ffffff', supporterNameColorEnabled:true, badgeOrder:null, badgeComments:null, statOrder:null, enabledStatIds:null, chartMetricIds:null, sectionOrder:null };
  const r=rows[0];
  return {
    backgroundImage:r.background_image || null,
    backgroundImageVersion:r.background_image_version == null ? null : Number(r.background_image_version),
    accentColor:r.accent_color || '#f06ab7',
    accentForegroundColor:r.accent_foreground_color || '#160d16',
    accentForegroundActiveColor:r.accent_foreground_active_color || '#ffffff',
    supporterNameColorEnabled:r.supporter_name_color_enabled !== false,
    badgeOrder:Array.isArray(r.badge_order) ? r.badge_order.map(Number) : null,
    badgeComments:r.badge_comments || null,
    statOrder:r.stat_order || null,
    enabledStatIds:r.enabled_stat_ids || null,
    chartMetricIds:r.chart_metric_ids || null,
    sectionOrder:r.section_order || null
  };
}

async function getPinnedScores(sql:any, playerId:string) {
  if (!sql) return [];
  await ensureModerationTables(sql);
  const rows:any[] = await sql`
    SELECT ps.position, ps.comment, s.*, p.id AS p_id,p.name,p.country,p.avatar,
      l.id AS lb_id,l.difficulty,l.game_mode,l.raw_difficulty,l.max_score,l.stars,l.status,l.created_at AS lb_created_at,
      m.id AS map_id,m.hash AS map_hash,m.bsid AS map_bsid,m.song_name,m.song_sub_name,m.song_author_name,m.level_author_name,m.bpm,m.cover_url,m.verified,m.created_at AS map_created_at,
      RANK() OVER (PARTITION BY s.leaderboard_id ORDER BY s.score DESC, s.accuracy DESC, s.created_at ASC) AS board_rank
    FROM pinned_scores ps
    JOIN scores s ON s.id=ps.score_id
    JOIN players p ON p.id=s.player_id
    JOIN leaderboards l ON l.id=s.leaderboard_id
    JOIN maps m ON m.id=l.map_id
    WHERE ps.player_id=${playerId}
    ORDER BY ps.position ASC
    LIMIT 6
  `;
  return rows.map((r)=>({
    score: dbScore(r,
      {id:r.p_id,name:r.name,country:r.country,avatar:r.avatar},
      {id:r.lb_id,difficulty:r.difficulty,game_mode:r.game_mode,raw_difficulty:r.raw_difficulty,max_score:r.max_score,stars:r.stars,status:r.status,created_at:r.lb_created_at},
      {id:r.map_id,hash:r.map_hash,bsid:r.map_bsid,song_name:r.song_name,song_sub_name:r.song_sub_name,song_author_name:r.song_author_name,level_author_name:r.level_author_name,bpm:r.bpm,cover_url:r.cover_url,verified:r.verified,created_at:r.map_created_at},
      Number(r.board_rank||1), true),
    comment:String(r.comment||'')
  }));
}

function dbPlayer(r: any) {
  const publicId = String(r.id);
  return {
    id: publicId,
    playerId: publicId,
    steamId: r.steam_id || null, name: r.name, playerNameInGame: r.name, role: r.role ?? null, avatar: r.avatar || '', avatarVersion: 1,
    bio: r.bio ?? null, country: r.country || 'XX', permissions: Number(r.permissions || 0), banned: Boolean(r.banned), silenced: Boolean(r.silenced), inactive: false,
    // Vanity is optional. Never synthesize one from the display name: doing so makes every
    // player appear to have a vanity URL even when they have not enabled one in settings.
    vanity: r.vanity || null, publicLivePresenceOptOut: false,
    stats: {
      realmId: 1, realmName: 'SnoreSaber', rank: Number(r.rank || 0), countryRank: Number(r.country_rank || 0), rankChange: 0,
      totalPP: Number(r.pp || 0), plusOnePP: Number(r.pp || 0), totalScore: String(r.total_score || 0), totalRankedScore: String(r.total_ranked_score || 0),
      totalPlayedLeaderboards: Number(r.total_plays || 0), totalPlayedRankedLeaderboards: Number(r.total_ranked_plays || 0),
      totalSubmittedPlays: Number(r.total_plays || 0), totalReplayViews: 0, averageAccuracy: Number(r.average_accuracy || 0),
      weightedAverageAccuracy: Number(r.average_accuracy || 0), completionAccuracy: Number(r.average_accuracy || 0),
      device: { hmd: null, controllerLeft: null, controllerRight: null }
    },
    profileCustomization: {
      backgroundImage: null, backgroundImageVersion: null, accentColor: '#f06ab7',
      accentForegroundColor: '#160d16', accentForegroundActiveColor: '#ffffff',
      supporterNameColorEnabled: false, badgeOrder: null, badgeComments: null,
      statOrder: null, enabledStatIds: null, chartMetricIds: null, sectionOrder: null
    },
    createdAt: r.created_at ? new Date(r.created_at).toISOString() : NOW(),
    lastSeenAt: r.last_seen_at ? new Date(r.last_seen_at).toISOString() : NOW(),
    badges: [],
    relationships: { following: [], mutuals: [] }
  };
}

function dbMap(r: any, lbs: any[] = []) {
  return {
    id: Number(r.id), hash: r.hash, bsid: r.bsid ?? null, songName: r.song_name, songSubName: r.song_sub_name || '',
    songAuthorName: r.song_author_name || '', levelAuthorName: r.level_author_name || '', bpm: Number(r.bpm || 0),
    coverUrl: r.cover_url || '/assets/snoresaber-icon.png', verified: Boolean(r.verified), totalScores: Number(r.total_scores || 0),
    dailyScores: Number(r.daily_scores || 0), createdAt: new Date(r.created_at).toISOString(), leaderboards: lbs,
    // Re-upload data is not currently mirrored in the local schema. The frontend expects
    // this collection to always exist because it builds the version selector with flatMap().
    reuploadVersions: []
  };
}

function dbLeaderboard(r: any, totalScores = 0) {
  const status = r.status || 'UNRANKED';
  return {
    id: Number(r.id), difficulty: Number(r.difficulty), gameMode: r.game_mode, rawDifficulty: r.raw_difficulty,
    maxScore: Number(r.max_score || 1000000), totalScores, dailyScores: 0, createdAt: new Date(r.created_at).toISOString(),
    realm: realm(Number(r.stars || 0), status)
  };
}

function dbScore(r: any, p: any, lb: any, mapRow: any, rank: number, personalBest: boolean) {
  return {
    id: Number(r.id), rank, unmodifiedScore: Number(r.score), modifiedScore: Number(r.score), accuracy: Number(r.accuracy),
    pp: Number(r.pp), weight: Number(r.weight || 1), mods: r.mods ? String(r.mods).split(',').filter(Boolean) : [], badCuts: Number(r.bad_cuts || 0),
    missedNotes: Number(r.missed_notes || 0), maxCombo: Number(r.max_combo || 0), fullCombo: Boolean(r.full_combo), hasReplay: Boolean(r.has_replay),
    replayViewCount: 0, personalBest, legacyHmdId: null, version: '1.40.8', playOutcome: 'CLEAR', playOutcomeTime: null,
    createdAt: new Date(r.created_at).toISOString(), hasHistory: false,
    player: { id: p.id, name: p.name, playerNameInGame: p.name, country: p.country, role: null, avatar: p.avatar || '', avatarVersion: 1, permissions: 0 },
    device: { hmd: null, controllerLeft: null, controllerRight: null },
    leaderboard: dbLeaderboard(lb, Number(lb.total_scores || 0)),
    map: dbMap(mapRow)
  };
}

async function rankRequestDifficultyVotes(sql: any, difficultyId: number, viewerId: string | null) {
  const rows: any[] = await sql`
    SELECT group_name, vote, COUNT(*)::int AS count
    FROM rank_request_votes
    WHERE difficulty_id=${difficultyId}
    GROUP BY group_name, vote
  `;
  const get = (group: string, vote: string) => Number(rows.find((r) => r.group_name === group && r.vote === vote)?.count || 0);
  const myRows: any[] = viewerId ? await sql`SELECT group_name,vote FROM rank_request_votes WHERE difficulty_id=${difficultyId} AND player_id=${viewerId}` : [];
  const my = (group: string) => myRows.find((r) => r.group_name === group)?.vote || null;
  return {
    rtVotes: { upvotes: get('RT','UPVOTE'), downvotes: get('RT','DOWNVOTE'), myVote: my('RT') },
    qatVotes: { upvotes: get('QAT','UPVOTE'), downvotes: get('QAT','DOWNVOTE'), neutrals: get('QAT','NEUTRAL'), myVote: my('QAT') }
  };
}

async function rankRequestDifficultyDetails(sql: any, row: any, viewerId: string | null) {
  const lb = await sql`SELECT l.*, m.* FROM leaderboards l JOIN maps m ON m.id=l.map_id WHERE l.id=${row.leaderboard_id} LIMIT 1`;
  if (!lb[0]) return null;
  const r = lb[0];
  const scoreCount: any[] = await sql`SELECT COUNT(*)::int AS count FROM scores WHERE leaderboard_id=${r.leaderboard_id || r.id}`;
  const votes = await rankRequestDifficultyVotes(sql, Number(row.id), viewerId);
  const comments: any[] = await sql`
    SELECT c.id,c.comment,c.created_at,c.edited,c.group_name,p.id AS p_id,p.name,p.country,p.avatar,p.role,p.permissions
    FROM rank_request_comments c JOIN players p ON p.id=c.player_id
    WHERE c.difficulty_id=${row.id}
    ORDER BY c.created_at ASC,c.id ASC
  `;
  const mapObj = dbMap(r);
  const leaderboardObj = {
    id:Number(r.id), map:mapObj,
    difficulty:{id:Number(r.id),difficulty:Number(r.difficulty),rawDifficulty:r.raw_difficulty,gameMode:r.game_mode},
    maxScore:Number(r.max_score||1000000), totalScores:Number(scoreCount[0]?.count||0), dailyScores:0,
    createdAt:new Date(r.created_at).toISOString(), realm:realm(Number(r.stars||0),r.status)
  };
  return {
    id:Number(row.id), description:String(row.description||''), approvalStatus:row.approval_status,
    leaderboard:leaderboardObj,
    rtVotes:votes.rtVotes, qatVotes:votes.qatVotes,
    rtComments:comments.filter((c)=>c.group_name==='RT').map((c)=>({id:Number(c.id),player:{id:String(c.p_id),name:c.name,playerNameInGame:c.name,country:c.country||'XX',role:c.role||null,avatar:c.avatar||'',avatarVersion:1,permissions:Number(c.permissions||0)},comment:c.comment,createdAt:new Date(c.created_at).toISOString(),edited:Boolean(c.edited)})),
    qatComments:comments.filter((c)=>c.group_name==='QAT').map((c)=>({id:Number(c.id),player:{id:String(c.p_id),name:c.name,playerNameInGame:c.name,country:c.country||'XX',role:c.role||null,avatar:c.avatar||'',avatarVersion:1,permissions:Number(c.permissions||0)},comment:c.comment,createdAt:new Date(c.created_at).toISOString(),edited:Boolean(c.edited)}))
  };
}

async function getRankRequestDetails(sql: any, requestId: number, viewerId: string | null) {
  const rows:any[] = await sql`SELECT rr.*,m.* FROM rank_requests rr JOIN maps m ON m.id=rr.map_id WHERE rr.id=${requestId} LIMIT 1`;
  if (!rows[0]) return null;
  const r=rows[0];
  const diffRows:any[] = await sql`SELECT * FROM rank_request_difficulties WHERE request_id=${requestId} ORDER BY id ASC`;
  const difficulties=[];
  for (const d of diffRows) { const detail=await rankRequestDifficultyDetails(sql,d,viewerId); if(detail) difficulties.push(detail); }
  return {
    id:Number(r.id), description:String(r.description||''), requestType:r.request_type, approvalStatus:r.approval_status,
    replacedBy:null, replacedFrom:null, weight:Number(r.weight||1), createdAt:new Date(r.created_at).toISOString(), map:dbMap(r),
    difficulties, commentsObfuscated:false
  };
}

async function getRankRequestSummary(sql:any, requestId:number, viewerId:string|null) {
  const r:any[] = await sql`SELECT rr.*,m.* FROM rank_requests rr JOIN maps m ON m.id=rr.map_id WHERE rr.id=${requestId} LIMIT 1`;
  if(!r[0]) return null;
  const row=r[0];
  const diffs:any[]=await sql`SELECT * FROM rank_request_difficulties WHERE request_id=${requestId} ORDER BY id ASC`;
  let rtUp=0,rtDown=0,qatUp=0,qatDown=0,qatNeutral=0,readyVotes=0;
  for(const d of diffs){ const v=await rankRequestDifficultyVotes(sql,Number(d.id),viewerId); rtUp+=v.rtVotes.upvotes;rtDown+=v.rtVotes.downvotes;qatUp+=v.qatVotes.upvotes;qatDown+=v.qatVotes.downvotes;qatNeutral+=v.qatVotes.neutrals; if(v.rtVotes.upvotes>=2 && v.rtVotes.downvotes===0) readyVotes++; }
  const missing=Math.max(0,2-rtUp);
  const readiness=rtDown>0?'BLOCKED':missing===0?'READY':missing===1?'CLOSE':'QUEUED';
  return {
    id:Number(row.id),description:String(row.description||''),requestType:row.request_type,approvalStatus:row.approval_status,weight:Number(row.weight||1),createdAt:new Date(row.created_at).toISOString(),
    map:dbMap(row),difficultyCount:diffs.length,
    rtVoteReadiness:{status:readiness,missingUpvotes:missing,downvotes:rtDown},
    totalRtVotes:{upvotes:rtUp,downvotes:rtDown,myVote:diffs.map(async()=>null) && null},
    totalQatVotes:{upvotes:qatUp,downvotes:qatDown,neutrals:qatNeutral,myVote:null}
  };
}

async function dbMapWithLeaderboards(sql: any, mapId: number) {
  const mapsRows: any[] = await sql`SELECT * FROM maps WHERE id=${mapId} LIMIT 1`;
  if (!mapsRows[0]) return null;
  const lbRows: any[] = await sql`
    SELECT l.*, COUNT(s.id)::int AS total_scores
    FROM leaderboards l LEFT JOIN scores s ON s.leaderboard_id=l.id
    WHERE l.map_id=${mapId}
    GROUP BY l.id ORDER BY l.difficulty ASC, l.id ASC`;
  const mapData:any = dbMap(mapsRows[0], lbRows.map((x) => dbLeaderboard(x, Number(x.total_scores || 0))));
  const active:any[] = await sql`SELECT id FROM rank_requests WHERE map_id=${mapId} AND approval_status NOT IN ('DENIED','REPLACED') ORDER BY created_at DESC LIMIT 1`;
  if (active[0]) mapData.rankRequest = await getRankRequestDetails(sql, Number(active[0].id), null);
  else mapData.rankRequest = null;
  return mapData;
}

async function resolveInternalPlayerId(sql: any, publicOrInternalId: string) {
  if (!sql) return publicOrInternalId;
  const rows: any[] = await sql`SELECT id FROM players WHERE id=${publicOrInternalId} OR steam_id=${publicOrInternalId} LIMIT 1`;
  return rows[0]?.id ?? null;
}

async function recalculateLeaderboardPlayers(sql: any, leaderboardId: number) {
  const lbRows: any[] = await sql`SELECT stars, max_score, status FROM leaderboards WHERE id=${leaderboardId} LIMIT 1`;
  if (!lbRows[0]) return 0;
  const lb = lbRows[0];
  const maxPP = Number(lb.stars || 0) * 450 / 10.685333512;

  // Reweight every score on this leaderboard from its stored accuracy. This keeps
  // recalculation deterministic and makes a star/weight change immediately visible.
  await sql`
    UPDATE scores
    SET pp = ROUND((${maxPP}) * GREATEST(0, LEAST(1, accuracy / 100.0)), 2),
        weight = CASE WHEN ${String(lb.status)} = 'RANKED' THEN 1 ELSE 0 END
    WHERE leaderboard_id=${leaderboardId}
  `;

  const players: any[] = await sql`SELECT DISTINCT player_id FROM scores WHERE leaderboard_id=${leaderboardId}`;
  for (const row of players) await recalculatePlayerStats(sql, String(row.player_id));
  return players.length;
}

async function recalculatePlayerStats(sql: any, playerId: string) {
  const rows: any[] = await sql`
    SELECT s.player_id, s.leaderboard_id, s.score, s.accuracy, s.pp, s.created_at, l.status
    FROM scores s JOIN leaderboards l ON l.id=s.leaderboard_id
    WHERE s.player_id=${playerId}
    ORDER BY s.pp DESC, s.score DESC, s.created_at DESC`;

  const bestAll = new Map<number, any>();
  const bestRanked = new Map<number, any>();
  for (const row of rows) {
    const id = Number(row.leaderboard_id);
    if (!bestAll.has(id)) bestAll.set(id, row);
    if (row.status === 'RANKED' && !bestRanked.has(id)) bestRanked.set(id, row);
  }

  const rankedBest = [...bestRanked.values()].sort((a, b) => Number(b.pp) - Number(a.pp));
  let pp = 0;
  let rankedScore = 0;
  let allScore = 0;
  let accuracySum = 0;
  for (const [index, row] of rankedBest.entries()) {
    pp += Number(row.pp || 0) * Math.pow(0.965, index);
    rankedScore += Number(row.score || 0);
    accuracySum += Number(row.accuracy || 0);
  }
  for (const row of bestAll.values()) allScore += Number(row.score || 0);

  const average = rankedBest.length ? accuracySum / rankedBest.length : 0;
  const totalPlays = rows.length;
  const rankedPlays = rows.filter((r) => r.status === 'RANKED').length;
  const totalLeaderboards = bestAll.size;
  const rankedLeaderboards = bestRanked.size;

  await sql`
    UPDATE players SET
      pp=${pp},
      total_score=${Math.round(allScore)},
      total_ranked_score=${Math.round(rankedScore)},
      total_plays=${totalPlays},
      total_ranked_plays=${rankedPlays},
      average_accuracy=${average},
      last_seen_at=now()
    WHERE id=${playerId}`;

  const all: any[] = await sql`SELECT id, country, pp FROM players ORDER BY pp DESC, id ASC`;
  const countryCounters = new Map<string, number>();
  for (let i = 0; i < all.length; i++) {
    const country = all[i].country || 'XX';
    const cr = (countryCounters.get(country) || 0) + 1;
    countryCounters.set(country, cr);
    await sql`UPDATE players SET rank=${i + 1}, country_rank=${cr} WHERE id=${all[i].id}`;
  }
}

export default defineHandler(async (event: any) => {
  const request: Request = event.req;
  const method = request.method || 'GET';
  const rawUrl = request.url || 'https://snoresaber.vercel.app/api/v2/health';
  const host = request.headers.get('x-forwarded-host') || request.headers.get('host') || 'snoresaber.vercel.app';
  const proto = request.headers.get('x-forwarded-proto') || 'https';
  const origin = `${proto}://${host}`;
  const url = new URL(rawUrl, origin);
  const path = url.pathname.replace(/^\/+/, '');
  const parts = path.split('/').filter(Boolean);
  const query = url.searchParams;

  if (!path.startsWith('api/v2/')) return json({ error: 'SnoreSaber API route not found' }, 404);
  const route = '/' + parts.slice(2).join('/');
  const sql = db();

  if (route === '/health') return json({ ok: true, service: 'SnoreSaber API', version: '3.0.0', database: Boolean(sql) });

  // ------------------------- RANKINGS -------------------------
  // Rankings are backed by the same SnoreSaber-owned players table as /players.
  // The frontend uses /players, while the public API also exposes /rankings.
  if ((route === '/rankings' || route === '/players') && method === 'GET') {
    const page = Math.max(1, Number(query.get('page') || 1));
    const limit = Math.min(100, Math.max(1, Number(query.get('limit') || 50)));
    const search = (query.get('search') || '').trim().toLowerCase();
    const countries = (query.get('countries') || '')
      .split(',')
      .map((x) => x.trim().toUpperCase())
      .filter(Boolean);
    const sort = query.get('sort') || 'rank';
    const direction = query.get('sortDirection') || 'asc';

    const sortValue = (p: any) => {
      const stats = p.stats || {};
      switch (sort) {
        case 'countryRank': return Number(stats.countryRank ?? p.country_rank ?? 0);
        case 'totalPP': return Number(stats.totalPP ?? p.pp ?? 0);
        case 'totalScore': return Number(stats.totalScore ?? p.total_score ?? 0);
        case 'totalRankedScore': return Number(stats.totalRankedScore ?? p.total_ranked_score ?? 0);
        case 'totalPlayedLeaderboards': return Number(stats.totalPlayedLeaderboards ?? 0);
        case 'totalPlayedRankedLeaderboards': return Number(stats.totalPlayedRankedLeaderboards ?? 0);
        case 'totalSubmittedPlays': return Number(stats.totalSubmittedPlays ?? p.total_plays ?? 0);
        case 'totalReplayViews': return Number(stats.totalReplayViews ?? 0);
        case 'averageAccuracy': return Number(stats.averageAccuracy ?? p.average_accuracy ?? 0);
        case 'weightedAverageAccuracy': return Number(stats.weightedAverageAccuracy ?? p.average_accuracy ?? 0);
        case 'completionAccuracy': return Number(stats.completionAccuracy ?? p.average_accuracy ?? 0);
        default: return Number(stats.rank ?? p.rank ?? 0);
      }
    };

    if (sql) {
      const rows: any[] = await sql`SELECT * FROM players`;
      let filtered = rows.filter((p) => {
        const matchesSearch = !search || p.name.toLowerCase().includes(search) || String(p.id).includes(search) || String(p.steam_id || '').includes(search);
        const matchesCountry = !countries.length || countries.includes(String(p.country || 'XX').toUpperCase());
        return matchesSearch && matchesCountry;
      });
      filtered.sort((a, b) => {
        const av = sortValue(dbPlayer(a));
        const bv = sortValue(dbPlayer(b));
        const primary = direction === 'desc' ? bv - av : av - bv;
        return primary || String(a.id).localeCompare(String(b.id));
      });
      const start = (page - 1) * limit;
      return json({
        data: filtered.slice(start, start + limit).map(dbPlayer),
        metadata: metadata(filtered.length, page, limit)
      });
    }

    let filtered = players.filter((p) => {
      const matchesSearch = !search || p.name.toLowerCase().includes(search) || p.id.includes(search);
      const matchesCountry = !countries.length || countries.includes(String(p.country || 'XX').toUpperCase());
      return matchesSearch && matchesCountry;
    });
    filtered.sort((a, b) => {
      const primary = direction === 'desc' ? sortValue(b) - sortValue(a) : sortValue(a) - sortValue(b);
      return primary || String(a.id).localeCompare(String(b.id));
    });
    const start = (page - 1) * limit;
    return json({ data: filtered.slice(start, start + limit), metadata: metadata(filtered.length, page, limit) });
  }

  // ------------------------- PLAYERS -------------------------
  if (route === '/players-legacy-unused' && method === 'GET') {
    const page = Math.max(1, Number(query.get('page') || 1));
    const limit = Math.min(100, Math.max(1, Number(query.get('limit') || 50)));
    const search = (query.get('search') || '').toLowerCase();
    const sort = query.get('sort') || 'rank';
    if (sql) {
      const rows: any[] = await sql`SELECT * FROM players ORDER BY pp DESC, id ASC`;
      let filtered = rows.filter((p) => !search || p.name.toLowerCase().includes(search) || String(p.id).includes(search));
      if (sort === 'name') filtered.sort((a, b) => a.name.localeCompare(b.name));
      const start = (page - 1) * limit;
      const data = filtered.slice(start, start + limit).map(dbPlayer);
      return json({ data, metadata: metadata(filtered.length, page, limit) });
    }
    let filtered = players.filter((p) => !search || p.name.toLowerCase().includes(search) || p.id.includes(search));
    filtered.sort((a, b) => sort === 'name' ? a.name.localeCompare(b.name) : b.stats.totalPP - a.stats.totalPP);
    const start = (page - 1) * limit;
    return json({ data: filtered.slice(start, start + limit), metadata: metadata(filtered.length, page, limit) });
  }
  if (route === '/players/count' && method === 'GET') {
    if (sql) { const rows: any[] = await sql`SELECT COUNT(*)::int AS count FROM players`; return json({ count: Number(rows[0]?.count || 0) }); }
    return json({ count: players.length });
  }
  if (route.startsWith('/players/vanity/') && method === 'GET') {
    const slug = decodeURIComponent(route.split('/').pop()!).toLowerCase();
    if (sql) {
      const rows: any[] = await sql`SELECT * FROM players WHERE lower(vanity)=${slug} OR lower(name)=${slug} OR lower(id)=${slug} OR lower(steam_id)=${slug} LIMIT 1`;
      return rows[0] ? json(dbPlayer(rows[0])) : json({ statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found' },404);
    }
    const p = players.find((x) => x.vanity === slug || x.name.toLowerCase() === slug);
    return p ? json(p) : json({ statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found' },404);
  }
  if (route.startsWith('/players/') && route.endsWith('/profile') && method === 'GET') {
    const seg = route.split('/');
    const requestedId = decodeURIComponent(seg[2]);
    if (sql) {
      const resolvedId = await resolvePlayerId(sql, requestedId);
      const pr: any[] = resolvedId ? await sql`SELECT * FROM players WHERE id=${resolvedId} LIMIT 1` : [];
      if (!pr[0]) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
      const p = dbPlayer(pr[0]);
      const viewerId = await authPlayerId(request, sql);
      const rel = await relationshipSummary(sql, pr[0].id, viewerId);
      const badges = await getPlayerBadges(sql, pr[0].id);
      const profileCustomization = await getProfileCustomization(sql, pr[0].id);
      const pinnedScores = await getPinnedScores(sql, pr[0].id);
      // Do not assume optional play-count columns exist. Older SnoreSaber databases
      // may have the original players schema, so derive play counts from scores.
      const playRows: any[] = await sql`
        SELECT s.leaderboard_id, l.status
        FROM scores s JOIN leaderboards l ON l.id=s.leaderboard_id
        WHERE s.player_id=${pr[0].id}`;
      const totalPlays = playRows.length;
      const rankedPlays = playRows.filter((row) => row.status === 'RANKED').length;
      const playedLeaderboards = new Set(playRows.map((row) => String(row.leaderboard_id))).size;
      const rankedLeaderboards = new Set(playRows.filter((row) => row.status === 'RANKED').map((row) => String(row.leaderboard_id))).size;
      const h = pr[0];
      const history = [{
        rank: Number(h.rank || 0), totalPP: Number(h.pp || 0), totalScore: String(h.total_score || 0),
        totalRankedScore: String(h.total_ranked_score || 0), totalPlayedLeaderboards: playedLeaderboards,
        totalPlayedRankedLeaderboards: rankedLeaderboards, totalSubmittedPlays: totalPlays,
        totalReplayViews: 0, averageAccuracy: Number(h.average_accuracy || 0),
        weightedAverageAccuracy: Number(h.average_accuracy || 0), completionAccuracy: Number(h.average_accuracy || 0),
        estimated: true, createdAt: new Date(h.created_at || Date.now()).toISOString()
      }];
      return json({ player: { ...p, badges, pinnedScores, profileCustomization, followers: rel.followers, following: rel.following, platformFriends: rel.platformFriends, recentFollowers: rel.recentFollowers, recentFollowing: rel.recentFollowing }, history, aliases: [] });
    }
    const p = players.find((x) => x.id === requestedId || x.steamId === requestedId || x.name.toLowerCase() === requestedId.toLowerCase());
    if (!p) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    return json({ player: { ...p, pinnedScores: [], followers: 0, following: 0, platformFriends: 0, recentFollowers: [], recentFollowing: [] }, history: [], aliases: [] });
  }

  // ------------------------- PLAYER RELATIONSHIPS -------------------------
  if (route.startsWith('/player/') && route.endsWith('/relationships') && method === 'GET') {
    const seg = route.split('/');
    const requestedId = decodeURIComponent(seg[2]);
    const type = query.get('type') || 'followers';
    const page = Math.max(1, Number(query.get('page') || 1));
    const limit = Math.min(100, Math.max(1, Number(query.get('limit') || 20)));
    if (!sql) return json({ data: [], metadata: metadata(0, page, limit) });
    const tableCheck: any[] = await sql`SELECT to_regclass('public.player_follows') AS table_name`;
    if (!tableCheck[0]?.table_name) return json({ data: [], metadata: metadata(0, page, limit) });
    const targetIdResolved = await resolvePlayerId(sql, requestedId);
    const targetRows: any[] = targetIdResolved ? await sql`SELECT id FROM players WHERE id=${targetIdResolved} LIMIT 1` : [];
    if (!targetRows[0]) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    const targetId = targetRows[0].id;
    let rows: any[];
    if (type === 'following') {
      rows = await sql`SELECT p.id,p.name,p.country,p.avatar,p.role,p.permissions FROM player_follows f JOIN players p ON p.id=f.following_id WHERE f.follower_id=${targetId} ORDER BY f.created_at DESC`;
    } else if (type === 'platform-friends') {
      rows = [];
    } else {
      rows = await sql`SELECT p.id,p.name,p.country,p.avatar,p.role,p.permissions FROM player_follows f JOIN players p ON p.id=f.follower_id WHERE f.following_id=${targetId} ORDER BY f.created_at DESC`;
    }
    const data = rows.slice((page-1)*limit, (page-1)*limit+limit).map((r) => ({
      player: { id:String(r.id), name:r.name, playerNameInGame:r.name, country:r.country || 'XX', role:r.role ?? null, avatar:r.avatar || '', avatarVersion:1, permissions:Number(r.permissions || 0) },
      relation: 'follow'
    }));
    return json({data,metadata:metadata(rows.length,page,limit)});
  }

  if (route.startsWith('/player/') && route.endsWith('/follow') && method === 'POST') {
    const requestedId = decodeURIComponent(route.split('/')[2]);
    const viewerId = await authPlayerId(request, sql);
    if (!viewerId) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if (!sql) return json({success:true});
    const targetIdResolved = await resolvePlayerId(sql, requestedId);
    const targetRows: any[] = targetIdResolved ? await sql`SELECT id FROM players WHERE id=${targetIdResolved} LIMIT 1` : [];
    if (!targetRows[0]) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    const targetId = targetRows[0].id;
    if (targetId === viewerId) return json({statusCode:400,error:'Bad Request',code:'INVALID_OPERATION',message:'You cannot follow yourself'},400);
    await sql`INSERT INTO player_follows (follower_id, following_id) VALUES (${viewerId},${targetId}) ON CONFLICT (follower_id, following_id) DO NOTHING`;
    return json({success:true});
  }

  if (route.startsWith('/player/') && route.endsWith('/unfollow') && method === 'POST') {
    const requestedId = decodeURIComponent(route.split('/')[2]);
    const viewerId = await authPlayerId(request, sql);
    if (!viewerId) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if (!sql) return json({success:true});
    const targetIdResolved = await resolvePlayerId(sql, requestedId);
    const targetRows: any[] = targetIdResolved ? await sql`SELECT id FROM players WHERE id=${targetIdResolved} LIMIT 1` : [];
    if (!targetRows[0]) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    await sql`DELETE FROM player_follows WHERE follower_id=${viewerId} AND following_id=${targetRows[0].id}`;
    return json({success:true});
  }

  if (route.startsWith('/players/') && method === 'GET') {
    const seg = route.split('/'); const id = decodeURIComponent(seg[2]);
    if (sql) {
      const resolvedId = await resolvePlayerId(sql, id);
      const pr: any[] = resolvedId ? await sql`SELECT * FROM players WHERE id=${resolvedId} LIMIT 1` : [];
      if (!pr[0]) return json({ statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found' },404);
      const p = dbPlayer(pr[0]);
      if (seg[3] === 'scores') {
        const page = Math.max(1, Number(query.get('page') || 1)); const limit = Math.min(100, Number(query.get('limit') || 50));
        const rows: any[] = await sql`
          SELECT s.*, l.*, l.id AS leaderboard_id, m.*
          FROM scores s JOIN leaderboards l ON l.id=s.leaderboard_id JOIN maps m ON m.id=l.map_id
          WHERE s.player_id=${pr[0].id} ORDER BY s.pp DESC, s.created_at DESC`;
        const best = new Map<number, any>();
        for (const r of rows) if (!best.has(Number(r.leaderboard_id))) best.set(Number(r.leaderboard_id), r);
        const data = [...best.values()].slice((page-1)*limit, (page-1)*limit+limit).map((r, i) => dbScore(r, pr[0], r, r, i+1, true));
        return json({ data, metadata: metadata(best.size,page,limit) });
      }
      if (seg[3] === 'history' || seg[3] === 'global-history') return json({ data: [], metadata: metadata(0,1,50) });
      if (seg[3] === 'basic') return json({ id:p.id,name:p.name,country:p.country,avatar:p.avatar,stats:p.stats });
      return json(p);
    }
    const p = players.find((x) => x.id === id);
    if (!p) return json({ statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found' },404);
    if (seg[3] === 'scores') {
      const limit = Math.min(100, Number(query.get('limit') || 8)); const ps = scores.filter((s) => s.player.id === p.id);
      return json({ data: ps.slice(0,limit), metadata: metadata(ps.length,1,limit) });
    }
    if (seg[3] === 'history' || seg[3] === 'global-history') return json({ data: [], metadata: metadata(0,1,50) });
    return json(p);
  }

  // ------------------------- MAPS -------------------------
  if (route === '/maps/sync' && (method === 'POST' || method === 'GET')) {
    if (!sql) return json({ synced: 0, source: 'fallback' });
    const cronSecret = process.env.CRON_SECRET || '';
    const authHeader = request.headers.get('authorization') || '';
    const cronAuthorized = cronSecret && authHeader === `Bearer ${cronSecret}`;
    const viewerId = await authPlayerId(request, sql);
    const adminAuthorized = viewerId ? await isAdmin(sql, viewerId) : false;
    if (!cronAuthorized && !adminAuthorized) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Map sync requires admin access'},401);
    try {
      const result = await syncBeatSaverMaps(sql, { maxPages: Number(query.get('pages') || 50), forceBootstrap: query.get('bootstrap') === '1' });
      return json(result);
    } catch (error) {
      console.error('[SnoreSaber] BeatSaver sync failed', error);
      return json({statusCode:502,error:'Bad Gateway',code:'BEATSAVER_SYNC_FAILED',message:error instanceof Error ? error.message : 'BeatSaver sync failed'},502);
    }
  }
  if (route === '/maps' && method === 'GET') {
    const page = Math.max(1, Number(query.get('page') || 1)); const limit = Math.min(100, Math.max(1, Number(query.get('limit') || 50)));
    const search = (query.get('search') || '').toLowerCase(); const verified = query.get('verified');
    const requestedStatuses = [...new Set(query.getAll('status').flatMap((value) => value.split(',')).map((x) => x.trim().toUpperCase()).filter(Boolean))];
    const minStars = Number(query.get('minStars') || 0); const maxStars = Number(query.get('maxStars') || 99);
    if (sql) {
      // Maps are served entirely from Neon. BeatSaver ingestion happens through
      // /maps/sync (Vercel cron/admin), so browsing maps never waits on BeatSaver
      // and never re-imports the entire catalog.
      const rows: any[] = await sql`SELECT * FROM maps ORDER BY created_at DESC`;
      const lbs: any[] = await sql`SELECT l.*, COUNT(s.id)::int AS total_scores FROM leaderboards l LEFT JOIN scores s ON s.leaderboard_id=l.id GROUP BY l.id ORDER BY l.id`;
      const dataRows = rows.map((r) => ({ r, lbs: lbs.filter((l) => Number(l.map_id) === Number(r.id)) }));
      let filtered = dataRows.filter(({r,lbs}) => {
        const curated = PUBLIC_BEATSAVER_MAP_KEYS.has(String(r.bsid || ''));
        const ranked = lbs.some((l) => String(l.status || '').toUpperCase() === 'RANKED');
        const ai = Boolean(r.is_ai);
        const q = !search || r.song_name.toLowerCase().includes(search) || r.level_author_name.toLowerCase().includes(search) || r.hash.toLowerCase().includes(search) || String(r.bsid || '').toLowerCase().includes(search);
        const stars = lbs.length ? Math.max(...lbs.map((l) => Number(l.stars || 0))) : 0;
        return curated && !ai && q && stars >= minStars && stars <= maxStars;
      });
      const sortBy = query.get('sortBy') || 'trending';
      const sortDirection = query.get('sortDirection') === 'asc' ? 1 : -1;
      if (sortBy === 'highestStars') filtered.sort((a,b) => (Math.max(0,...b.lbs.map((l:any)=>Number(l.stars||0))) - Math.max(0,...a.lbs.map((l:any)=>Number(l.stars||0)))) * sortDirection);
      else if (sortBy === 'latestRankedAt') filtered.sort((a,b) => {
        const ar = Math.max(0,...a.lbs.filter((l:any)=>l.status==='RANKED').map((l:any)=>new Date(l.ranked_at || 0).getTime()));
        const br = Math.max(0,...b.lbs.filter((l:any)=>l.status==='RANKED').map((l:any)=>new Date(l.ranked_at || 0).getTime()));
        return (br-ar)*sortDirection;
      });
      else if (sortBy === 'latest') filtered.sort((a,b) => (new Date(b.r.created_at).getTime()-new Date(a.r.created_at).getTime())*sortDirection);
      const slice = filtered.slice((page-1)*limit,(page-1)*limit+limit).map(({r,lbs}) => dbMap(r,lbs.map((x) => dbLeaderboard(x,Number(x.total_scores||0)))));
      return json({ data:slice, metadata:metadata(filtered.length,page,limit) });
    }
    let filtered = maps.filter((m) => {
      const curated = PUBLIC_BEATSAVER_MAP_KEYS.has(String(m.bsid || ''));
      const ranked = (m.leaderboards || []).some((l:any) => String(l.realm?.leaderboardStatus || '').toUpperCase() === 'RANKED');
      const q = !search || m.songName.toLowerCase().includes(search) || m.levelAuthorName.toLowerCase().includes(search) || m.hash.toLowerCase().includes(search) || String(m.bsid || '').toLowerCase().includes(search);
      const stars = Math.max(0,...(m.leaderboards || []).map((l:any)=>Number(l.realm?.stars||0)));
      return curated && ranked && q && stars >= minStars && stars <= maxStars;
    });
    return json({ data:filtered.slice((page-1)*limit,(page-1)*limit+limit), metadata:metadata(filtered.length,page,limit) });
  }
  if (route.startsWith('/maps/hash/') && method === 'GET') {
    const hash = route.split('/')[2];
    if (sql) { const rows:any[] = await sql`SELECT * FROM maps WHERE lower(hash)=lower(${hash}) LIMIT 1`; return rows[0] ? json(await dbMapWithLeaderboards(sql, Number(rows[0].id))) : json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404); }
    const m = maps.find((x) => x.hash.toLowerCase() === hash.toLowerCase()); return m ? json(m) : json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404);
  }
  if (route.startsWith('/maps/') && method === 'GET') {
    const identifier = decodeURIComponent(route.split('/')[2] || '').trim();
    const numericId = Number(identifier);
    if (sql) {
      let rows: any[] = [];
      if (Number.isInteger(numericId) && numericId > 0) rows = await sql`SELECT * FROM maps WHERE id=${numericId} LIMIT 1`;
      if (!rows[0] && identifier) rows = await sql`SELECT * FROM maps WHERE lower(bsid)=lower(${identifier}) OR lower(hash)=lower(${identifier}) LIMIT 1`;
      if (!rows[0]) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404);
      const id = Number(rows[0].id);
      try {
        const m = await dbMapWithLeaderboards(sql,id);
        return m ? json(m) : json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404);
      } catch (error) {
        console.error('[SnoreSaber] Map detail failed', { identifier, error });
        const lbs:any[] = await sql`SELECT l.*, COUNT(s.id)::int AS total_scores FROM leaderboards l LEFT JOIN scores s ON s.leaderboard_id=l.id WHERE l.map_id=${id} GROUP BY l.id ORDER BY l.difficulty ASC,l.id ASC`;
        return json(dbMap(rows[0], lbs.map((x:any)=>dbLeaderboard(x,Number(x.total_scores||0)))));
      }
    }
    const m = maps.find((x) => x.id === numericId || String(x.bsid || '').toLowerCase() === identifier.toLowerCase() || String(x.hash || '').toLowerCase() === identifier.toLowerCase());
    return m ? json({...m,reuploadVersions:[]}) : json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404);
  }

  // ------------------------- LEADERBOARDS -------------------------
  if (route === '/leaderboards' && method === 'GET') {
    if (sql) {
      const rows:any[] = await sql`
        SELECT l.*, m.*, COUNT(s.id)::int AS total_scores
        FROM leaderboards l JOIN maps m ON m.id=l.map_id LEFT JOIN scores s ON s.leaderboard_id=l.id
        GROUP BY l.id,m.id ORDER BY l.id`;
      const data = rows.map((r) => ({ id:Number(r.id), map:dbMap(r), difficulty:{id:Number(r.id),difficulty:Number(r.difficulty),rawDifficulty:r.raw_difficulty,gameMode:r.game_mode}, maxScore:Number(r.max_score),totalScores:Number(r.total_scores||0),dailyScores:0,createdAt:new Date(r.created_at).toISOString(),realm:realm(Number(r.stars||0),r.status) }));
      return json({data,metadata:metadata(data.length,1,data.length||1)});
    }
    const data = maps.flatMap((m) => m.leaderboards.map((l:any) => ({ id:l.id,map:m,difficulty:{id:l.id,difficulty:l.difficulty,rawDifficulty:l.rawDifficulty,gameMode:l.gameMode},maxScore:l.maxScore,totalScores:l.totalScores,dailyScores:l.dailyScores,createdAt:l.createdAt,realm:l.realm })));
    return json({data,metadata:metadata(data.length,1,data.length||1)});
  }
  if (route.startsWith('/leaderboards/hash/') && method === 'GET') {
    const hash = route.split('/')[2];
    if (sql) { const rows:any[] = await sql`SELECT l.* FROM leaderboards l JOIN maps m ON m.id=l.map_id WHERE lower(m.hash)=lower(${hash}) ORDER BY l.difficulty LIMIT 1`; return rows[0] ? json(dbLeaderboard(rows[0])) : json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Leaderboard not found'},404); }
    const m = maps.find((x)=>x.hash.toLowerCase()===hash.toLowerCase()); return m ? json(m.leaderboards[0]) : json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Leaderboard not found'},404);
  }
  if (route.startsWith('/leaderboards/')) {
    const seg = route.split('/'); const id = Number(seg[2]);
    if (sql) {
      const rows:any[] = await sql`SELECT l.*, m.* FROM leaderboards l JOIN maps m ON m.id=l.map_id WHERE l.id=${id} LIMIT 1`;
      if (!rows[0]) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Leaderboard not found'},404);
      const r=rows[0];
      if (seg[3] === 'scores') {
        const page=Math.max(1,Number(query.get('page')||1)); const limit=Math.min(100,Number(query.get('limit')||50));
        const rows2:any[]=await sql`SELECT s.*, p.id AS p_id,p.name,p.country,p.avatar, l.id AS lb_id,l.difficulty,l.game_mode,l.raw_difficulty,l.max_score,l.stars,l.status,l.created_at AS lb_created_at, m.id AS map_id,m.hash AS map_hash,m.bsid AS map_bsid,m.song_name,m.song_sub_name,m.song_author_name,m.level_author_name,m.bpm,m.cover_url,m.verified,m.created_at AS map_created_at FROM scores s JOIN players p ON p.id=s.player_id JOIN leaderboards l ON l.id=s.leaderboard_id JOIN maps m ON m.id=l.map_id WHERE s.leaderboard_id=${id} ORDER BY s.score DESC, s.accuracy DESC, s.created_at ASC`;
        const best=new Map<string,any>(); for(const x of rows2) if(!best.has(x.player_id)) best.set(x.player_id,x);
        const ranked=[...best.values()]; const data=ranked.slice((page-1)*limit,(page-1)*limit+limit).map((x,i)=>dbScore(x,{id:x.p_id,name:x.name,country:x.country,avatar:x.avatar},{id:x.lb_id,difficulty:x.difficulty,game_mode:x.game_mode,raw_difficulty:x.raw_difficulty,max_score:x.max_score,stars:x.stars,status:x.status,created_at:x.lb_created_at}, {id:x.map_id,hash:x.map_hash,bsid:x.map_bsid,song_name:x.song_name,song_sub_name:x.song_sub_name,song_author_name:x.song_author_name,level_author_name:x.level_author_name,bpm:x.bpm,cover_url:x.cover_url,verified:x.verified,created_at:x.map_created_at},i+1,true));
        return json({data,metadata:metadata(ranked.length,page,limit)});
      }
      const count:any[]=await sql`SELECT COUNT(*)::int AS count FROM scores WHERE leaderboard_id=${id}`;
      return json({id:Number(r.id),map:dbMap(r),difficulty:{id:Number(r.id),difficulty:Number(r.difficulty),rawDifficulty:r.raw_difficulty,gameMode:r.game_mode},maxScore:Number(r.max_score),totalScores:Number(count[0]?.count||0),dailyScores:0,createdAt:new Date(r.created_at).toISOString(),realm:realm(Number(r.stars||0),r.status)});
    }
    const m=maps.find((x)=>x.leaderboards.some((l:any)=>l.id===id)); if(!m)return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Leaderboard not found'},404);
    const lb=m.leaderboards.find((l:any)=>l.id===id)!;
    if(seg[3]==='scores')return json({data:scores.map((s,i)=>({...s,rank:i+1})),metadata:metadata(scores.length,1,50)});
    return json({id:lb.id,map:m,difficulty:{id:lb.id,difficulty:lb.difficulty,rawDifficulty:lb.rawDifficulty,gameMode:lb.gameMode},maxScore:lb.maxScore,totalScores:lb.totalScores,dailyScores:lb.dailyScores,createdAt:lb.createdAt,realm:lb.realm});
  }

  // ------------------------- RANK REQUESTS -------------------------
  if (route === '/ranking/requests' && method === 'GET') {
    if (!sql) return json({data:[],metadata:metadata(0,1,24)});
    await ensureModerationTables(sql);
    const page=Math.max(1,Number(query.get('page')||1));
    const limit=Math.min(100,Math.max(1,Number(query.get('limit')||24)));
    const viewerId=await authPlayerId(request,sql);
    const rows:any[]=await sql`SELECT id FROM rank_requests WHERE approval_status NOT IN ('DENIED','REPLACED') ORDER BY weight DESC,created_at ASC,id ASC`;
    const data=[];
    for(const row of rows.slice((page-1)*limit,(page-1)*limit+limit)){ const item=await getRankRequestSummary(sql,Number(row.id),viewerId); if(item) data.push(item); }
    return json({data,metadata:metadata(rows.length,page,limit)});
  }

  if (route.startsWith('/ranking/requests/') && method === 'GET') {
    if (!sql) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Rank request not found'},404);
    await ensureModerationTables(sql);
    const id=Number(route.split('/')[3]);
    const viewerId=await authPlayerId(request,sql);
    const item=await getRankRequestDetails(sql,id,viewerId);
    return item ? json(item) : json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Rank request not found'},404);
  }

  if (route === '/ranking/requests' && method === 'POST') {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({statusCode:500,error:'Internal Server Error',code:'INTERNAL_SERVER_ERROR',message:'Database unavailable'},500);
    await ensureModerationTables(sql);
    let body:any={}; try{body=JSON.parse(await request.text()||'{}')}catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    const mapId=Number(body.mapId); const description=String(body.description||'').trim();
    const leaderboardIds=Array.isArray(body.leaderboardIds)?body.leaderboardIds.map(Number).filter((x:number)=>Number.isInteger(x)&&x>0):[];
    if(!Number.isInteger(mapId)||mapId<=0||!description||leaderboardIds.length===0||leaderboardIds.length>32) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'mapId, description and leaderboardIds are required'},400);
    const mapRows:any[]=await sql`SELECT * FROM maps WHERE id=${mapId} LIMIT 1`; if(!mapRows[0]) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404);
    const lbRows:any[]=await sql`SELECT id FROM leaderboards WHERE map_id=${mapId} AND id=ANY(${leaderboardIds})`;
    if(lbRows.length!==new Set(leaderboardIds).size) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'One or more leaderboards do not belong to this map'},400);
    const existing:any[]=await sql`SELECT id FROM rank_requests WHERE map_id=${mapId} AND approval_status NOT IN ('DENIED','REPLACED') LIMIT 1`;
    if(existing[0]) return json({statusCode:409,error:'Conflict',code:'ALREADY_EXISTS',message:'This map already has an active rank request'},409);
    const inserted:any[]=await sql`INSERT INTO rank_requests(map_id,description,request_type,approval_status,weight,created_by) VALUES(${mapId},${description},'RANK','PENDING',1,${pid}) RETURNING id`;
    const requestId=Number(inserted[0].id);
    for(const lbId of [...new Set(leaderboardIds)]) await sql`INSERT INTO rank_request_difficulties(request_id,leaderboard_id,description,approval_status) VALUES(${requestId},${lbId},'', 'PENDING')`;
    return json(await getRankRequestDetails(sql,requestId,pid));
  }

  if (route.startsWith('/ranking/requests/') && route.endsWith('/rt/vote') && method === 'POST') {
    const pid=await authPlayerId(request,sql); if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true}); await ensureModerationTables(sql);
    const difficultyId=Number(route.split('/')[3]); let body:any={}; try{body=JSON.parse(await request.text()||'{}')}catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    const vote=body.vote==='DOWNVOTE'?'DOWNVOTE':body.vote==='UPVOTE'?'UPVOTE':null; if(!vote)return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid vote'},400);
    const d:any[]=await sql`SELECT id FROM rank_request_difficulties WHERE id=${difficultyId} LIMIT 1`; if(!d[0])return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Rank request difficulty not found'},404);
    const old:any[]=await sql`SELECT vote FROM rank_request_votes WHERE difficulty_id=${difficultyId} AND player_id=${pid} AND group_name='RT' LIMIT 1`;
    if(old[0]?.vote===vote) await sql`DELETE FROM rank_request_votes WHERE difficulty_id=${difficultyId} AND player_id=${pid} AND group_name='RT'`;
    else await sql`INSERT INTO rank_request_votes(difficulty_id,player_id,group_name,vote) VALUES(${difficultyId},${pid},'RT',${vote}) ON CONFLICT(difficulty_id,player_id,group_name) DO UPDATE SET vote=EXCLUDED.vote,created_at=now()`;
    return json({success:true});
  }

  if (route.startsWith('/ranking/requests/') && route.endsWith('/qat/vote') && method === 'POST') {
    const pid=await authPlayerId(request,sql); if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true}); await ensureModerationTables(sql);
    const difficultyId=Number(route.split('/')[3]); let body:any={}; try{body=JSON.parse(await request.text()||'{}')}catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    const vote=['UPVOTE','DOWNVOTE','NEUTRAL'].includes(body.vote)?body.vote:null; if(!vote)return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid vote'},400);
    const d:any[]=await sql`SELECT id FROM rank_request_difficulties WHERE id=${difficultyId} LIMIT 1`; if(!d[0])return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Rank request difficulty not found'},404);
    const old:any[]=await sql`SELECT vote FROM rank_request_votes WHERE difficulty_id=${difficultyId} AND player_id=${pid} AND group_name='QAT' LIMIT 1`;
    if(old[0]?.vote===vote) await sql`DELETE FROM rank_request_votes WHERE difficulty_id=${difficultyId} AND player_id=${pid} AND group_name='QAT'`;
    else await sql`INSERT INTO rank_request_votes(difficulty_id,player_id,group_name,vote) VALUES(${difficultyId},${pid},'QAT',${vote}) ON CONFLICT(difficulty_id,player_id,group_name) DO UPDATE SET vote=EXCLUDED.vote,created_at=now()`;
    return json({success:true});
  }

  if (route.startsWith('/ranking/requests/') && (route.endsWith('/rt/comment') || route.endsWith('/qat/comment')) && method === 'POST') {
    const pid=await authPlayerId(request,sql); if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true}); await ensureModerationTables(sql);
    const difficultyId=Number(route.split('/')[3]); const group=route.endsWith('/rt/comment')?'RT':'QAT'; let body:any={}; try{body=JSON.parse(await request.text()||'{}')}catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    const comment=String(body.comment||'').trim().slice(0,4096); if(!comment)return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Comment is required'},400);
    const d:any[]=await sql`SELECT id FROM rank_request_difficulties WHERE id=${difficultyId} LIMIT 1`; if(!d[0])return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Rank request difficulty not found'},404);
    await sql`INSERT INTO rank_request_comments(difficulty_id,player_id,group_name,comment) VALUES(${difficultyId},${pid},${group},${comment})`; return json({success:true});
  }


  // ------------------------- ADMIN CURATED MAP RANKING -------------------------
  if (route === '/admin/maps/rank-from-beatsaver' && method === 'POST' && sql) {
    const viewerId = await authPlayerId(request, sql);
    if (!(await isAdmin(sql, viewerId))) {
      return json({ statusCode: 401, error: 'Unauthorized', code: 'UNAUTHORIZED', message: 'Administrator permission required' }, 401);
    }

    let body: any = {};
    try { body = JSON.parse(await request.text() || '{}'); }
    catch { return json({ statusCode: 400, error: 'Bad Request', code: 'VALIDATION_ERROR', message: 'Invalid JSON' }, 400); }

    const link = String(body.beatSaverLink || '').trim();
    const keyMatch = link.match(/(?:beatsaver\.com\/maps\/|\/maps\/)([A-Za-z0-9]+)/i);
    const key = String(body.key || keyMatch?.[1] || '').trim();

    if (!key) {
      return json({ statusCode: 400, error: 'Bad Request', code: 'VALIDATION_ERROR', message: 'Enter a BeatSaver map link or map key' }, 400);
    }
    if (!PUBLIC_BEATSAVER_MAP_KEYS.has(key)) {
      return json({
        statusCode: 400,
        error: 'Bad Request',
        code: 'MAP_NOT_CURATED',
        message: "That BeatSaver map is not in SnoreSaber's six-map catalog"
      }, 400);
    }

    const preview = body.preview === true;
    const rankings = Array.isArray(body.rankings) ? body.rankings : [];
    for (const item of rankings) {
      const stars = Number(item.stars);
      if (!Number.isFinite(stars) || stars < 0 || stars > 100) {
        return json({ statusCode: 400, error: 'Bad Request', code: 'VALIDATION_ERROR', message: 'Stars must be between 0 and 100' }, 400);
      }
    }

    const response = await fetch(`${BEATSAVER_API}/maps/id/${encodeURIComponent(key)}`, {
      headers: { accept: 'application/json', 'user-agent': 'SnoreSaber/3.0 admin map ranking' },
      cache: 'no-store'
    });
    if (!response.ok) {
      return json({ statusCode: 502, error: 'Bad Gateway', code: 'BEATSAVER_FAILED', message: `BeatSaver returned HTTP ${response.status}` }, 502);
    }

    const map = await response.json();
    const version = Array.isArray(map?.versions)
      ? (map.versions.find((v: any) => String(v.state || '').toLowerCase() === 'published') || map.versions[0])
      : null;
    const hash = String(version?.hash || '').trim();
    if (!hash) {
      return json({ statusCode: 502, error: 'Bad Gateway', code: 'BEATSAVER_INVALID', message: 'BeatSaver did not return a published map version' }, 502);
    }

    const metadata = map.metadata || {};
    const coverUrl = String(version?.coverURL || map.coverURL || `https://eu.cdn.beatsaver.com/${hash}.jpg`).trim();
    const inserted: any[] = await sql`
      INSERT INTO maps (hash, bsid, song_name, song_sub_name, song_author_name, level_author_name, bpm, cover_url, verified, is_ai, created_at)
      VALUES (
        ${hash}, ${key}, ${String(metadata.songName || map.name || 'Unknown')}, ${String(metadata.songSubName || '')},
        ${String(metadata.songAuthorName || '')}, ${String(metadata.levelAuthorName || map.uploader?.name || '')},
        ${Number(metadata.bpm || 0)}, ${coverUrl}, ${Boolean(map.verified || map.uploader?.verifiedMapper)},
        false, COALESCE(${map.uploaded ? new Date(map.uploaded).toISOString() : null}::timestamptz, now())
      )
      ON CONFLICT (hash) DO UPDATE SET
        bsid=EXCLUDED.bsid, song_name=EXCLUDED.song_name, song_sub_name=EXCLUDED.song_sub_name,
        song_author_name=EXCLUDED.song_author_name, level_author_name=EXCLUDED.level_author_name,
        bpm=EXCLUDED.bpm, cover_url=EXCLUDED.cover_url, verified=EXCLUDED.verified, is_ai=false
      RETURNING id`;
    const mapId = Number(inserted[0]?.id);
    if (!mapId) return json({ statusCode: 500, error: 'Internal Server Error', code: 'MAP_SAVE_FAILED', message: 'Could not save the map' }, 500);

    // A preview only loads the BeatSaver difficulties. A save is authoritative:
    // submitted star values are the complete SnoreSaber ranking for this map.
    if (!preview) {
      await sql`UPDATE leaderboards SET status='UNRANKED', stars=0, ranked_at=NULL WHERE map_id=${mapId}`;
    }

    const diffs = Array.isArray(version?.diffs) ? version.diffs : [];
    const saved: any[] = [];

    for (const diff of diffs) {
      const difficulty = beatSaverDifficultyValue(diff.difficulty);
      if (!difficulty) continue;
      const gameMode = beatSaverGameMode(diff.characteristic);
      const rawDifficulty = String(diff.difficulty || 'ExpertPlus');
      const maxScore = Number(diff.maxScore || 1000000);

      const row: any[] = await sql`
        INSERT INTO leaderboards (map_id, difficulty, game_mode, raw_difficulty, max_score, stars, status, ranked_at)
        VALUES (${mapId}, ${difficulty}, ${gameMode}, ${rawDifficulty}, ${maxScore}, 0, 'UNRANKED', NULL)
        ON CONFLICT (map_id, difficulty, game_mode) DO UPDATE SET
          raw_difficulty=EXCLUDED.raw_difficulty, max_score=EXCLUDED.max_score
        RETURNING id`;

      const leaderboardId = Number(row[0]?.id);
      if (!leaderboardId) continue;

      const submitted = rankings.find((r: any) =>
        Number(r.leaderboardId) === leaderboardId ||
        (Number(r.difficulty) === difficulty && String(r.gameMode || 'Standard') === gameMode)
      );
      const stars = Number(submitted?.stars || 0);

      if (!preview && stars > 0) {
        await sql`
          UPDATE leaderboards
          SET status='RANKED', stars=${Number(stars.toFixed(3))}, ranked_at=COALESCE(ranked_at, now())
          WHERE id=${leaderboardId}`;
        await recalculateLeaderboardPlayers(sql, leaderboardId);
      }

      const current: any[] = await sql`
        SELECT stars, status
        FROM leaderboards
        WHERE id=${leaderboardId}
        LIMIT 1
      `;

      saved.push({
        id: leaderboardId,
        difficulty,
        gameMode,
        rawDifficulty,
        stars: Number(current[0]?.stars || 0),
        status: String(current[0]?.status || 'UNRANKED')
      });
    }

    return json({
      success: true,
      map: { id: mapId, bsid: key, songName: String(metadata.songName || map.name || 'Unknown') },
      leaderboards: saved
    });
  }

  // ------------------------- ADMIN LEADERBOARD ACTIONS -------------------------
  if (route.startsWith('/admin/leaderboards/') && method === 'POST' && sql) {
    await ensureLeaderboardAdminColumns(sql);
    const seg = route.split('/').filter(Boolean);
    const leaderboardId = Number(seg[2]);
    const action = seg[3];
    if (!Number.isInteger(leaderboardId) || leaderboardId <= 0) {
      return json({ statusCode: 400, error: 'Bad Request', code: 'INVALID_PATH_PARAMETER', message: 'Leaderboard id must be a positive integer' }, 400);
    }

    const viewerId = await authPlayerId(request, sql);
    if (!(await isAdmin(sql, viewerId))) {
      return json({ statusCode: 401, error: 'Unauthorized', code: 'UNAUTHORIZED', message: 'Administrator permission required' }, 401);
    }

    const found: any[] = await sql`SELECT * FROM leaderboards WHERE id=${leaderboardId} LIMIT 1`;
    if (!found[0]) {
      return json({ statusCode: 404, error: 'Not Found', code: 'NOT_FOUND', message: 'Leaderboard not found', details: { resource: 'leaderboard', id: leaderboardId } }, 404);
    }

    let body: any = {};
    try { body = JSON.parse(await request.text() || '{}'); } catch { body = {}; }

    if (action === 'rank') {
      const suppliedStars = Number(body.stars);
      const suppliedPP = Number(body.maxPP);
      const stars = Number.isFinite(suppliedStars) && suppliedStars > 0
        ? Number(suppliedStars.toFixed(3))
        : Number.isFinite(suppliedPP) && suppliedPP > 0
          ? Number(((suppliedPP * 10.685333512) / 450).toFixed(3))
          : 0;
      if (!Number.isFinite(stars) || stars <= 0) {
        return json({ statusCode: 400, error: 'Bad Request', code: 'VALIDATION_ERROR', message: 'Stars or maxPP must be greater than 0' }, 400);
      }
      await sql`UPDATE leaderboards SET status='RANKED', stars=${stars}, ranked_at=COALESCE(ranked_at, now()) WHERE id=${leaderboardId}`;
      const affected = await recalculateLeaderboardPlayers(sql, leaderboardId);
      return json({ success: true, affectedPlayers: affected });
    }

    if (action === 'unrank') {
      await sql`UPDATE leaderboards SET status='UNRANKED', ranked_at=NULL WHERE id=${leaderboardId}`;
      const affected = await recalculateLeaderboardPlayers(sql, leaderboardId);
      return json({ success: true, affectedPlayers: affected });
    }

    if (action === 'qualify') {
      await sql`UPDATE leaderboards SET status='QUALIFIED', ranked_at=NULL WHERE id=${leaderboardId}`;
      const affected = await recalculateLeaderboardPlayers(sql, leaderboardId);
      return json({ success: true, affectedPlayers: affected });
    }

    if (action === 'love') {
      await sql`UPDATE leaderboards SET status='LOVED', ranked_at=NULL WHERE id=${leaderboardId}`;
      const affected = await recalculateLeaderboardPlayers(sql, leaderboardId);
      return json({ success: true, affectedPlayers: affected });
    }

    if (action === 'pp-manual') {
      const suppliedStars = Number(body.stars);
      const suppliedPP = Number(body.maxPP);
      const stars = Number.isFinite(suppliedStars) && suppliedStars >= 0
        ? Number(suppliedStars.toFixed(3))
        : Number.isFinite(suppliedPP) && suppliedPP >= 0
          ? Number(((suppliedPP * 10.685333512) / 450).toFixed(3))
          : NaN;
      if (!Number.isFinite(stars)) {
        return json({ statusCode: 400, error: 'Bad Request', code: 'VALIDATION_ERROR', message: 'Stars or maxPP must be a non-negative number' }, 400);
      }
      await sql`UPDATE leaderboards SET stars=${stars} WHERE id=${leaderboardId}`;
      const affected = await recalculateLeaderboardPlayers(sql, leaderboardId);
      return json({ success: true, affectedPlayers: affected });
    }

    if (action === 'pp') {
      const affected = await recalculateLeaderboardPlayers(sql, leaderboardId);
      return json({ success: true, affectedPlayers: affected });
    }

    return json({ statusCode: 404, error: 'Not Found', code: 'NOT_FOUND', message: 'Unknown leaderboard admin action' }, 404);
  }

  // ------------------------- ADMIN MODERATION -------------------------
  if (route.startsWith('/admin/user/') && route.endsWith('/ban') && method === 'GET') {
    const targetId = decodeURIComponent(route.split('/')[3]);
    if (!sql) return json(null);
    const rows: any[] = await sql`SELECT banned,ban_reason,ban_notes,ban_created_at,ban_auto_unban,ban_auto_unbans_at,ban_earliest_appeal_date FROM players WHERE id=${targetId} OR steam_id=${targetId} LIMIT 1`;
    if (!rows[0]) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    if (!rows[0].banned) return json(null);
    return json({reason:rows[0].ban_reason || '',notes:rows[0].ban_notes || null,createdAt:rows[0].ban_created_at ? new Date(rows[0].ban_created_at).toISOString() : NOW(),autoUnban:Boolean(rows[0].ban_auto_unban),autoUnbansAt:rows[0].ban_auto_unbans_at ? new Date(rows[0].ban_auto_unbans_at).toISOString() : null,earliestAppealDate:rows[0].ban_earliest_appeal_date ? new Date(rows[0].ban_earliest_appeal_date).toISOString() : null});
  }

  if (route.startsWith('/admin/user/') && route.endsWith('/ban') && method === 'POST') {
    const targetId = decodeURIComponent(route.split('/')[3]);
    const viewerId = await authPlayerId(request, sql);
    if (!(await isAdmin(sql, viewerId))) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Administrator permission required'},401);
    if (!sql) return json({success:true});
    let body:any = {}; try { body = JSON.parse(await request.text() || '{}'); } catch { return json({error:'Invalid JSON'},400); }
    const reason = String(body.reason || '').trim();
    if (!reason) return json({error:'reason is required'},400);
    const autoUnban = Boolean(body.autoUnban);
    const targetRows: any[] = await sql`SELECT id FROM players WHERE id=${targetId} OR steam_id=${targetId} LIMIT 1`;
    if (!targetRows[0]) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    const target = targetRows[0].id;
    await sql`UPDATE players SET banned=true, ban_reason=${reason}, ban_notes=${body.notes ? String(body.notes) : null}, ban_created_at=now(), ban_auto_unban=${autoUnban}, ban_auto_unbans_at=${body.autoUnbansAt ? new Date(body.autoUnbansAt) : null}, ban_earliest_appeal_date=${body.earliestAppealDate ? new Date(body.earliestAppealDate) : null} WHERE id=${target}`;
    return json({success:true});
  }

  if (route.startsWith('/admin/user/') && route.endsWith('/unban') && method === 'POST') {
    const targetId = decodeURIComponent(route.split('/')[3]);
    const viewerId = await authPlayerId(request, sql);
    if (!(await isAdmin(sql, viewerId))) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Administrator permission required'},401);
    if (!sql) return json({success:true});
    const targetRows: any[] = await sql`SELECT id FROM players WHERE id=${targetId} OR steam_id=${targetId} LIMIT 1`;
    if (!targetRows[0]) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    await sql`UPDATE players SET banned=false, ban_reason=null, ban_notes=null, ban_created_at=null, ban_auto_unban=false, ban_auto_unbans_at=null, ban_earliest_appeal_date=null WHERE id=${targetRows[0].id}`;
    return json({success:true});
  }


  // ------------------------- ADMIN SOCIAL / PROFILE ACTIONS -------------------------
  if (route === '/admin/permissions' && method === 'GET') {
    const viewerId = await authPlayerId(request, sql);
    if (!(await isAdmin(sql, viewerId))) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Administrator permission required'},401);
    return json(Object.entries(PERMISSION_VALUES).map(([name,value]) => ({name,value})));
  }

  if (route === '/admin/badges' && method === 'GET') {
    const viewerId = await authPlayerId(request, sql);
    if (!(await isAdmin(sql, viewerId))) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Administrator permission required'},401);
    await ensureModerationTables(sql);
    const rows: any[] = await sql`
      SELECT b.id,b.image,b.description,b.image_url,COUNT(pb.player_id)::int AS assignment_count
      FROM badges b LEFT JOIN player_badges pb ON pb.badge_id=b.id
      GROUP BY b.id ORDER BY b.id ASC
    `;
    return json(rows.map((r) => ({
      id:Number(r.id),
      image:(String(r.description)==='SnoreSaber Developer' ? '/assets/badges/developer.png' : String(r.description)==='SnoreSaber Tester' ? '/assets/badges/tester.png' : String(r.description)==='SnoreSaber Staff' ? '/assets/badges/staff.png' : String(r.description)==='Verified Player' ? '/assets/badges/verified.png' : String(r.description)==='Map Contributor' ? '/assets/badges/mapper.png' : String(r.description)==='Tournament Staff' ? '/assets/badges/tournament.png' : (r.image_url || r.image)),
      description:r.description,
      imageUrl:(String(r.description)==='SnoreSaber Developer' ? '/assets/badges/developer.png' : String(r.description)==='SnoreSaber Tester' ? '/assets/badges/tester.png' : String(r.description)==='SnoreSaber Staff' ? '/assets/badges/staff.png' : String(r.description)==='Verified Player' ? '/assets/badges/verified.png' : String(r.description)==='Map Contributor' ? '/assets/badges/mapper.png' : String(r.description)==='Tournament Staff' ? '/assets/badges/tournament.png' : (r.image_url || r.image)), assignmentCount:Number(r.assignment_count || 0)
    })));
  }

  if (route.startsWith('/admin/badges/player/') && method === 'GET') {
    const playerId = decodeURIComponent(route.split('/')[4] || '');
    const viewerId = await authPlayerId(request, sql);
    if (!(await isAdmin(sql, viewerId))) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Administrator permission required'},401);
    const target = await resolvePlayerId(sql, playerId);
    if (!target) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    await ensureModerationTables(sql);
    const rows: any[] = await sql`
      SELECT badge_id,description_override,added_at FROM player_badges
      WHERE player_id=${target} ORDER BY added_at ASC,badge_id ASC
    `;
    return json(rows.map((r) => ({badgeId:Number(r.badge_id),descriptionOverride:r.description_override ?? null,addedAt:new Date(r.added_at).toISOString()})));
  }

  if (route.startsWith('/admin/badges/player/') && (method === 'PUT' || method === 'POST')) {
    const playerId = decodeURIComponent(route.split('/')[4] || '');
    const viewerId = await authPlayerId(request, sql);
    if (!(await isAdmin(sql, viewerId))) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Administrator permission required'},401);
    const target = await resolvePlayerId(sql, playerId);
    if (!target) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    let body:any={}; try { body=JSON.parse(await request.text() || '{}'); } catch { return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400); }
    if (!Array.isArray(body.badges) || body.badges.length > 1000) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'badges must be an array'},400);
    await ensureModerationTables(sql);
    const badgeIds = body.badges.map((x:any)=>Number(x.badgeId)).filter((x:number)=>Number.isInteger(x)&&x>0);
    if (badgeIds.length) {
      const available:any[] = await sql`SELECT id FROM badges WHERE id = ANY(${badgeIds})`;
      if (available.length !== badgeIds.length) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Unknown badge id'},400);
    }
    await sql`DELETE FROM player_badges WHERE player_id=${target}`;
    for (const badge of body.badges) {
      const badgeId=Number(badge.badgeId);
      const override=badge.descriptionOverride == null ? null : String(badge.descriptionOverride).trim().slice(0,256);
      await sql`INSERT INTO player_badges (player_id,badge_id,description_override) VALUES (${target},${badgeId},${override}) ON CONFLICT (player_id,badge_id) DO UPDATE SET description_override=EXCLUDED.description_override`;
    }
    return json(await getPlayerBadges(sql,target));
  }

  if (route.startsWith('/admin/user/') && route.endsWith('/role-text') && method === 'POST') {
    const targetId=decodeURIComponent(route.split('/')[3] || '');
    const viewerId=await authPlayerId(request,sql);
    if (!(await isAdmin(sql,viewerId))) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Administrator permission required'},401);
    const target=await resolvePlayerId(sql,targetId);
    if (!target) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    let body:any={}; try { body=JSON.parse(await request.text()||'{}'); } catch { return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400); }
    const roleText=String(body.roleText ?? '').trim().slice(0,128);
    await sql`UPDATE players SET role=${roleText || null} WHERE id=${target}`;
    return json({success:true});
  }

  if (route.startsWith('/admin/user/') && route.endsWith('/reset-country') && method === 'POST') {
    const targetId=decodeURIComponent(route.split('/')[3] || '');
    const viewerId=await authPlayerId(request,sql);
    if (!(await isAdmin(sql,viewerId))) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Administrator permission required'},401);
    const target=await resolvePlayerId(sql,targetId);
    if (!target) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    let body:any={}; try { body=JSON.parse(await request.text()||'{}'); } catch { return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400); }
    const country=String(body.country||'XX').trim().toUpperCase();
    if (!/^[A-Z]{2}$/.test(country)) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Country must be a two-letter code'},400);
    await sql`UPDATE players SET country=${country} WHERE id=${target}`;
    return json({success:true});
  }

  if (route.startsWith('/admin/user/') && route.endsWith('/permissions') && method === 'POST') {
    const targetId=decodeURIComponent(route.split('/')[3] || '');
    const viewerId=await authPlayerId(request,sql);
    if (!(await isAdmin(sql,viewerId))) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Administrator permission required'},401);
    const target=await resolvePlayerId(sql,targetId);
    if (!target) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    let body:any={}; try { body=JSON.parse(await request.text()||'{}'); } catch { return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400); }
    const add=Array.isArray(body.add)?body.add:[], remove=Array.isArray(body.remove)?body.remove:[];
    const rows:any[]=await sql`SELECT permissions FROM players WHERE id=${target} LIMIT 1`;
    let permissions=Number(rows[0]?.permissions||0);
    for (const name of add) if (PERMISSION_VALUES[String(name)] != null) permissions |= PERMISSION_VALUES[String(name)];
    for (const name of remove) if (PERMISSION_VALUES[String(name)] != null) permissions &= ~PERMISSION_VALUES[String(name)];
    await sql`UPDATE players SET permissions=${permissions} WHERE id=${target}`;
    return json({success:true,permissions});
  }

  if (route.startsWith('/admin/user/') && route.endsWith('/merge') && method === 'POST') {
    const targetId=decodeURIComponent(route.split('/')[3] || '');
    const viewerId=await authPlayerId(request,sql);
    if (!(await isAdmin(sql,viewerId))) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Administrator permission required'},401);
    const target=await resolvePlayerId(sql,targetId);
    if (!target) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Target player not found'},404);
    let body:any={}; try { body=JSON.parse(await request.text()||'{}'); } catch { return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400); }
    const source=await resolvePlayerId(sql,String(body.sourcePlayerId||''));
    const reason=String(body.reason||'').trim().slice(0,512);
    if (!source || !reason) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'sourcePlayerId and reason are required'},400);
    if (source===target) return json({statusCode:400,error:'Bad Request',code:'INVALID_OPERATION',message:'Source and target must differ'},400);

    // Keep the higher score for duplicate leaderboard/player pairs, then move the
    // remaining scores and social links onto the target account.
    const sourceScores:any[]=await sql`SELECT * FROM scores WHERE player_id=${source} ORDER BY id ASC`;
    for (const scoreRow of sourceScores) {
      const existing:any[]=await sql`SELECT id,score,pp FROM scores WHERE player_id=${target} AND leaderboard_id=${scoreRow.leaderboard_id} ORDER BY score DESC LIMIT 1`;
      if (!existing[0]) {
        await sql`UPDATE scores SET player_id=${target} WHERE id=${scoreRow.id}`;
      } else if (Number(scoreRow.score)>Number(existing[0].score)) {
        await sql`DELETE FROM scores WHERE id=${existing[0].id}`;
        await sql`UPDATE scores SET player_id=${target} WHERE id=${scoreRow.id}`;
      } else {
        await sql`DELETE FROM scores WHERE id=${scoreRow.id}`;
      }
    }
    try {
      await sql`CREATE TABLE IF NOT EXISTS player_follows (
        follower_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
        following_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
        created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
        PRIMARY KEY (follower_id, following_id),
        CHECK (follower_id <> following_id)
      )`;
      await sql`
        INSERT INTO player_follows (follower_id,following_id,created_at)
        SELECT ${target},following_id,created_at FROM player_follows
        WHERE follower_id=${source} AND following_id<>${target}
        ON CONFLICT (follower_id,following_id) DO NOTHING
      `;
      await sql`
        INSERT INTO player_follows (follower_id,following_id,created_at)
        SELECT follower_id,${target},created_at FROM player_follows
        WHERE following_id=${source} AND follower_id<>${target}
        ON CONFLICT (follower_id,following_id) DO NOTHING
      `;
      await sql`DELETE FROM player_follows WHERE follower_id=${source} OR following_id=${source}`;
    } catch {}
    await ensureModerationTables(sql);
    await sql`
      INSERT INTO player_badges (player_id,badge_id,description_override,added_at)
      SELECT ${target},badge_id,description_override,added_at FROM player_badges
      WHERE player_id=${source}
      ON CONFLICT (player_id,badge_id) DO NOTHING
    `;
    await sql`DELETE FROM player_badges WHERE player_id=${source}`;
    await sql`INSERT INTO account_merges (target_player_id,source_player_id,reason,merged_by) VALUES (${target},${source},${reason},${viewerId})`;
    await sql`DELETE FROM players WHERE id=${source}`;
    await recalculatePlayerStats(sql,target);
    return json({success:true,targetPlayerId:target,publicPlayerId:target,mergedPublicPlayerIds:[source]});
  }

  // ------------------------- REPORTS -------------------------
  if ((route.startsWith('/player/') || route.startsWith('/players/')) && route.endsWith('/report') && method === 'POST') {
    const targetRequested=decodeURIComponent(route.split('/')[2] || '');
    const reporter=await authPlayerId(request,sql);
    if (!reporter) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    const target=await resolvePlayerId(sql,targetRequested);
    if (!target) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    if (target===reporter) return json({statusCode:400,error:'Bad Request',code:'INVALID_OPERATION',message:'You cannot report yourself'},400);
    let body:any={}; try { body=JSON.parse(await request.text()||'{}'); } catch { return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400); }
    const allowed=['INAPPROPRIATE_PROFILE','IMPERSONATION','HARASSMENT','CHEATING','OTHER'];
    const reason=String(body.reason||'');
    const details=String(body.details||'').slice(0,1000);
    if (!allowed.includes(reason)) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid report reason'},400);
    await ensureModerationTables(sql);
    await sql`INSERT INTO profile_reports (reporter_id,target_player_id,reason,details) VALUES (${reporter},${target},${reason},${details})`;
    return json({success:true});
  }

  if (route === '/admin/reports' && method === 'GET') {
    const viewerId=await authPlayerId(request,sql);
    if (!(await isAdmin(sql,viewerId))) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Administrator permission required'},401);
    await ensureModerationTables(sql);
    const rows:any[]=await sql`
      SELECT r.*, reporter.name AS reporter_name, target.name AS target_name
      FROM profile_reports r
      JOIN players reporter ON reporter.id=r.reporter_id
      JOIN players target ON target.id=r.target_player_id
      ORDER BY r.created_at DESC LIMIT 200
    `;
    return json(rows.map((r)=>({
      id:Number(r.id), reporterId:String(r.reporter_id), reporterName:r.reporter_name,
      targetPlayerId:String(r.target_player_id), targetName:r.target_name,
      reason:r.reason, details:r.details, status:r.status, createdAt:new Date(r.created_at).toISOString()
    })));
  }

  // ------------------------- SCORE DETAIL / SUBMISSION -------------------------
  if (route.startsWith('/scores/') && method === 'GET') {
    const id=Number(route.split('/')[2]);
    if (sql) {
      const rows:any[]=await sql`SELECT s.*, p.id AS p_id,p.name,p.country,p.avatar, l.id AS lb_id,l.difficulty,l.game_mode,l.raw_difficulty,l.max_score,l.stars,l.status,l.created_at AS lb_created_at, m.id AS map_id,m.hash AS map_hash,m.bsid AS map_bsid,m.song_name,m.song_sub_name,m.song_author_name,m.level_author_name,m.bpm,m.cover_url,m.verified,m.created_at AS map_created_at FROM scores s JOIN players p ON p.id=s.player_id JOIN leaderboards l ON l.id=s.leaderboard_id JOIN maps m ON m.id=l.map_id WHERE s.id=${id} LIMIT 1`;
      if(!rows[0])return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Score not found'},404);
      const r=rows[0];
      return json(dbScore(r,{id:r.p_id,name:r.name,country:r.country,avatar:r.avatar},{id:r.lb_id,difficulty:r.difficulty,game_mode:r.game_mode,raw_difficulty:r.raw_difficulty,max_score:r.max_score,stars:r.stars,status:r.status,created_at:r.lb_created_at},{id:r.map_id,hash:r.map_hash,bsid:r.map_bsid,song_name:r.song_name,song_sub_name:r.song_sub_name,song_author_name:r.song_author_name,level_author_name:r.level_author_name,bpm:r.bpm,cover_url:r.cover_url,verified:r.verified,created_at:r.map_created_at},1,true));
    }
    const s=scores.find((x)=>x.id===id); if(!s)return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Score not found'},404); return json(s);
  }

  if (route === '/scores/submit' && method === 'POST') {
    if (!hasIngestAuth(request) && !(await authPlayerId(request, sql))) return json({error:'Authentication required'},401);
    const bodyText=await request.text(); let body:any; try{body=JSON.parse(bodyText||'{}')}catch{return json({error:'Invalid JSON'},400)}
    const requestedPlayerId=String(body.playerId||'');
    const authenticatedPlayerId=await authPlayerId(request, sql);
    const playerId=sql ? (await resolveInternalPlayerId(sql, requestedPlayerId || String(authenticatedPlayerId || '')) || '') : (requestedPlayerId || String(authenticatedPlayerId || ''));
    const mapHash=String(body.mapHash||'');
    if(!playerId||!mapHash)return json({error:'playerId and mapHash are required'},400);
    const scoreValue=Math.max(0,Math.round(Number(body.score||body.modifiedScore||0))); const accuracy=Number(body.accuracy||0); const pp=Number(body.pp||0);
    if(!Number.isFinite(scoreValue)||!Number.isFinite(accuracy)||!Number.isFinite(pp))return json({error:'score, accuracy and pp must be numeric'},400);
    if(sql){
      const playerRows:any[]=await sql`SELECT * FROM players WHERE id=${playerId} LIMIT 1`; if(!playerRows[0])return json({error:'Unknown player'},404);
      const lbRows:any[]=await sql`SELECT l.* FROM leaderboards l JOIN maps m ON m.id=l.map_id WHERE lower(m.hash)=lower(${mapHash}) ORDER BY l.difficulty DESC LIMIT 1`; if(!lbRows[0])return json({error:'Unknown map'},404);
      const lb=lbRows[0];
      const existing:any[]=await sql`SELECT id,score,pp FROM scores WHERE leaderboard_id=${lb.id} AND player_id=${playerId} ORDER BY score DESC LIMIT 1`;
      const isPB = !existing[0] || Number(existing[0].score) < scoreValue;
      const rankedPlay = String(lb.status || 'UNRANKED') === 'RANKED';
      await sql`UPDATE players SET total_plays=COALESCE(total_plays,0)+1, total_ranked_plays=COALESCE(total_ranked_plays,0)+${rankedPlay ? 1 : 0}, last_seen_at=now() WHERE id=${playerId}`;
      if(!isPB) {
        return json({accepted:false,reason:'not_a_personal_best',scoreId:Number(existing[0].id),score:Number(existing[0].score),pp:Number(existing[0].pp),playCounted:true});
      }
      const inserted:any[]=await sql`INSERT INTO scores (leaderboard_id,player_id,score,accuracy,pp,weight,mods,bad_cuts,missed_notes,max_combo,full_combo,has_replay) VALUES (${lb.id},${playerId},${scoreValue},${accuracy},${pp},1,${Array.isArray(body.mods)?body.mods.join(','):String(body.mods||'')},${Number(body.badCuts||0)},${Number(body.missedNotes||0)},${Number(body.maxCombo||0)},${Boolean(body.fullCombo)},${Boolean(body.hasReplay)}) RETURNING id`;
      await recalculatePlayerStats(sql,playerId);
      return json({accepted:true,personalBest:true,playerId,mapHash,score:scoreValue,accuracy,pp,scoreId:Number(inserted[0].id),playCounted:true});
    }
    const p=players.find((x)=>x.id===playerId); const m=maps.find((x)=>x.hash.toLowerCase()===mapHash.toLowerCase()); if(!p)return json({error:'Unknown player'},404); if(!m)return json({error:'Unknown map'},404);
    const existing=scores.find((x)=>x.player.id===playerId&&x.leaderboard.id===m.leaderboards[0].id); if(existing&&existing.modifiedScore>=scoreValue)return json({accepted:false,reason:'not_a_personal_best',scoreId:existing.id});
    if(existing){existing.modifiedScore=scoreValue;existing.unmodifiedScore=scoreValue;existing.accuracy=accuracy;existing.pp=pp;existing.createdAt=NOW();return json({accepted:true,personalBest:true,scoreId:existing.id});}
    const id=Math.max(...scores.map(s=>Number(s.id)),1000)+1; scores.push(score(id,p,m,accuracy,scoreValue,pp)); return json({accepted:true,personalBest:true,scoreId:id});
  }


  // ------------------------- ACCOUNT / PROFILE -------------------------
  if ((route === '/user/@me/name' && method === 'PUT') || (route === '/user/update-name' && method === 'POST')) {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true});
    let body:any={}; try{body=JSON.parse(await request.text()||'{}')}catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    const name=String(body.name||'').trim().slice(0,128);
    if(!name) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Name is required'},400);
    await sql`UPDATE players SET name=${name},last_seen_at=now() WHERE id=${pid}`;
    return json({success:true});
  }

  if ((route === '/user/@me/bio' && method === 'PUT') || (route === '/user/update-bio' && method === 'POST')) {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true});
    let body:any={}; try{body=JSON.parse(await request.text()||'{}')}catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    const bio=String(body.bio||'').slice(0,4096);
    await sql`UPDATE players SET bio=${bio||null},last_seen_at=now() WHERE id=${pid}`;
    return json({success:true});
  }

  if ((route === '/user/@me/vanity' && (method === 'GET' || method === 'PUT')) || (route === '/user/@me/vanity' && method === 'POST')) {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({slug:null,canChangeAt:null});
    await sql`ALTER TABLE players ADD COLUMN IF NOT EXISTS vanity_changed_at TIMESTAMPTZ`;
    const rows:any[]=await sql`SELECT vanity,vanity_changed_at FROM players WHERE id=${pid} LIMIT 1`;
    const current=rows[0];
    const changedAt=current?.vanity_changed_at ? new Date(current.vanity_changed_at) : null;
    const canChangeAt=changedAt ? new Date(changedAt.getTime()+7*24*60*60*1000) : null;
    if (method === 'GET') return json({slug:current?.vanity || null,canChangeAt:canChangeAt?.toISOString() || null});
    let body:any={}; try{body=JSON.parse(await request.text()||'{}')}catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    if (canChangeAt && canChangeAt.getTime() > Date.now()) return json({statusCode:429,error:'Too Many Requests',code:'VANITY_COOLDOWN',message:'Vanity can only be changed once every 7 days',details:{canChangeAt:canChangeAt.toISOString()}},429);
    const slug=String(body.slug||'').trim().toLowerCase();
    if(!/^[a-z0-9_-]{3,32}$/.test(slug)) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Vanity must be 3-32 characters using letters, numbers, _ or -'},400);
    const taken:any[]=await sql`SELECT id FROM players WHERE lower(vanity)=${slug} AND id<>${pid} LIMIT 1`;
    if(taken[0]) return json({statusCode:409,error:'Conflict',code:'ALREADY_EXISTS',message:'Vanity is already in use'},409);
    await sql`UPDATE players SET vanity=${slug},vanity_changed_at=now(),last_seen_at=now() WHERE id=${pid}`;
    const next=new Date(Date.now()+7*24*60*60*1000).toISOString();
    return json({slug,canChangeAt:next});
  }

  if ((route === '/user/@me/avatar' && method === 'POST') || (route === '/user/avatar' && method === 'POST')) {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true});
    const form=await request.formData(); const file=form.get('avatar');
    if(!(file instanceof File)||file.size===0) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Avatar is required'},400);
    if(file.size>2*1024*1024) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Avatar must be 2MB or smaller'},400);
    const bytes=new Uint8Array(await file.arrayBuffer());
    let binary=''; for(let i=0;i<bytes.length;i+=0x8000) binary += String.fromCharCode(...bytes.subarray(i,i+0x8000));
    const dataUrl=`data:${file.type||'image/png'};base64,${Buffer.from(binary,'binary').toString('base64')}`;
    await sql`UPDATE players SET avatar=${dataUrl},last_seen_at=now() WHERE id=${pid}`;
    return json({success:true,avatar:dataUrl,avatarVersion:Date.now()});
  }

  if (route === '/user/can-reset-country' && method === 'GET') {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({canReset:true,lastReset:null,country:'XX'});
    const rows:any[]=await sql`SELECT country FROM players WHERE id=${pid} LIMIT 1`;
    return json({canReset:true,lastReset:null,country:String(rows[0]?.country || 'XX')});
  }

  if (route === '/user/reset-country' && method === 'POST') {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    return json({success:true});
  }

  if (route === '/user/connections' && method === 'GET') {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    return json([]);
  }

  // ------------------------- PROFILE CUSTOMIZATION -------------------------
  if (route === '/user/@me/profile-customization' && method === 'PUT') {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true});
    let body:any={}; try{ body=JSON.parse(await request.text()||'{}'); }catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    const hex=(v:any)=>v==null?null:(/^#[0-9A-Fa-f]{6}$/.test(String(v))?String(v):null);
    const arr=(v:any,max:number)=>Array.isArray(v)?v.slice(0,max).map(String):null;
    const badgeOrder=Array.isArray(body.badgeOrder)?body.badgeOrder.slice(0,128).map(Number).filter((x:number)=>Number.isInteger(x)&&x>0):null;
    const badgeComments=body.badgeComments && typeof body.badgeComments==='object' ? body.badgeComments : null;
    await ensureModerationTables(sql);
    await sql`
      INSERT INTO profile_customizations (player_id,accent_color,accent_foreground_color,accent_foreground_active_color,supporter_name_color_enabled,badge_order,badge_comments,stat_order,enabled_stat_ids,chart_metric_ids,section_order,updated_at)
      VALUES (${pid},${hex(body.accentColor)},${hex(body.accentForegroundColor)},${hex(body.accentForegroundActiveColor)},${body.supporterNameColorEnabled!==false},${badgeOrder},${badgeComments?JSON.stringify(badgeComments):null},${arr(body.statOrder,16)},${arr(body.enabledStatIds,16)},${arr(body.chartMetricIds,8)},${arr(body.sectionOrder,8)},now())
      ON CONFLICT (player_id) DO UPDATE SET accent_color=EXCLUDED.accent_color,accent_foreground_color=EXCLUDED.accent_foreground_color,accent_foreground_active_color=EXCLUDED.accent_foreground_active_color,supporter_name_color_enabled=EXCLUDED.supporter_name_color_enabled,badge_order=EXCLUDED.badge_order,badge_comments=EXCLUDED.badge_comments,stat_order=EXCLUDED.stat_order,enabled_stat_ids=EXCLUDED.enabled_stat_ids,chart_metric_ids=EXCLUDED.chart_metric_ids,section_order=EXCLUDED.section_order,updated_at=now()
    `;
    return json(await getProfileCustomization(sql,pid));
  }

  if (route === '/user/@me/pinned-scores' && method === 'PUT') {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true});
    let body:any={}; try{ body=JSON.parse(await request.text()||'{}'); }catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    if(!Array.isArray(body.pinnedScores)||body.pinnedScores.length>6) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Up to 6 pinned scores are allowed'},400);
    await ensureModerationTables(sql);
    const ids=body.pinnedScores.map((x:any)=>Number(x.scoreId)).filter((x:number)=>Number.isInteger(x)&&x>0);
    if(ids.length){
      const valid:any[]=await sql`SELECT id FROM scores WHERE player_id=${pid} AND id = ANY(${ids})`;
      if(valid.length!==ids.length) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'One or more scores do not belong to this player'},400);
    }
    await sql`DELETE FROM pinned_scores WHERE player_id=${pid}`;
    let position=0;
    for(const item of body.pinnedScores){
      const scoreId=Number(item.scoreId); if(!Number.isInteger(scoreId)||scoreId<=0) continue;
      await sql`INSERT INTO pinned_scores(player_id,score_id,position,comment) VALUES(${pid},${scoreId},${position++},${String(item.comment||'').slice(0,512)})`;
    }
    return json({success:true});
  }

  if (route === '/user/@me/profile-customization/background' && method === 'POST') {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true});
    const form=await request.formData(); const file=form.get('backgroundImage');
    if(!(file instanceof File)||file.size===0) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Background image is required'},400);
    if(file.size>2*1024*1024) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Background image must be 2MB or smaller'},400);
    const bytes=new Uint8Array(await file.arrayBuffer());
    let binary=''; for(let i=0;i<bytes.length;i+=0x8000) binary += String.fromCharCode(...bytes.subarray(i,i+0x8000));
    const mime=file.type||'image/jpeg'; const dataUrl=`data:${mime};base64,${Buffer.from(binary,'binary').toString('base64')}`;
    await ensureModerationTables(sql);
    await sql`INSERT INTO profile_customizations(player_id,background_image,background_image_version,updated_at) VALUES(${pid},${dataUrl},1,now()) ON CONFLICT(player_id) DO UPDATE SET background_image=EXCLUDED.background_image,background_image_version=profile_customizations.background_image_version+1,updated_at=now()`;
    return json(await getProfileCustomization(sql,pid));
  }

  if (route === '/user/@me/profile-customization/background' && method === 'DELETE') {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true});
    await ensureModerationTables(sql);
    await sql`UPDATE profile_customizations SET background_image=null,background_image_version=background_image_version+1,updated_at=now() WHERE player_id=${pid}`;
    return json(await getProfileCustomization(sql,pid));
  }

  // ------------------------- AUTH -------------------------
  if (route === '/user/@me' && method === 'GET') {
    const pid=await authPlayerId(request, sql); if(!pid)return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(sql){const rows:any[]=await sql`SELECT * FROM players WHERE id=${pid} LIMIT 1`; if(!rows[0]) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401); const p=dbPlayer(rows[0]); const profileCustomization=await getProfileCustomization(sql,pid); const pinnedScores=await getPinnedScores(sql,pid); const badges=await getPlayerBadges(sql,pid); const relationships=await userRelationships(sql,pid); return json({...p,profileCustomization,pinnedScores,badges,relationships});}
    const p=players.find((x)=>x.id===pid); return p?json(p):json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
  }
  if (route === '/auth/steam' && method === 'GET') {
    const state = randomBytes(24).toString('hex');
    const callback = new URL('/api/v2/auth/steam/callback', origin);
    callback.searchParams.set('redirectTo', query.get('redirectTo') || '/');
    callback.searchParams.set('state', state);
    const steam = new URL('https://steamcommunity.com/openid/login');
    steam.searchParams.set('openid.ns','http://specs.openid.net/auth/2.0'); steam.searchParams.set('openid.mode','checkid_setup'); steam.searchParams.set('openid.return_to',callback.toString());
    steam.searchParams.set('openid.realm',origin); steam.searchParams.set('openid.identity','http://specs.openid.net/auth/2.0/identifier_select'); steam.searchParams.set('openid.claimed_id','http://specs.openid.net/auth/2.0/identifier_select');
    return json({redirectUrl:steam.toString()},200,{'set-cookie':`steam-auth-state=${state}; Path=/; HttpOnly; SameSite=Lax; Secure; Max-Age=600`});
  }
  if (route === '/auth/steam/callback' && method === 'GET') {
    const state = query.get('state');
    const stateCookie = (request.headers.get('cookie') || '').split(';').map((x) => x.trim()).find((x) => x.startsWith('steam-auth-state='));
    const storedState = stateCookie ? decodeURIComponent(stateCookie.slice('steam-auth-state='.length)) : null;
    if (!state || !storedState || state !== storedState) return new Response('Steam authentication state expired or invalid',{status:400,headers:{'content-type':'text/plain'}});
    const steamId=await verifySteam(request); if(!steamId)return new Response('Steam authentication failed',{status:401,headers:{'content-type':'text/plain'}});
    let p:any;
    let internalPlayerId = steamId;
    const steamProfile = await fetchSteamProfile(steamId);
    const steamName = steamProfile?.name || `SteamUser_${steamId.slice(-5)}`;
    const steamAvatar = steamProfile?.avatar || '';
    if(sql){
      const rows:any[]=await sql`SELECT * FROM players WHERE steam_id=${steamId} OR id=${steamId} LIMIT 1`;
      if(rows[0]) {
        await sql`UPDATE players SET name=${steamName}, avatar=${steamAvatar}, last_seen_at=now() WHERE id=${rows[0].id}`;
        const updated:any[]=await sql`SELECT * FROM players WHERE id=${rows[0].id} LIMIT 1`;
        internalPlayerId = updated[0].id;
        p=dbPlayer(updated[0]);
      } else {
        const created:any[]=await sql`INSERT INTO players (id,steam_id,name,country,avatar) VALUES (${steamId},${steamId},${steamName},'XX',${steamAvatar}) RETURNING *`;
        internalPlayerId = created[0].id;
        p=dbPlayer(created[0]);
      }
    } else {
      p=players.find((x)=>x.id===steamId);
      if(!p){p=player(steamId,steamName,'XX',players.length+1,0);players.push(p);}
      else {p.name=steamName; p.playerNameInGame=steamName; p.avatar=steamAvatar || p.avatar;}
    }
    const token=tokenFor(internalPlayerId); const redirectTo=query.get('redirectTo')||'/';
    return new Response(null,{status:302,headers:{Location:redirectTo,'set-cookie':`token=${encodeURIComponent(token)}; Path=/; HttpOnly; SameSite=Lax; Secure; Max-Age=2592000`}});
  }
  if (route === '/auth/token' && method === 'GET') {const token=tokenFromCookie(request.headers.get('cookie')||undefined);return token?json({token}):json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);}
  if (route === '/auth/logout' && method === 'POST') return json({ok:true},200,{'set-cookie':'token=; Path=/; HttpOnly; SameSite=Lax; Secure; Max-Age=0'});

  return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:`SnoreSaber endpoint ${method} ${route} is not implemented yet`},404);
});
