export default async function handler(req, res) {
  const base = process.env.SABERRANK_API_URL;
  if (!base) return res.status(500).json({ error: 'SABERRANK_API_URL is not configured' });
  const path = Array.isArray(req.query.path) ? req.query.path.join('/') : (req.query.path || '');
  const target = new URL(path + (req.url.includes('?') ? '?' + req.url.split('?')[1] : ''), base.endsWith('/') ? base : base + '/');
  const headers = { ...req.headers };
  delete headers.host;
  delete headers['content-length'];
  let body;
  if (req.method !== 'GET' && req.method !== 'HEAD') {
    body = typeof req.body === 'string' ? req.body : JSON.stringify(req.body ?? {});
  }
  try {
    const upstream = await fetch(target, { method: req.method, headers, body, redirect: 'manual' });
    const text = await upstream.text();
    res.status(upstream.status);
    upstream.headers.forEach((v,k) => { if (k.toLowerCase() !== 'content-encoding') res.setHeader(k,v); });
    return res.send(text);
  } catch (e) {
    return res.status(502).json({ error: 'SaberRank API proxy failed', detail: String(e) });
  }
}
