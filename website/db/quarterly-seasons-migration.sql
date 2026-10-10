BEGIN;
CREATE TABLE IF NOT EXISTS snore_seasons (season_key TEXT PRIMARY KEY, starts_at DATE NOT NULL, ends_at DATE NOT NULL, activated_at TIMESTAMPTZ NOT NULL DEFAULT now());
CREATE TABLE IF NOT EXISTS snore_season_maps (season_key TEXT NOT NULL REFERENCES snore_seasons(season_key) ON DELETE CASCADE, leaderboard_id BIGINT NOT NULL, stars DOUBLE PRECISION NOT NULL DEFAULT 0, PRIMARY KEY (season_key, leaderboard_id));
CREATE TABLE IF NOT EXISTS snore_next_season_maps (leaderboard_id BIGINT PRIMARY KEY, stars DOUBLE PRECISION NOT NULL DEFAULT 0, added_by TEXT, created_at TIMESTAMPTZ NOT NULL DEFAULT now());
COMMIT;
