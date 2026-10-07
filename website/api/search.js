const BASE = 'https://scoresaber.com/api/v2/';
async function get(path){const r=await fetch(BASE+path,{headers:{accept:'application/json','user-agent':'SnoreSaber/1.0'}});if(!r.ok)throw new Error(`ScoreSaber returned ${r.status}`);return r.json();}
const arr=x=>Array.isArray(x?.data)?x.data:Array.isArray(x)?x:[];
export default async function handler(req,res){
 if(req.method!=='GET')return res.status(405).json({error:'GET only'});
 const q=String(req.query?.q||'').trim();
 if(q.length<2)return res.status(400).json({error:'Query is too short.'});
 try{
  const [players,maps]=await Promise.allSettled([
   get(`players?page=1&limit=8&search=${encodeURIComponent(q)}`),
   get(`maps?page=1&limit=8&search=${encodeURIComponent(q)}`)
  ]);
  res.setHeader('Cache-Control','s-maxage=30, stale-while-revalidate=120');
  return res.status(200).json({
   players: players.status==='fulfilled'?arr(players.value).map(p=>({id:p.id||p.playerId,name:p.name||p.playerNameInGame,country:p.country,avatar:p.avatar||p.avatarUrl,rank:p.rank,pp:p.pp})):[],
   maps: maps.status==='fulfilled'?arr(maps.value).map(m=>({id:m.id,hash:m.hash,name:m.songName||m.name,subName:m.songSubName,mapper:m.levelAuthorName||m.mapper,cover:m.coverUrl||m.coverImage,stars:m.stars||m.realm?.stars})):[]
  });
 }catch(e){return res.status(502).json({error:e?.message||'Search failed'});}
}
