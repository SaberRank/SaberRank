import {cookie, encodeSession, parseCookies} from '../../_lib/auth.js';

function redirect(res, path) { return res.redirect(302, path); }

export default async function handler(req, res) {
  if (req.method !== 'GET') return res.status(405).send('GET only');
  try {
    const query = req.query || {};
    const cookies = parseCookies(req);
    const state = typeof query.state === 'string' ? query.state : '';
    if (!state || state !== cookies.snore_oauth_state) return redirect(res, '/signin?error=invalid_state');
    if (query['openid.mode'] !== 'id_res') return redirect(res, '/signin?error=steam_denied');

    const verifyParams = new URLSearchParams();
    for (const [key, value] of Object.entries(query)) {
      if (key === 'state') continue;
      verifyParams.set(key, Array.isArray(value) ? String(value[0]) : String(value));
    }
    verifyParams.set('openid.mode', 'check_authentication');
    const verify = await fetch('https://steamcommunity.com/openid/login', {
      method: 'POST', headers: {'content-type': 'application/x-www-form-urlencoded', accept: 'text/plain'}, body: verifyParams.toString()
    });
    const verification = await verify.text();
    if (!verify.ok || !/is_valid\s*:\s*true/i.test(verification)) return redirect(res, '/signin?error=steam_verification_failed');

    const claimed = String(query['openid.claimed_id'] || '');
    const match = claimed.match(/\/id\/(\d+)\/?$/);
    if (!match) return redirect(res, '/signin?error=steam_id_missing');
    const steamId = match[1];

    let displayName = `Steam ${steamId}`;
    let avatarUrl = '';
    try {
      const profileResponse = await fetch(`https://steamcommunity.com/profiles/${steamId}?xml=1`, {headers:{accept:'application/xml'}});
      const xml = await profileResponse.text();
      displayName = xml.match(/<steamID><!\[CDATA\[(.*?)\]\]><\/steamID>/)?.[1] || displayName;
      avatarUrl = xml.match(/<avatarFull><!\[CDATA\[(.*?)\]\]><\/avatarFull>/)?.[1] || '';
    } catch (_) {}

    let scoresaberId = steamId;
    try {
      const ss = await fetch(`https://scoresaber.com/api/v2/players/${steamId}`, {headers:{accept:'application/json','user-agent':'SnoreSaber/1.0'}});
      if (!ss.ok) scoresaberId = null;
    } catch (_) { scoresaberId = null; }

    const session = encodeSession({steamId, displayName, avatarUrl, scoresaberId});
    res.setHeader('Set-Cookie', [
      cookie('snore_session', session, {maxAge:60*60*24*30}),
      cookie('snore_oauth_state', '', {maxAge:0}),
    ]);
    return redirect(res, '/?login=success');
  } catch (error) {
    console.error('Steam auth callback failed:', error);
    return redirect(res, `/signin?error=${encodeURIComponent(error?.message || 'server_error')}`);
  }
}
