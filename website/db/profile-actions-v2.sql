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

INSERT INTO badges (image, description, image_url)
SELECT * FROM (VALUES
  ('snoresaber-icon.png','SnoreSaber Staff','/assets/snoresaber-icon.png'),
  ('snoresaber-icon.png','Early Supporter','/assets/snoresaber-icon.png'),
  ('snoresaber-icon.png','Verified Player','/assets/snoresaber-icon.png'),
  ('snoresaber-icon.png','Map Contributor','/assets/snoresaber-icon.png'),
  ('snoresaber-icon.png','Tournament Staff','/assets/snoresaber-icon.png')
) AS seed(image,description,image_url)
WHERE NOT EXISTS (SELECT 1 FROM badges);
