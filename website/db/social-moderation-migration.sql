-- SnoreSaber social/moderation + play-count migration
ALTER TABLE players ADD COLUMN IF NOT EXISTS total_ranked_plays INTEGER NOT NULL DEFAULT 0;
ALTER TABLE players ADD COLUMN IF NOT EXISTS total_played_leaderboards INTEGER NOT NULL DEFAULT 0;
ALTER TABLE players ADD COLUMN IF NOT EXISTS total_played_ranked_leaderboards INTEGER NOT NULL DEFAULT 0;
ALTER TABLE players ADD COLUMN IF NOT EXISTS permissions INTEGER NOT NULL DEFAULT 0;
ALTER TABLE players ADD COLUMN IF NOT EXISTS role TEXT;
ALTER TABLE players ADD COLUMN IF NOT EXISTS bio TEXT;
ALTER TABLE players ADD COLUMN IF NOT EXISTS vanity TEXT;
ALTER TABLE players ADD COLUMN IF NOT EXISTS banned BOOLEAN NOT NULL DEFAULT false;
ALTER TABLE players ADD COLUMN IF NOT EXISTS silenced BOOLEAN NOT NULL DEFAULT false;
ALTER TABLE players ADD COLUMN IF NOT EXISTS ban_reason TEXT;
ALTER TABLE players ADD COLUMN IF NOT EXISTS ban_notes TEXT;
ALTER TABLE players ADD COLUMN IF NOT EXISTS ban_created_at TIMESTAMPTZ;
ALTER TABLE players ADD COLUMN IF NOT EXISTS ban_auto_unban BOOLEAN NOT NULL DEFAULT false;
ALTER TABLE players ADD COLUMN IF NOT EXISTS ban_auto_unbans_at TIMESTAMPTZ;
ALTER TABLE players ADD COLUMN IF NOT EXISTS ban_earliest_appeal_date TIMESTAMPTZ;

CREATE TABLE IF NOT EXISTS player_follows (
  follower_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  following_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  PRIMARY KEY (follower_id, following_id),
  CHECK (follower_id <> following_id)
);

CREATE INDEX IF NOT EXISTS idx_player_follows_following ON player_follows(following_id, created_at DESC);
CREATE INDEX IF NOT EXISTS idx_player_follows_follower ON player_follows(follower_id, created_at DESC);

-- Backfill play counters from the scores currently stored.
UPDATE players p SET
  total_plays = COALESCE(x.total_plays, 0),
  total_ranked_plays = COALESCE(x.total_ranked_plays, 0),
  total_played_leaderboards = COALESCE(x.total_leaderboards, 0),
  total_played_ranked_leaderboards = COALESCE(x.ranked_leaderboards, 0)
FROM (
  SELECT s.player_id, COUNT(*)::int AS total_plays,
    COUNT(*) FILTER (WHERE l.status='RANKED')::int AS total_ranked_plays,
    COUNT(DISTINCT s.leaderboard_id)::int AS total_leaderboards,
    COUNT(DISTINCT s.leaderboard_id) FILTER (WHERE l.status='RANKED')::int AS ranked_leaderboards
  FROM scores s JOIN leaderboards l ON l.id=s.leaderboard_id
  GROUP BY s.player_id
) x WHERE p.id=x.player_id;
