import {getSessionUser, safeUser} from '../_lib/auth.js';
export default async function handler(req, res) {
  if (req.method !== 'GET') return res.status(405).json({error: 'GET only'});
  try { return res.status(200).json({user: safeUser(await getSessionUser(req))}); }
  catch (e) { console.error(e); return res.status(500).json({error: e?.message || 'Unable to load session'}); }
}
