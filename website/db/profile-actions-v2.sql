-- SnoreSaber profile actions / moderation v2
-- Safe to run repeatedly.
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

CREATE TABLE IF NOT EXISTS profile_reports (
  id BIGSERIAL PRIMARY KEY,
  reporter_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  target_player_id TEXT NOT NULL REFERENCES players(id) ON DELETE CASCADE,
  reason TEXT NOT NULL,
  details TEXT NOT NULL DEFAULT '',
  status TEXT NOT NULL DEFAULT 'OPEN',
  created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS account_merges (
  id BIGSERIAL PRIMARY KEY,
  target_player_id TEXT NOT NULL,
  source_player_id TEXT NOT NULL,
  reason TEXT NOT NULL,
  merged_by TEXT NOT NULL,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

UPDATE badges SET description='SnoreSaber Tester', image='tester.svg', image_url='/assets/badges/tester.svg' WHERE description='Early Supporter' AND NOT EXISTS (SELECT 1 FROM badges WHERE description='SnoreSaber Tester');

INSERT INTO badges (image, description, image_url)
SELECT * FROM (VALUES
  ('staff.svg','SnoreSaber Staff','/assets/badges/staff.svg'),
  ('tester.svg','SnoreSaber Tester','/assets/badges/tester.svg'),
  ('verified.svg','Verified Player','/assets/badges/verified.svg'),
  ('mapper.svg','Map Contributor','/assets/badges/mapper.svg'),
  ('tournament.svg','Tournament Staff','/assets/badges/tournament.svg'),
  ('developer.svg','SnoreSaber Developer','/assets/badges/developer.svg')
) AS seed(image,description,image_url)
WHERE NOT EXISTS (SELECT 1 FROM badges WHERE description = seed.description);

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
