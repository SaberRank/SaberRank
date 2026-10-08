import { createHmac, randomBytes, timingSafeEqual } from 'node:crypto';
import { defineHandler } from 'nitro';
import { db } from '../../utils/db';

const SECRET = process.env.SESSION_SECRET || 'snoresaber-development-secret-change-me';
const INGEST_KEY = process.env.SNORE_INGEST_KEY || '';
const STEAM_API_KEY = process.env.STEAM_API_KEY || '';
const NOW = () => new Date().toISOString();

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
    }, bio: null, vanity: name.toLowerCase(),
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
  const rows: any[] = await sql`SELECT permissions FROM players WHERE id=${playerId} LIMIT 1`;
  return Boolean(Number(rows[0]?.permissions || 0) & 16);
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
  PPV3: 512,
  CCT: 1024,
  CCTHead: 2048,
  CAT: 4096,
  RTR: 8192,
  EXTERNAL_DEV: 16384,
  TOURNAMENT_ORGANIZER: 32768
};

async function resolvePlayerId(sql: any, requestedId: string) {
  if (!sql) return requestedId;
  const rows: any[] = await sql`SELECT id FROM players WHERE id=${requestedId} OR steam_id=${requestedId} LIMIT 1`;
  return rows[0]?.id ?? null;
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
  await sql`UPDATE badges SET description='SnoreSaber Tester', image='tester.svg', image_url='/assets/badges/tester.svg' WHERE description='Early Supporter' AND NOT EXISTS (SELECT 1 FROM badges WHERE description='SnoreSaber Tester')`;
  const defaultBadges = [
    ['staff.svg', 'SnoreSaber Staff', '/assets/badges/staff.svg'],
    ['tester.svg', 'SnoreSaber Tester', '/assets/badges/tester.svg'],
    ['verified.svg', 'Verified Player', '/assets/badges/verified.svg'],
    ['mapper.svg', 'Map Contributor', '/assets/badges/mapper.svg'],
    ['tournament.svg', 'Tournament Staff', '/assets/badges/tournament.svg'],
    ['developer.svg', 'SnoreSaber Developer', '/assets/badges/developer.svg']
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
  return rows.map((r) => ({
    id: Number(r.id),
    image: r.image_url || r.image,
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
    vanity: r.vanity || r.name?.toLowerCase(), publicLivePresenceOptOut: false,
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
    dailyScores: Number(r.daily_scores || 0), createdAt: new Date(r.created_at).toISOString(), leaderboards: lbs
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

async function dbMapWithLeaderboards(sql: any, mapId: number) {
  const mapsRows: any[] = await sql`SELECT * FROM maps WHERE id=${mapId} LIMIT 1`;
  if (!mapsRows[0]) return null;
  const lbRows: any[] = await sql`
    SELECT l.*, COUNT(s.id)::int AS total_scores
    FROM leaderboards l LEFT JOIN scores s ON s.leaderboard_id=l.id
    WHERE l.map_id=${mapId}
    GROUP BY l.id ORDER BY l.difficulty ASC, l.id ASC`;
  return dbMap(mapsRows[0], lbRows.map((x) => dbLeaderboard(x, Number(x.total_scores || 0))));
}

async function resolveInternalPlayerId(sql: any, publicOrInternalId: string) {
  if (!sql) return publicOrInternalId;
  const rows: any[] = await sql`SELECT id FROM players WHERE id=${publicOrInternalId} OR steam_id=${publicOrInternalId} LIMIT 1`;
  return rows[0]?.id ?? null;
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
  for (const row of bestAll) allScore += Number(row[1].score || 0);

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
      total_played_leaderboards=${totalLeaderboards},
      total_played_ranked_leaderboards=${rankedLeaderboards},
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
      const rows: any[] = await sql`SELECT * FROM players WHERE lower(name)=${slug} OR lower(id)=${slug} OR lower(steam_id)=${slug} LIMIT 1`;
      return rows[0] ? json(dbPlayer(rows[0])) : json({ statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found' },404);
    }
    const p = players.find((x) => x.vanity === slug || x.name.toLowerCase() === slug);
    return p ? json(p) : json({ statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found' },404);
  }
  if (route.startsWith('/players/') && route.endsWith('/profile') && method === 'GET') {
    const seg = route.split('/');
    const requestedId = decodeURIComponent(seg[2]);
    if (sql) {
      const pr: any[] = await sql`SELECT * FROM players WHERE id=${requestedId} OR steam_id=${requestedId} LIMIT 1`;
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
    const targetRows: any[] = await sql`SELECT id FROM players WHERE id=${requestedId} OR steam_id=${requestedId} LIMIT 1`;
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
    const targetRows: any[] = await sql`SELECT id FROM players WHERE id=${requestedId} OR steam_id=${requestedId} LIMIT 1`;
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
    const targetRows: any[] = await sql`SELECT id FROM players WHERE id=${requestedId} OR steam_id=${requestedId} LIMIT 1`;
    if (!targetRows[0]) return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found'},404);
    await sql`DELETE FROM player_follows WHERE follower_id=${viewerId} AND following_id=${targetRows[0].id}`;
    return json({success:true});
  }

  if (route.startsWith('/players/') && method === 'GET') {
    const seg = route.split('/'); const id = decodeURIComponent(seg[2]);
    if (sql) {
      const pr: any[] = await sql`SELECT * FROM players WHERE id=${id} OR steam_id=${id} LIMIT 1`;
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
  if (route === '/maps' && method === 'GET') {
    const page = Math.max(1, Number(query.get('page') || 1)); const limit = Math.min(100, Math.max(1, Number(query.get('limit') || 50)));
    const search = (query.get('search') || '').toLowerCase(); const verified = query.get('verified');
    const minStars = Number(query.get('minStars') || 0); const maxStars = Number(query.get('maxStars') || 99);
    if (sql) {
      const rows: any[] = await sql`SELECT * FROM maps ORDER BY created_at DESC`;
      const lbs: any[] = await sql`SELECT l.*, COUNT(s.id)::int AS total_scores FROM leaderboards l LEFT JOIN scores s ON s.leaderboard_id=l.id GROUP BY l.id ORDER BY l.id`;
      const dataRows = rows.map((r) => ({ r, lbs: lbs.filter((l) => Number(l.map_id) === Number(r.id)) }));
      let filtered = dataRows.filter(({r,lbs}) => {
        const q = !search || r.song_name.toLowerCase().includes(search) || r.level_author_name.toLowerCase().includes(search) || r.hash.toLowerCase().includes(search);
        const v = verified == null || String(Boolean(r.verified)) === verified;
        const stars = lbs.length ? Math.max(...lbs.map((l) => Number(l.stars || 0))) : 0;
        return q && v && stars >= minStars && stars <= maxStars;
      });
      if (query.get('sortBy') === 'highestStars') filtered.sort((a,b) => (Number(b.lbs[0]?.stars||0)-Number(a.lbs[0]?.stars||0)));
      const slice = filtered.slice((page-1)*limit,(page-1)*limit+limit).map(({r,lbs}) => dbMap(r,lbs.map((x) => dbLeaderboard(x,Number(x.total_scores||0)))));
      return json({ data:slice, metadata:metadata(filtered.length,page,limit) });
    }
    let filtered = maps.filter((m) => (!search || m.songName.toLowerCase().includes(search) || m.levelAuthorName.toLowerCase().includes(search) || m.hash.toLowerCase().includes(search)) && (verified == null || String(m.verified) === verified) && (m.leaderboards[0]?.realm.stars || 0) >= minStars && (m.leaderboards[0]?.realm.stars || 0) <= maxStars);
    return json({ data:filtered.slice((page-1)*limit,(page-1)*limit+limit), metadata:metadata(filtered.length,page,limit) });
  }
  if (route.startsWith('/maps/hash/') && method === 'GET') {
    const hash = route.split('/')[2];
    if (sql) { const rows:any[] = await sql`SELECT * FROM maps WHERE lower(hash)=lower(${hash}) LIMIT 1`; return rows[0] ? json(await dbMapWithLeaderboards(sql, Number(rows[0].id))) : json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404); }
    const m = maps.find((x) => x.hash.toLowerCase() === hash.toLowerCase()); return m ? json(m) : json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404);
  }
  if (route.startsWith('/maps/') && method === 'GET') {
    const id = Number(route.split('/')[2]);
    if (sql) { const m = await dbMapWithLeaderboards(sql,id); return m ? json(m) : json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404); }
    const m = maps.find((x) => x.id === id); return m ? json({...m,reuploadVersions:[]}) : json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404);
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
      id:Number(r.id), image:r.image, description:r.description,
      imageUrl:r.image_url || r.image, assignmentCount:Number(r.assignment_count || 0)
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
  if (route === '/user/@me/name' && method === 'PUT') {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true});
    let body:any={}; try{body=JSON.parse(await request.text()||'{}')}catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    const name=String(body.name||'').trim().slice(0,128);
    if(!name) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Name is required'},400);
    await sql`UPDATE players SET name=${name},last_seen_at=now() WHERE id=${pid}`;
    return json({success:true});
  }

  if (route === '/user/@me/bio' && method === 'PUT') {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true});
    let body:any={}; try{body=JSON.parse(await request.text()||'{}')}catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    const bio=String(body.bio||'').slice(0,4096);
    await sql`UPDATE players SET bio=${bio||null},last_seen_at=now() WHERE id=${pid}`;
    return json({success:true});
  }

  if (route === '/user/@me/vanity' && method === 'POST') {
    const pid=await authPlayerId(request,sql);
    if(!pid) return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    if(!sql) return json({success:true});
    let body:any={}; try{body=JSON.parse(await request.text()||'{}')}catch{return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Invalid JSON'},400)}
    const slug=String(body.slug||'').trim().toLowerCase();
    if(!/^[a-z0-9_-]{3,32}$/.test(slug)) return json({statusCode:400,error:'Bad Request',code:'VALIDATION_ERROR',message:'Vanity must be 3-32 characters using letters, numbers, _ or -'},400);
    const taken:any[]=await sql`SELECT id FROM players WHERE lower(vanity)=${slug} AND id<>${pid} LIMIT 1`;
    if(taken[0]) return json({statusCode:409,error:'Conflict',code:'ALREADY_EXISTS',message:'Vanity is already in use'},409);
    await sql`UPDATE players SET vanity=${slug} WHERE id=${pid}`;
    return json({success:true,vanity:slug});
  }

  if (route === '/user/@me/avatar' && method === 'POST') {
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
    return json({success:true});
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
