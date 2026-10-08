import { neon } from '@neondatabase/serverless';
import crypto from 'node:crypto';

const mem = globalThis.__snoresaber_mem ||= {
  players: [
    { id: 1, steamId: null, name: 'YawningSylveon', alias: 'YawningSylveon', country: 'CA', pp: 16284.32, rank: 1, totalScore: 0, playCount: 0 },
    { id: 2, steamId: null, name: 'Lunaa', alias: 'Lunaa', country: 'US', pp: 16112.07, rank: 2, totalScore: 0, playCount: 0 },
    { id: 3, steamId: null, name: 'Kyouki', alias: 'Kyouki', country: 'JP', pp: 15998.44, rank: 3, totalScore: 0, playCount: 0 },
    { id: 4, steamId: null, name: 'Rho', alias: 'Rho', country: 'US', pp: 15781.20, rank: 4, totalScore: 0, playCount: 0 },
    { id: 5, steamId: null, name: 'Astra', alias: 'Astra', country: 'GB', pp: 15662.11, rank: 5, totalScore: 0, playCount: 0 }
  ],
  maps: [
    { id: 'imprinting', hash: 'DEMO-IMPRINTING', songName: 'Imprinting', mapper: 'Sotarks', difficulty: 'Expert+', characteristic: 'Standard', stars: 10.42, ranked: true, plays: 382144 },
    { id: 'bbkkbkk', hash: 'DEMO-BBKK', songName: 'B.B.K.K.B.K.K.', mapper: 'nora2r', difficulty: 'Expert+', characteristic: 'Standard', stars: 10.18, ranked: true, plays: 301552 },
    { id: 'kimi-no-bouken', hash: 'DEMO-KIMI', songName: 'Kimi no Bouken', mapper: 'Sotarks', difficulty: 'Expert+', characteristic: 'Standard', stars: 9.84, ranked: true, plays: 298771 },
    { id: 'ghost', hash: 'DEMO-GHOST', songName: 'Ghost', mapper: 'Rustic', difficulty: 'Expert+', characteristic: 'Standard', stars: 9.71, ranked: true, plays: 286905 },
    { id: 'machine-gun', hash: 'DEMO-MACHINE', songName: 'Machine Gun', mapper: 'Sotarks', difficulty: 'Expert+', characteristic: 'Standard', stars: 9.55, ranked: true, plays: 274663 }
  ],
  scores: [
    { id: 1, playerName: 'YawningSylveon', playerAlias: 'YawningSylveon', songName: 'Imprinting', accuracy: 98.42, pp: 512.31, score: 986421, playedAt: new Date(Date.now() - 2*60000).toISOString() },
    { id: 2, playerName: 'Lunaa', playerAlias: 'Lunaa', songName: 'Kimi no Bouken', accuracy: 97.15, pp: 498.12, score: 971502, playedAt: new Date(Date.now() - 5*60000).toISOString() },
    { id: 3, playerName: 'Kyouki', playerAlias: 'Kyouki', songName: 'Ghost', accuracy: 96.88, pp: 487.44, score: 968801, playedAt: new Date(Date.now() - 6*60000).toISOString() },
    { id: 4, playerName: 'Rho', playerAlias: 'Rho', songName: 'B.B.K.K.B.K.K.', accuracy: 97.63, pp: 481.09, score: 976301, playedAt: new Date(Date.now() - 8*60000).toISOString() },
    { id: 5, playerName: 'Astra', playerAlias: 'Astra', songName: 'Machine Gun', accuracy: 95.21, pp: 463.77, score: 952104, playedAt: new Date(Date.now() - 11*60000).toISOString() }
  ],
  events: [],
  clans: []
};
function db() { return process.env.DATABASE_URL ? neon(process.env.DATABASE_URL) : null; }
function send(res, code, body) { res.status(code).setHeader('Cache-Control', 'no-store').json(body); }
function cookie(res, value, maxAge = 2592000) { res.setHeader('Set-Cookie', `snoresaber_session=${value}; Path=/; Max-Age=${maxAge}; HttpOnly; Secure; SameSite=Lax`); }
function sign(value) { const secret = process.env.SESSION_SECRET || 'development-only-secret'; const sig = crypto.createHmac('sha256', secret).update(value).digest('base64url'); return `${value}.${sig}`; }
function verify(token) { if (!token) return null; const [value, sig] = token.split('.'); if (!value || !sig) return null; const secret = process.env.SESSION_SECRET || 'development-only-secret'; const expected = crypto.createHmac('sha256', secret).update(value).digest('base64url'); try { return crypto.timingSafeEqual(Buffer.from(sig), Buffer.from(expected)) ? value : null; } catch { return null; } }

export default async function handler(req, res) {
  const rawPath = req.query?.path;
  const pathParts = Array.isArray(rawPath) ? rawPath : (rawPath ? [rawPath] : []);
  const parts = pathParts.map((part) => decodeURIComponent(String(part)));
  const route = parts.join('/');
  const q = req.query || {};
  try {
    if (route === 'stats') {
      const sql = db();
      if (sql) {
        const [p] = await sql`SELECT COUNT(*)::int count FROM players`; const [m] = await sql`SELECT COUNT(*)::int count FROM maps`; const [s] = await sql`SELECT COUNT(*)::int count FROM scores`; const [c] = await sql`SELECT COUNT(*)::int count FROM clans`;
        return send(res, 200, { stats: { players:p.count, maps:m.count, scores:s.count, clans:c.count } });
      }
      return send(res, 200, { stats:{ players:mem.players.length, maps:mem.maps.length, scores:mem.scores.length, clans:mem.clans.length } });
    }

    if (route === 'rankings') {
      const limit = Math.min(Number(q.limit || 50), 100); const sql = db();
      if (sql) { const rows = await sql`SELECT id,name,alias,country,pp,rank,total_score AS "totalScore",play_count AS "playCount" FROM players ORDER BY pp DESC NULLS LAST LIMIT ${limit}`; return send(res,200,{players:rows}); }
      return send(res,200,{players:mem.players.slice().sort((a,b)=>(b.pp||0)-(a.pp||0)).slice(0,limit)});
    }

    if (route === 'maps') {
      const limit = Math.min(Number(q.limit || 50), 100); const sql = db();
      if (sql) { const rows = await sql`SELECT m.id,m.song_name AS "songName",m.mapper,m.difficulty,m.characteristic,m.stars,COUNT(s.id)::int AS plays FROM maps m LEFT JOIN scores s ON s.map_id=m.id GROUP BY m.id ORDER BY plays DESC,m.stars DESC LIMIT ${limit}`; return send(res,200,{maps:rows}); }
      return send(res,200,{maps:mem.maps.slice(0,limit)});
    }

    if (route.startsWith('maps/') && req.method === 'GET') {
      const id = decodeURIComponent(parts.slice(1).join('/')); const sql = db();
      if (sql) {
        const map = await sql`SELECT id,hash,song_name AS "songName",mapper,difficulty,characteristic,stars,ranked FROM maps WHERE id=${id} OR hash=${id} LIMIT 1`;
        if (!map[0]) return send(res,404,{error:'Map not found'});
        const scores = await sql`SELECT s.id,s.score,s.accuracy,s.pp,s.modifiers,s.max_combo AS "maxCombo",s.misses,s.full_combo AS "fullCombo",s.played_at AS "playedAt",p.name AS "playerName",p.alias AS "playerAlias" FROM scores s JOIN players p ON p.id=s.player_id WHERE s.map_id=${map[0].id} ORDER BY s.pp DESC,s.accuracy DESC LIMIT 100`;
        return send(res,200,{...map[0],scores});
      }
      const map = mem.maps.find(x=>String(x.id)===id || x.hash===id); if (!map) return send(res,404,{error:'Map not found'});
      return send(res,200,{...map,scores:mem.scores.filter(x=>String(x.mapId)===String(map.id)).sort((a,b)=>(b.pp||0)-(a.pp||0))});
    }

    if (route === 'scores' && req.method === 'GET') {
      const limit = Math.min(Number(q.limit || 25), 100); const sql = db();
      if (sql) { const rows = await sql`SELECT s.id,s.score,s.accuracy,s.pp,s.modifiers,s.max_combo AS "maxCombo",s.misses,s.full_combo AS "fullCombo",s.played_at AS "playedAt",p.name AS "playerName",p.alias AS "playerAlias",m.id AS "mapId",m.song_name AS "songName",m.difficulty FROM scores s JOIN players p ON p.id=s.player_id JOIN maps m ON m.id=s.map_id ORDER BY s.played_at DESC LIMIT ${limit}`; return send(res,200,{scores:rows}); }
      return send(res,200,{scores:mem.scores.slice().sort((a,b)=>new Date(b.playedAt)-new Date(a.playedAt)).slice(0,limit)});
    }

    if (route === 'events') { const sql=db(); if(sql) return send(res,200,{events:await sql`SELECT id,name,description,starts_at AS "startsAt",ends_at AS "endsAt" FROM events ORDER BY starts_at DESC NULLS LAST LIMIT 25`}); return send(res,200,{events:mem.events}); }
    if (route === 'clans') { const sql=db(); if(sql) return send(res,200,{clans:await sql`SELECT c.id,c.name,c.tag,COUNT(cm.player_id)::int AS "memberCount" FROM clans c LEFT JOIN clan_members cm ON cm.clan_id=c.id GROUP BY c.id ORDER BY "memberCount" DESC LIMIT 25`}); return send(res,200,{clans:mem.clans}); }

    if (route === 'search') {
      const term=String(q.q||'').trim(); if(!term) return send(res,400,{error:'Search query required'}); const sql=db();
      if(sql){ const p=await sql`SELECT name,alias FROM players WHERE name ILIKE ${'%'+term+'%'} OR alias ILIKE ${'%'+term+'%'} ORDER BY pp DESC LIMIT 1`; if(p[0]) return send(res,200,{result:{type:'player',...p[0]}}); const m=await sql`SELECT id,song_name AS "songName" FROM maps WHERE song_name ILIKE ${'%'+term+'%'} ORDER BY ranked DESC LIMIT 1`; if(m[0]) return send(res,200,{result:{type:'map',...m[0]}}); }
      const p=mem.players.find(x=>(x.name||'').toLowerCase().includes(term.toLowerCase()) || (x.alias||'').toLowerCase().includes(term.toLowerCase())); if(p) return send(res,200,{result:{type:'player',name:p.name,alias:p.alias}}); const m=mem.maps.find(x=>(x.songName||'').toLowerCase().includes(term.toLowerCase())); if(m) return send(res,200,{result:{type:'map',id:m.id,songName:m.songName}}); return send(res,404,{error:'No result'});
    }

    if (route === 'auth/me') {
      const token=req.headers.cookie?.match(/snoresaber_session=([^;]+)/)?.[1]; const steamId=verify(token); if(!steamId) return send(res,200,{player:null}); const sql=db();
      if(sql){ const p=await sql`SELECT id,name,alias,country,pp,rank,total_score AS "totalScore",play_count AS "playCount",avatar_url AS "avatarUrl" FROM players WHERE steam_id=${steamId} LIMIT 1`; return send(res,200,{player:p[0]||null}); }
      return send(res,200,{player:mem.players.find(p=>p.steamId===steamId)||null});
    }
    if (route === 'auth/logout') { cookie(res,'',0); return send(res,200,{ok:true}); }

    if (route === 'auth/steam') {
      const callback=process.env.STEAM_RETURN_URL; if(!callback) return send(res,500,{error:'STEAM_RETURN_URL is not configured'}); const u=new URL(callback); const params=new URLSearchParams({'openid.ns':'http://specs.openid.net/auth/2.0','openid.mode':'checkid_setup','openid.return_to':callback,'openid.realm':u.origin+'/','openid.identity':'http://specs.openid.net/auth/2.0/identifier_select','openid.claimed_id':'http://specs.openid.net/auth/2.0/identifier_select'}); res.status(302).setHeader('Location','https://steamcommunity.com/openid/login?'+params.toString()).end(); return;
    }
    if (route === 'auth/steam/callback') {
      const mode=q['openid.mode']; const claimed=q['openid.claimed_id']; if(mode!=='id_res'||!claimed) return send(res,400,{error:'Invalid Steam OpenID response'}); const steamId=claimed.split('/').pop(); const sql=db();
      if(sql){ await sql`INSERT INTO players (steam_id,name,alias) VALUES (${steamId},${'Steam '+steamId},${'steam_'+steamId}) ON CONFLICT (steam_id) DO NOTHING`; }
      else if(!mem.players.some(p=>p.steamId===steamId)) mem.players.push({id:Date.now(),steamId,name:'Steam '+steamId,alias:'steam_'+steamId,pp:0,rank:null,totalScore:0,playCount:0});
      cookie(res,sign(steamId)); const target=new URL(process.env.STEAM_RETURN_URL).origin+'/'; res.status(302).setHeader('Location',target).end(); return;
    }

    if (route.startsWith('players/')) {
      const alias=decodeURIComponent(parts.slice(1).join('/')); const sql=db();
      if(sql){ const p=await sql`SELECT id,name,alias,country,pp,rank,total_score AS "totalScore",play_count AS "playCount",avatar_url AS "avatarUrl" FROM players WHERE alias=${alias} OR name=${alias} LIMIT 1`; if(!p[0]) return send(res,404,{error:'Player not found'}); return send(res,200,p[0]); }
      const p=mem.players.find(x=>x.alias===alias||x.name===alias); return p?send(res,200,p):send(res,404,{error:'Player not found'});
    }

    if (route === 'scores' && req.method === 'POST') {
      if(!process.env.SNORE_INGEST_KEY || req.headers['x-snoresaber-key']!==process.env.SNORE_INGEST_KEY) return send(res,401,{error:'Unauthorized'});
      const body=typeof req.body==='string'?JSON.parse(req.body):req.body||{}; if(!body.steamId||!body.mapHash||typeof body.score!=='number'||typeof body.accuracy!=='number') return send(res,400,{error:'steamId, mapHash, score and accuracy are required'}); const sql=db();
      if(!sql){ const item={id:Date.now(),...body,playedAt:new Date().toISOString()}; mem.scores.push(item); return send(res,202,{accepted:true,id:item.id,persistent:false}); }
      const player=await sql`SELECT id FROM players WHERE steam_id=${body.steamId} LIMIT 1`; const map=await sql`SELECT id FROM maps WHERE hash=${body.mapHash} LIMIT 1`; if(!player[0]||!map[0]) return send(res,404,{error:'Player or map is not registered'});
      const inserted=await sql`INSERT INTO scores(player_id,map_id,score,accuracy,pp,modifiers,max_combo,misses,full_combo) VALUES(${player[0].id},${map[0].id},${body.score},${body.accuracy},${body.pp||0},${body.modifiers||[]},${body.maxCombo||null},${body.misses||null},${!!body.fullCombo}) RETURNING id`;
      await sql`UPDATE players SET pp=(SELECT COALESCE(SUM(pp),0) FROM scores WHERE player_id=${player[0].id}), total_score=(SELECT COALESCE(SUM(score),0) FROM scores WHERE player_id=${player[0].id}), play_count=(SELECT COUNT(*) FROM scores WHERE player_id=${player[0].id}), updated_at=NOW() WHERE id=${player[0].id}`;
      return send(res,202,{accepted:true,id:inserted[0].id,persistent:true});
    }

    return send(res,404,{error:'SnoreSaber API route not found'});
  } catch(e) { return send(res,500,{error:e.message||'Internal SnoreSaber error'}); }
}
