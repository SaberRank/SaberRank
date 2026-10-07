import {createHmac, randomBytes, timingSafeEqual} from 'node:crypto';

function secret() {
  const value = process.env.SESSION_SECRET;
  if (!value || value.length < 32) throw new Error('SESSION_SECRET must be set to a random value of at least 32 characters');
  return value;
}

function sign(value) {
  return createHmac('sha256', secret()).update(value).digest('base64url');
}

export function encodeSession(user) {
  const payload = Buffer.from(JSON.stringify({...user, exp: Date.now() + 1000 * 60 * 60 * 24 * 30})).toString('base64url');
  return `${payload}.${sign(payload)}`;
}

export function decodeSession(token) {
  try {
    const [payload, signature] = String(token || '').split('.');
    if (!payload || !signature) return null;
    const expected = sign(payload);
    const a = Buffer.from(signature); const b = Buffer.from(expected);
    if (a.length !== b.length || !timingSafeEqual(a, b)) return null;
    const user = JSON.parse(Buffer.from(payload, 'base64url').toString('utf8'));
    if (!user.exp || user.exp < Date.now()) return null;
    delete user.exp;
    return user;
  } catch (_) { return null; }
}

export function parseCookies(req) {
  const out = {};
  const raw = req.headers.cookie || '';
  for (const piece of raw.split(';')) {
    const i = piece.indexOf('=');
    if (i < 0) continue;
    out[piece.slice(0, i).trim()] = decodeURIComponent(piece.slice(i + 1).trim());
  }
  return out;
}

export function cookie(name, value, options = {}) {
  const parts = [`${name}=${encodeURIComponent(value)}`, `Path=${options.path || '/'}`];
  if (options.maxAge != null) parts.push(`Max-Age=${options.maxAge}`);
  if (options.httpOnly !== false) parts.push('HttpOnly');
  if (options.secure !== false) parts.push('Secure');
  parts.push(`SameSite=${options.sameSite || 'Lax'}`);
  return parts.join('; ');
}

export function getSessionUser(req) {
  return decodeSession(parseCookies(req).snore_session);
}

export function safeUser(row) {
  if (!row) return null;
  return {
    id: row.scoresaberId || row.steamId,
    steamId: row.steamId,
    displayName: row.displayName,
    avatarUrl: row.avatarUrl || '',
    scoresaberId: row.scoresaberId || null,
  };
}
