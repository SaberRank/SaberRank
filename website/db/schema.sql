-- SnoreSaber 3.0 core database
CREATE TABLE IF NOT EXISTS players (
  id TEXT PRIMARY KEY,
  steam_id TEXT UNIQUE,
  name TEXT NOT NULL,
  country TEXT NOT NULL DEFAULT 'XX',
  avatar TEXT NOT NULL DEFAULT '',
  pp DOUBLE PRECISION NOT NULL DEFAULT 0,
  rank INTEGER NOT NULL DEFAULT 0,
  country_rank INTEGER NOT NULL DEFAULT 0,
  total_score NUMERIC(20,0) NOT NULL DEFAULT 0,
  total_ranked_score NUMERIC(20,0) NOT NULL DEFAULT 0,
  total_plays INTEGER NOT NULL DEFAULT 0,
  total_ranked_plays INTEGER NOT NULL DEFAULT 0,
  total_played_leaderboards INTEGER NOT NULL DEFAULT 0,
  total_played_ranked_leaderboards INTEGER NOT NULL DEFAULT 0,
  permissions INTEGER NOT NULL DEFAULT 0,
  role TEXT,
  bio TEXT,
  vanity TEXT,
  banned BOOLEAN NOT NULL DEFAULT false,
  silenced BOOLEAN NOT NULL DEFAULT false,
  ban_reason TEXT,
  ban_notes TEXT,
  ban_created_at TIMESTAMPTZ,
  ban_auto_unban BOOLEAN NOT NULL DEFAULT false,
  ban_auto_unbans_at TIMESTAMPTZ,
  ban_earliest_appeal_date TIMESTAMPTZ,
  average_accuracy DOUBLE PRECISION NOT NULL DEFAULT 0,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  last_seen_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS maps (
  id BIGSERIAL PRIMARY KEY,
  hash TEXT UNIQUE NOT NULL,
  bsid TEXT,
  song_name TEXT NOT NULL,
  song_sub_name TEXT NOT NULL DEFAULT '',
  song_author_name TEXT NOT NULL DEFAULT '',
  level_author_name TEXT NOT NULL DEFAULT '',
  bpm DOUBLE PRECISION NOT NULL DEFAULT 0,
  cover_url TEXT NOT NULL DEFAULT '',
  verified BOOLEAN NOT NULL DEFAULT false,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS leaderboards (
  id BIGSERIAL PRIMARY KEY,
  map_id BIGINT NOT NULL REFERENCES maps(id) ON DELETE CASCADE,
  difficulty INTEGER NOT NULL,
  game_mode TEXT NOT NULL DEFAULT 'Standard',
  raw_difficulty TEXT NOT NULL DEFAULT 'ExpertPlus',
  max_score INTEGER NOT NULL DEFAULT 1000000,
  stars DOUBLE PRECISION NOT NULL DEFAULT 0,
  status TEXT NOT NULL DEFAULT 'UNRANKED',
  ranked_at TIMESTAMPTZ,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  UNIQUE(map_id, difficulty, game_mode)
);

CREATE TABLE IF NOT EXISTS scores (
  id BIGSERIAL PRIMARY KEY,
  leaderboard_id BIGINT NOT NULL REFERENCES leaderboards(id) ON DELETE CASCADE,
  player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  score INTEGER NOT NULL,
  accuracy DOUBLE PRECISION NOT NULL,
  pp DOUBLE PRECISION NOT NULL DEFAULT 0,
  weight DOUBLE PRECISION NOT NULL DEFAULT 1,
  mods TEXT NOT NULL DEFAULT '',
  bad_cuts INTEGER NOT NULL DEFAULT 0,
  missed_notes INTEGER NOT NULL DEFAULT 0,
  max_combo INTEGER NOT NULL DEFAULT 0,
  full_combo BOOLEAN NOT NULL DEFAULT false,
  has_replay BOOLEAN NOT NULL DEFAULT false,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS sessions (
  token_hash TEXT PRIMARY KEY,
  player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  expires_at TIMESTAMPTZ NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_players_pp ON players(pp DESC);
CREATE INDEX IF NOT EXISTS idx_scores_leaderboard ON scores(leaderboard_id, score DESC);
CREATE INDEX IF NOT EXISTS idx_scores_player ON scores(player_id, created_at DESC);
CREATE INDEX IF NOT EXISTS idx_maps_name ON maps(song_name);


CREATE TABLE IF NOT EXISTS player_follows (
  follower_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  following_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  PRIMARY KEY (follower_id, following_id),
  CHECK (follower_id <> following_id)
);

CREATE INDEX IF NOT EXISTS idx_player_follows_following ON player_follows(following_id, created_at DESC);
CREATE INDEX IF NOT EXISTS idx_player_follows_follower ON player_follows(follower_id, created_at DESC);
