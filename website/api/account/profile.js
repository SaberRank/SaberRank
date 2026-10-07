import {getSessionUser, safeUser} from '../_lib/auth.js';
export default async function handler(req,res){
  if(req.method!=='GET') return res.status(405).json({error:'GET only'});
  try { const u=await getSessionUser(req); return res.status(200).json({user:safeUser(u)}); }
  catch(e){return res.status(500).json({error:e?.message||'Unable to load account'});}
}
