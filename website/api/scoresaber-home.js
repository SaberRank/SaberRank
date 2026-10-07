const SCORE_SABER = 'https://scoresaber.com/api/v2/';

async function get(path) {
  const response = await fetch(SCORE_SABER + path, {
    headers: { accept: 'application/json', 'user-agent': 'SnoreSaber/1.0' },
  });
  if (!response.ok) throw new Error(`${response.status} ${path}`);
  return response.json();
}

const list = value => Array.isArray(value) ? value : Array.isArray(value?.data) ? value.data : [];
const number = (...values) => values.find(v => typeof v === 'number' && Number.isFinite(v)) ?? 0;
const text = (...values) => values.find(v => typeof v === 'string' && v.length) ?? '';

function normalizePlayer(p, index) {
  const stats = p?.stats || {};
  return {
    id: text(p?.id, p?.playerId),
    name: text(p?.name, p?.playerNameInGame, p?.playerName, 'Unknown Player'),
    country: text(p?.country, stats?.country),
    avatar: text(p?.avatar, p?.avatarUrl, stats?.avatar),
    rank: number(p?.rank, stats?.rank, index + 1),
    pp: number(p?.pp, p?.performancePoints, stats?.pp),
  };
}

function normalizeMap(m, index) {
  const realm = m?.realm || {};
  return {
    id: number(m?.id, m?.leaderboardId),
    hash: text(m?.hash, m?.songHash),
    name: text(m?.songName, m?.name, 'Unknown Map'),
    subName: text(m?.songSubName),
    author: text(m?.songAuthorName, m?.songAuthor, 'Unknown Artist'),
    mapper: text(m?.levelAuthorName, m?.mapper, 'Unknown Mapper'),
    cover: text(m?.coverUrl, m?.coverImage, m?.coverImageUrl),
    stars: number(m?.stars, realm?.stars),
    plays: number(m?.plays, m?.totalScores, m?.dailyPlays),
    status: text(realm?.leaderboardStatus, m?.ranked ? 'RANKED' : m?.qualified ? 'QUALIFIED' : 'UNRANKED'),
    position: index + 1,
  };
}

function normalizeScore(s, owner) {
  const leaderboard = s?.leaderboard || s?.map || {};
  const player = s?.player || owner || {};
  const accuracy = number(s?.accuracy, s?.acc, s?.scoreAccuracy, s?.percentage);
  return {
    id: number(s?.id, s?.scoreId),
    playerId: text(player?.id, player?.playerId, owner?.id),
    playerName: text(player?.name, player?.playerNameInGame, owner?.name, 'Unknown Player'),
    country: text(player?.country, owner?.country),
    score: number(s?.score, s?.rawScore),
    accuracy: accuracy > 1 ? accuracy : accuracy * 100,
    pp: number(s?.pp, s?.performancePoints),
    rank: number(s?.rank, s?.leaderboardRank),
    timeset: text(s?.timeset, s?.timeSet, s?.createdAt, s?.playedAt),
    mapId: number(leaderboard?.id, leaderboard?.leaderboardId, s?.leaderboardId),
    mapName: text(leaderboard?.songName, leaderboard?.map?.songName, s?.songName, 'Unknown Map'),
    cover: text(leaderboard?.coverUrl, leaderboard?.coverImage, leaderboard?.map?.coverUrl, s?.coverUrl),
  };
}

export default async function handler(req, res) {
  if (req.method !== 'GET') return res.status(405).json({ error: 'GET only' });

  try {
    const [playersResponse, mapsResponse, playerCountResponse] = await Promise.all([
      get('players?page=1&limit=5&sort=rank&sortDirection=asc'),
      get('maps?page=1&limit=5&sort=plays&sortDirection=desc'),
      get('players/count'),
    ]);

    const players = list(playersResponse).map(normalizePlayer);
    const maps = list(mapsResponse).map(normalizeMap);

    const recent = await Promise.all(players.slice(0, 5).map(async player => {
      try {
        return await get(`players/${encodeURIComponent(player.id)}/scores?page=1&limit=1&sort=timeset&sortDirection=desc`);
      } catch (_) {
        try {
          return await get(`players/${encodeURIComponent(player.id)}/scores?page=1&limit=1`);
        } catch (_) {
          return null;
        }
      }
    }));

    const scores = recent
      .map((response, i) => {
        const score = list(response)[0];
        return score ? normalizeScore(score, players[i]) : null;
      })
      .filter(Boolean)
      .sort((a, b) => String(b.timeset).localeCompare(String(a.timeset)))
      .slice(0, 5);

    const playerTotal = number(playerCountResponse?.count, playerCountResponse?.total, playerCountResponse?.data?.count, playersResponse?.metadata?.total);
    const mapTotal = number(mapsResponse?.metadata?.total, mapsResponse?.total);

    res.setHeader('Cache-Control', 's-maxage=60, stale-while-revalidate=300');
    return res.status(200).json({
      source: 'ScoreSaber',
      updatedAt: new Date().toISOString(),
      stats: {
        players: playerTotal,
        maps: mapTotal,
        scores: number(playersResponse?.metadata?.totalScores, playersResponse?.totalScores),
      },
      players,
      maps,
      scores,
    });
  } catch (error) {
    console.error('SnoreSaber home data error:', error);
    return res.status(502).json({ error: 'Unable to load live ScoreSaber data', detail: String(error?.message || error) });
  }
}
