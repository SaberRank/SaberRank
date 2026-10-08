import { createHmac, randomBytes } from 'node:crypto';
import { defineHandler } from 'nitro';
import { db } from '../../utils/db';

const NOW = new Date().toISOString();
const SECRET = process.env.SESSION_SECRET || 'snoresaber-development-secret-change-me';

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
    createdAt: NOW, lastSeenAt: NOW, badges: [], relationships: { following: [], mutuals: [] }
  };
}

function realm(stars: number) {
  return { realmId: 1, realmName: 'SnoreSaber', leaderboardStatus: 'RANKED', positiveModifiers: false, stars, rankedAt: NOW, qualifiedAt: null, lovedAt: null };
}

function map(id: number, hash: string, songName: string, sub: string, author: string, mapper: string, bpm: number, totalScores: number, stars: number) {
  return {
    id, hash, bsid: null, songName, songSubName: sub, songAuthorName: author, levelAuthorName: mapper,
    bpm, coverUrl: '/assets/snoresaber-icon.png', verified: true, totalScores, dailyScores: Math.floor(totalScores / 90), createdAt: NOW,
    leaderboards: [leaderboard(id * 10, id, 9, 'Standard', 'ExpertPlus', stars, totalScores)]
  };
}

function leaderboard(id: number, mapId: number, difficulty: number, gameMode: string, rawDifficulty: string, stars: number, totalScores: number) {
  return { id, difficulty, gameMode, rawDifficulty, maxScore: 1000000, totalScores, dailyScores: Math.floor(totalScores / 90), createdAt: NOW, realm: realm(stars) };
}

function score(id: number, p: any, m: any, accuracy: number, modifiedScore: number, pp: number) {
  return {
    id, rank: p.stats.rank, unmodifiedScore: modifiedScore, modifiedScore, accuracy, pp, weight: 1,
    mods: '', badCuts: 0, missedNotes: 0, maxCombo: 999, fullCombo: true, hasReplay: false, personalBest: true,
    legacyHmdId: 2, version: '1.40.8', playOutcome: 'COMPLETED', playOutcomeTime: null, createdAt: NOW,
    player: { id: p.id, name: p.name, playerNameInGame: p.name, country: p.country, role: null, avatar: p.avatar, avatarVersion: 1, permissions: 0 },
    device: { hmd: 'Quest 2', controllerLeft: 'Touch', controllerRight: 'Touch' },
    leaderboard: leaderboard(m.id * 10, m.id, 9, 'Standard', 'ExpertPlus', m.leaderboards[0].realm.stars, m.totalScores)
  };
}

function metadata(total: number, page: number, limit: number) {
  return { page, itemsPerPage: limit, totalItems: total, totalPages: Math.max(1, Math.ceil(total / limit)) };
}

function json(data: unknown, status = 200, headers: Record<string,string> = {}) {
  return new Response(JSON.stringify(data), { status, headers: { 'content-type': 'application/json; charset=utf-8', ...headers } });
}

function tokenFor(playerId: string) {
  const body = Buffer.from(JSON.stringify({ sub: playerId, iat: Date.now() })).toString('base64url');
  const sig = createHmac('sha256', SECRET).update(body).digest('base64url');
  return `${body}.${sig}`;
}

function playerFromToken(token?: string | null) {
  if (!token) return null;
  const [body, sig] = token.split('.');
  if (!body || !sig) return null;
  const expected = createHmac('sha256', SECRET).update(body).digest('base64url');
  if (sig !== expected) return null;
  try { return players.find((p) => p.id === JSON.parse(Buffer.from(body, 'base64url').toString()).sub) ?? null; } catch { return null; }
}

function tokenFromCookie(raw: string | undefined) {
  raw = raw || '';
  const found = raw.split(';').map((x) => x.trim()).find((x) => x.startsWith('token='));
  return found ? decodeURIComponent(found.slice(6)) : null;
}

async function verifySteam(req: Request) {
  const url = new URL(req.url);
  const params = new URLSearchParams();
  url.searchParams.forEach((v, k) => { if (k.startsWith('openid.')) params.set(k, v); });
  params.set('openid.mode', 'check_authentication');
  const response = await fetch('https://steamcommunity.com/openid/login', { method: 'POST', headers: { 'content-type': 'application/x-www-form-urlencoded' }, body: params });
  const text = await response.text();
  if (!text.includes('is_valid:true')) return null;
  const claimed = url.searchParams.get('openid.claimed_id') || '';
  const match = claimed.match(/([0-9]{17})$/);
  return match?.[1] ?? null;
}

function dbPlayer(r:any) {
  return { id:r.id,name:r.name,playerNameInGame:r.name,role:null,avatar:r.avatar||'',avatarVersion:1,bio:null,country:r.country||'XX',permissions:0,banned:false,inactive:false,vanity:r.name.toLowerCase(),stats:{rank:r.rank||0,countryRank:r.country_rank||0,totalPP:Number(r.pp||0),totalScore:String(r.total_score||0),totalRankedScore:String(r.total_ranked_score||0),totalPlayedLeaderboards:0,totalPlayedRankedLeaderboards:0,totalSubmittedPlays:r.total_plays||0,totalReplayViews:0,averageAccuracy:Number(r.average_accuracy||0),weightedAverageAccuracy:Number(r.average_accuracy||0),completionAccuracy:Number(r.average_accuracy||0)},connections:[],replaySlots:[]};
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

  if (route === '/health') return json({ ok: true, service: 'SnoreSaber API', version: '3.0.0' });

  if (route === '/players' && method === 'GET') {
    const page = Math.max(1, Number(query.get('page') || 1)); const limit = Math.min(100, Math.max(1, Number(query.get('limit') || 50)));
    const search = (query.get('search') || '').toLowerCase();
    const filtered = players.filter((p) => !search || p.name.toLowerCase().includes(search));
    const sort = query.get('sort') || 'rank';
    filtered.sort((a,b) => sort === 'totalPP' ? b.stats.totalPP-a.stats.totalPP : a.stats.rank-b.stats.rank);
    const start = (page-1)*limit;
    return json({ data: filtered.slice(start,start+limit), metadata: metadata(filtered.length,page,limit) });
  }
  if (route === '/players/count') return json({ count: players.length });
  if (route.startsWith('/players/vanity/')) {
    const slug = decodeURIComponent(route.split('/').pop()!); const p = players.find(x => x.vanity === slug || x.name.toLowerCase() === slug.toLowerCase());
    return p ? json(p) : json({ statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found' },404);
  }
  if (route.startsWith('/players/') && method === 'GET') {
    const seg = route.split('/'); const id = decodeURIComponent(seg[2]); const p = players.find(x => x.id === id);
    if (!p) return json({ statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Player not found' },404);
    if (seg[3] === 'scores') {
      const limit = Math.min(100, Number(query.get('limit') || 8));
      const ps = scores.filter(s => s.player.id === p.id);
      return json({ data: ps.slice(0,limit), metadata: metadata(ps.length,Number(query.get('page')||1),limit) });
    }
    if (seg[3] === 'history' || seg[3] === 'global-history') return json({ data: [], metadata: metadata(0,1,50) });
    if (seg[3] === 'basic') return json({ id:p.id,name:p.name,country:p.country,avatar:p.avatar,stats:p.stats });
    return json(p);
  }

  if (route === '/leaderboards' && method === 'GET') {
    const data = maps.flatMap(m => m.leaderboards.map((l:any) => ({ id:l.id, map:m, difficulty:{ id:l.id,difficulty:l.difficulty,rawDifficulty:l.rawDifficulty,gameMode:l.gameMode }, maxScore:l.maxScore,totalScores:l.totalScores,dailyScores:l.dailyScores,createdAt:l.createdAt,realm:l.realm })));
    return json({ data, metadata: metadata(data.length,1,data.length) });
  }
  if (route.startsWith('/leaderboards/hash/')) {
    const hash = route.split('/')[2]; const m = maps.find(x=>x.hash.toLowerCase()===hash.toLowerCase());
    return m ? json(m.leaderboards[0]) : json({ statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Leaderboard not found' },404);
  }
  if (route.startsWith('/leaderboards/')) {
    const seg=route.split('/'); const id=Number(seg[2]); const m=maps.find(x=>x.leaderboards.some((l:any)=>l.id===id));
    if (!m) return json({ statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Leaderboard not found' },404);
    const lb=m.leaderboards.find((l:any)=>l.id===id)!;
    if (seg[3]==='scores') return json({ data:scores.map(s=>({...s,rank:s.player.id===players[0].id?1:2})), metadata:metadata(scores.length,1,50) });
    return json({ id:lb.id,map:m,difficulty:{id:lb.id,difficulty:lb.difficulty,rawDifficulty:lb.rawDifficulty,gameMode:lb.gameMode},maxScore:lb.maxScore,totalScores:lb.totalScores,dailyScores:lb.dailyScores,createdAt:lb.createdAt,realm:lb.realm });
  }

  if (route === '/maps' && method === 'GET') {
    const page=Math.max(1,Number(query.get('page')||1)); const limit=Math.min(100,Math.max(1,Number(query.get('limit')||50))); const search=(query.get('search')||'').toLowerCase();
    const filtered=maps.filter(m=>!search||m.songName.toLowerCase().includes(search)||m.levelAuthorName.toLowerCase().includes(search));
    return json({ data:filtered.slice((page-1)*limit,(page-1)*limit+limit), metadata:metadata(filtered.length,page,limit) });
  }
  if (route.startsWith('/maps/hash/')) { const hash=route.split('/')[2]; const m=maps.find(x=>x.hash.toLowerCase()===hash.toLowerCase()); return m?json(m):json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404); }
  if (route.startsWith('/maps/')) { const id=Number(route.split('/')[2]); const m=maps.find(x=>x.id===id); return m?json({...m,reuploadVersions:[]}):json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Map not found'},404); }

  if (route.startsWith('/scores/') && method === 'GET') {
    const id=Number(route.split('/')[2]); const s=scores.find(x=>x.id===id); if(!s)return json({statusCode:404,error:'Not Found',code:'NOT_FOUND',message:'Score not found'},404);
    if(route.endsWith('/history')) return json({data:[],metadata:metadata(0,1,50)});
    if(route.endsWith('/stats')) return json({score:s,leaderboard:s.leaderboard,scoreStats:null});
    return json(s);
  }

  if (route === '/user/@me' && method === 'GET') {
    const p=playerFromToken(tokenFromCookie(request.headers.get('cookie') || undefined));
    if(!p)return json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401);
    const sql=db();
    if (sql) {
      const rows:any[]=await sql`SELECT * FROM players WHERE id=${p.id} LIMIT 1`;
      if (rows[0]) return json(dbPlayer(rows[0]));
    }
    return json({...p, connections:[], replaySlots:[]});
  }

  if (route === '/scores/submit' && method === 'POST') {
    const bodyText=await request.text();
    let body:any; try{body=JSON.parse(bodyText||'{}')}catch{return json({error:'Invalid JSON'},400)}
    const playerId=String(body.playerId||playerFromToken(tokenFromCookie(request.headers.get('cookie') || undefined))?.id||'');
    if(!playerId||!body.mapHash)return json({error:'playerId and mapHash are required'},400);
    const m=maps.find(x=>x.hash.toLowerCase()===String(body.mapHash).toLowerCase());
    if(!m)return json({error:'Unknown map'},404);
    const scoreValue=Number(body.score||body.modifiedScore||0); const accuracy=Number(body.accuracy||0); const pp=Number(body.pp||0);
    const sql=db();
    if(sql){
      const lb=m.leaderboards[0];
      await sql`INSERT INTO scores (leaderboard_id,player_id,score,accuracy,pp,mods,bad_cuts,missed_notes,max_combo,full_combo,has_replay) VALUES (${lb.id},${playerId},${scoreValue},${accuracy},${pp},${String(body.mods||'')},${Number(body.badCuts||0)},${Number(body.missedNotes||0)},${Number(body.maxCombo||0)},${Boolean(body.fullCombo)},${Boolean(body.hasReplay)})`;
    }
    return json({accepted:true,playerId,mapHash:body.mapHash,score:scoreValue,accuracy,pp});
  }

  if (route === '/auth/steam' && method === 'GET') {
    const callback = new URL('/api/v2/auth/steam/callback', origin);
    callback.searchParams.set('redirectTo', query.get('redirectTo') || '/');
    callback.searchParams.set('state', randomBytes(16).toString('hex'));
    const steam = new URL('https://steamcommunity.com/openid/login');
    steam.searchParams.set('openid.ns','http://specs.openid.net/auth/2.0');
    steam.searchParams.set('openid.mode','checkid_setup');
    steam.searchParams.set('openid.return_to',callback.toString());
    steam.searchParams.set('openid.realm',origin);
    steam.searchParams.set('openid.identity','http://specs.openid.net/auth/2.0/identifier_select');
    steam.searchParams.set('openid.claimed_id','http://specs.openid.net/auth/2.0/identifier_select');
    return json({redirectUrl:steam.toString()});
  }

  if (route === '/auth/steam/callback' && method === 'GET') {
    const steamId=await verifySteam(request);
    if(!steamId) return new Response('Steam authentication failed', {status:401,headers:{'content-type':'text/plain'}});
    let p=players.find(x=>x.id===steamId);
    const sql=db();
    if (sql) {
      const rows:any[]=await sql`SELECT * FROM players WHERE id=${steamId} LIMIT 1`;
      if (rows[0]) p=dbPlayer(rows[0]);
      else {
        const name=`SteamUser_${steamId.slice(-5)}`;
        await sql`INSERT INTO players (id,steam_id,name,country,avatar) VALUES (${steamId},${steamId},${name},'XX','') ON CONFLICT (id) DO NOTHING`;
        const created:any[]=await sql`SELECT * FROM players WHERE id=${steamId} LIMIT 1`;
        p=dbPlayer(created[0]);
      }
    } else if(!p){ p=player(steamId,`SteamUser_${steamId.slice(-5)}`,'XX',players.length+1,0); }
    if (!players.some((x) => x.id === p.id)) players.push(p);
    const token=tokenFor(p.id); const redirectTo=query.get('redirectTo') || '/';
    return new Response(null,{status:302,headers:{Location:redirectTo,'set-cookie':`token=${encodeURIComponent(token)}; Path=/; HttpOnly; SameSite=Lax; Secure; Max-Age=2592000`}});
  }
  if (route === '/auth/token' && method === 'GET') { const token=tokenFromCookie(request.headers.get('cookie') || undefined); return token?json({token}):json({statusCode:401,error:'Unauthorized',code:'UNAUTHORIZED',message:'Not signed in'},401); }
  if (route === '/auth/logout' && method === 'POST') return json({ok:true}, 200, {'set-cookie':'token=; Path=/; HttpOnly; SameSite=Lax; Secure; Max-Age=0'});

  return json({ statusCode: 404, error: 'Not Found', code: 'NOT_FOUND', message: `SnoreSaber endpoint ${method} ${route} is not implemented yet` }, 404);
});
