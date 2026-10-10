-- SnoreSaber 3.0 core database
CREATE TABLE IF NOT EXISTS players (
  id TEXT PRIMARY KEY,
  steam_id TEXT UNIQUE,
  login_email TEXT,
  password_hash TEXT,
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
  vanity_changed_at TIMESTAMPTZ,
  banned BOOLEAN NOT NULL DEFAULT false,
  silenced BOOLEAN NOT NULL DEFAULT false,
  ban_reason TEXT,
  ban_notes TEXT,
  ban_created_at TIMESTAMPTZ,
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
  is_ai BOOLEAN NOT NULL DEFAULT false,
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

-- Older PBs are archived by personal-best-scores-migration.sql before the unique
-- active-score index is applied to an existing installation.
CREATE TABLE IF NOT EXISTS score_history (
  id BIGSERIAL PRIMARY KEY,
  original_score_id BIGINT NOT NULL UNIQUE,
  leaderboard_id BIGINT NOT NULL,
  player_id TEXT NOT NULL,
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
  created_at TIMESTAMPTZ NOT NULL,
  archived_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE UNIQUE INDEX IF NOT EXISTS idx_scores_active_player_leaderboard ON scores(leaderboard_id, player_id);
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

-- SnoreSaber profile customization / badges
CREATE TABLE IF NOT EXISTS badges (
  id BIGSERIAL PRIMARY KEY,
  image TEXT NOT NULL,
  description TEXT NOT NULL,
  image_url TEXT NOT NULL DEFAULT '',
  created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS player_badges (
  player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  badge_id BIGINT NOT NULL REFERENCES badges(id) ON DELETE CASCADE,
  description_override TEXT,
  added_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  PRIMARY KEY (player_id, badge_id)
);

CREATE TABLE IF NOT EXISTS profile_customizations (
  player_id TEXT PRIMARY KEY REFERENCES players(id) ON DELETE CASCADE,
  background_image TEXT,
  background_image_version BIGINT NOT NULL DEFAULT 1,
  accent_color TEXT,
  accent_foreground_color TEXT,
  accent_foreground_active_color TEXT,
  supporter_name_color_enabled BOOLEAN NOT NULL DEFAULT true,
  badge_order BIGINT[],
  badge_comments JSONB,
  stat_order TEXT[],
  enabled_stat_ids TEXT[],
  chart_metric_ids TEXT[],
  section_order TEXT[],
  updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS pinned_scores (
  player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  score_id BIGINT NOT NULL REFERENCES scores(id) ON DELETE CASCADE,
  position INTEGER NOT NULL,
  comment TEXT NOT NULL DEFAULT '',
  PRIMARY KEY (player_id, score_id)
);


-- SnoreSaber ranking requests
CREATE TABLE IF NOT EXISTS rank_requests (
  id BIGSERIAL PRIMARY KEY,
  map_id BIGINT NOT NULL REFERENCES maps(id) ON DELETE CASCADE,
  description TEXT NOT NULL DEFAULT '',
  request_type TEXT NOT NULL DEFAULT 'RANK',
  approval_status TEXT NOT NULL DEFAULT 'PENDING',
  weight DOUBLE PRECISION NOT NULL DEFAULT 1,
  created_by TEXT REFERENCES players(id) ON DELETE SET NULL,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);
CREATE TABLE IF NOT EXISTS rank_request_difficulties (
  id BIGSERIAL PRIMARY KEY,
  request_id BIGINT NOT NULL REFERENCES rank_requests(id) ON DELETE CASCADE,
  leaderboard_id BIGINT NOT NULL REFERENCES leaderboards(id) ON DELETE CASCADE,
  description TEXT NOT NULL DEFAULT '',
  approval_status TEXT NOT NULL DEFAULT 'PENDING',
  UNIQUE(request_id, leaderboard_id)
);
CREATE TABLE IF NOT EXISTS rank_request_votes (
  difficulty_id BIGINT NOT NULL REFERENCES rank_request_difficulties(id) ON DELETE CASCADE,
  player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  group_name TEXT NOT NULL,
  vote TEXT NOT NULL,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  PRIMARY KEY(difficulty_id, player_id, group_name)
);
CREATE TABLE IF NOT EXISTS rank_request_comments (
  id BIGSERIAL PRIMARY KEY,
  difficulty_id BIGINT NOT NULL REFERENCES rank_request_difficulties(id) ON DELETE CASCADE,
  player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  group_name TEXT NOT NULL,
  comment TEXT NOT NULL,
  edited BOOLEAN NOT NULL DEFAULT false,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);


CREATE TABLE IF NOT EXISTS beatsaver_sync_state (
  id INTEGER PRIMARY KEY CHECK (id=1),
  last_sync_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  maps_synced INTEGER NOT NULL DEFAULT 0
);
ALTER TABLE beatsaver_sync_state ADD COLUMN IF NOT EXISTS bootstrap_before TIMESTAMPTZ;
ALTER TABLE beatsaver_sync_state ADD COLUMN IF NOT EXISTS bootstrap_complete BOOLEAN NOT NULL DEFAULT false;
ALTER TABLE beatsaver_sync_state ADD COLUMN IF NOT EXISTS newest_uploaded_at TIMESTAMPTZ;

CREATE UNIQUE INDEX IF NOT EXISTS idx_players_login_email ON players (lower(login_email)) WHERE login_email IS NOT NULL;


-- Fixed calendar-quarter seasonal rankings (UTC quarter boundaries).
CREATE TABLE IF NOT EXISTS snore_seasons (
  season_key TEXT PRIMARY KEY,
  starts_at DATE NOT NULL,
  ends_at DATE NOT NULL,
  activated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);
CREATE TABLE IF NOT EXISTS snore_season_maps (
  season_key TEXT NOT NULL REFERENCES snore_seasons(season_key) ON DELETE CASCADE,
  leaderboard_id BIGINT NOT NULL,
  stars DOUBLE PRECISION NOT NULL DEFAULT 0,
  PRIMARY KEY (season_key, leaderboard_id)
);
CREATE TABLE IF NOT EXISTS snore_next_season_maps (
  leaderboard_id BIGINT PRIMARY KEY,
  stars DOUBLE PRECISION NOT NULL DEFAULT 0,
  added_by TEXT,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_snore_season_maps_leaderboard ON snore_season_maps(leaderboard_id);
