import {cookie, encodeSession, getSessionUser, safeUser} from '../_lib/auth.js';
export default async function handler(req,res){
 if(req.method!=='POST')return res.status(405).json({error:'POST only'});
 try{
  const user=getSessionUser(req); if(!user)return res.status(401).json({error:'Sign in with Steam first.'});
  const id=String(req.body?.scoresaberId||'').trim();
  if(!/^\d+$/.test(id))return res.status(400).json({error:'Enter a numeric ScoreSaber player ID.'});
  const r=await fetch(`https://scoresaber.com/api/v2/players/${id}`,{headers:{accept:'application/json','user-agent':'SnoreSaber/1.0'}});
  if(!r.ok)return res.status(404).json({error:'ScoreSaber player not found.'});
  const p=await r.json();
  if(String(p.id||p.playerId)!==String(user.steamId))return res.status(403).json({error:'That ScoreSaber profile is not the Steam account you signed into.'});
  const next={...user,scoresaberId:id,displayName:p.name||user.displayName,avatarUrl:p.avatar||p.avatarUrl||user.avatarUrl};
  res.setHeader('Set-Cookie',cookie('snore_session',encodeSession(next),{maxAge:60*60*24*30}));
  return res.status(200).json({ok:true,user:safeUser(next),player:p});
 }catch(e){return res.status(500).json({error:e?.message||'Unable to link profile'});}
}
