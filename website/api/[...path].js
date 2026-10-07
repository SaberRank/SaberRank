const SCORE_SABER = 'https://scoresaber.com/api/';

export default async function handler(req, res) {
  if (req.method !== 'GET' && req.method !== 'HEAD') return res.status(405).json({error: 'GET/HEAD only'});
  const path = Array.isArray(req.query.path) ? req.query.path.join('/') : (req.query.path || '');
  if (!path) return res.status(404).json({error: 'API route not found'});
  // Legacy pages in the inherited application used /api/...; never send users to a missing SaberRank backend.
  // ScoreSaber v2 data is available through /api/scoresaber/v2/... and should be used by SnoreSaber pages.
  return res.status(404).json({error: 'This SnoreSaber API route does not exist', hint: 'Use the SnoreSaber API endpoints under /api/scoresaber/.'});
}
