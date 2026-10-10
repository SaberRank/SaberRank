-- Run once against the production database before deploying the PB-only score handler.
-- Preserve superseded rows in score_history, retain the highest score as the active row,
-- and enforce one active score per player/leaderboard so concurrent uploads cannot duplicate it.
BEGIN;

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

WITH ranked AS (
  SELECT id, leaderboard_id, player_id, score, accuracy, pp, weight, mods, bad_cuts,
         missed_notes, max_combo, full_combo, has_replay, created_at,
         row_number() OVER (PARTITION BY leaderboard_id, player_id ORDER BY score DESC, created_at DESC, id DESC) AS position
  FROM scores
)
INSERT INTO score_history (original_score_id, leaderboard_id, player_id, score, accuracy, pp, weight, mods, bad_cuts, missed_notes, max_combo, full_combo, has_replay, created_at)
SELECT id, leaderboard_id, player_id, score, accuracy, pp, weight, mods, bad_cuts, missed_notes, max_combo, full_combo, has_replay, created_at
FROM ranked WHERE position > 1
ON CONFLICT (original_score_id) DO NOTHING;

WITH ranked AS (
  SELECT id, row_number() OVER (PARTITION BY leaderboard_id, player_id ORDER BY score DESC, created_at DESC, id DESC) AS position
  FROM scores
)
DELETE FROM scores WHERE id IN (SELECT id FROM ranked WHERE position > 1);

CREATE UNIQUE INDEX IF NOT EXISTS idx_scores_active_player_leaderboard ON scores(leaderboard_id, player_id);
COMMIT;
