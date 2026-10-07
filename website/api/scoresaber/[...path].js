export default async function handler(req, res) {
  if (req.method !== 'GET' && req.method !== 'HEAD') return res.status(405).json({ error: 'GET/HEAD only' });
  const raw = req.query?.path || '';
  const path = Array.isArray(raw) ? raw.join('/') : raw;
  const query = req.url?.includes('?') ? req.url.slice(req.url.indexOf('?')) : '';
  const target = `https://scoresaber.com/api/${path}${query}`;

  try {
    const upstream = await fetch(target, { method: req.method, headers: { accept: 'application/json', 'user-agent': 'SnoreSaber/1.0' } });
    const body = await upstream.text();
    res.status(upstream.status);
    res.setHeader('content-type', upstream.headers.get('content-type') || 'application/json');
    res.setHeader('cache-control', 's-maxage=30, stale-while-revalidate=120');
    return res.send(body);
  } catch (error) {
    return res.status(502).json({ error: 'ScoreSaber API proxy failed', detail: String(error?.message || error) });
  }
}
