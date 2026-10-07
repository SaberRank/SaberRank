import {randomBytes} from 'node:crypto';
import {cookie} from '../_lib/auth.js';

export default async function handler(req, res) {
  if (req.method !== 'GET') return res.status(405).json({error: 'GET only'});
  const state = randomBytes(24).toString('base64url');
  const proto = req.headers['x-forwarded-proto'] || 'https';
  const host = req.headers.host;
  if (!host) return res.status(400).json({error: 'Missing host'});
  const returnTo = `${proto}://${host}/api/auth/steam/callback`;
  const params = new URLSearchParams({
    'openid.ns': 'http://specs.openid.net/auth/2.0',
    'openid.mode': 'checkid_setup',
    'openid.return_to': `${returnTo}?state=${encodeURIComponent(state)}`,
    'openid.realm': `${proto}://${host}/`,
    'openid.identity': 'http://specs.openid.net/auth/2.0/identifier_select',
    'openid.claimed_id': 'http://specs.openid.net/auth/2.0/identifier_select',
  });
  res.setHeader('Set-Cookie', cookie('snore_oauth_state', state, {maxAge: 600}));
  return res.redirect(302, `https://steamcommunity.com/openid/login?${params.toString()}`);
}
