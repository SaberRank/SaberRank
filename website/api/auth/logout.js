import {deleteSession, cookie} from '../_lib/auth.js';
export default async function handler(req, res) {
  if (req.method !== 'POST') return res.status(405).json({error: 'POST only'});
  try { await deleteSession(req); } catch (e) { console.error(e); }
  res.setHeader('Set-Cookie', cookie('snore_session', '', {maxAge: 0}));
  return res.status(200).json({ok: true});
}
