-- SnoreSaber Steam login / sequential public player IDs migration.
-- Run this ONCE in Neon after deploying this version if your players table already exists.

BEGIN;

ALTER TABLE players ADD COLUMN IF NOT EXISTS player_number BIGINT;

CREATE SEQUENCE IF NOT EXISTS players_player_number_seq;
ALTER SEQUENCE players_player_number_seq OWNED BY players.player_number;
ALTER TABLE players ALTER COLUMN player_number SET DEFAULT nextval('players_player_number_seq');

WITH ranked AS (
  SELECT id, ROW_NUMBER() OVER (ORDER BY rank ASC, created_at ASC, id ASC) AS player_number
  FROM players
  WHERE player_number IS NULL
)
UPDATE players p
SET player_number = ranked.player_number
FROM ranked
WHERE p.id = ranked.id;

SELECT setval(
  'players_player_number_seq',
  GREATEST((SELECT COALESCE(MAX(player_number), 0) FROM players), 1),
  true
);

ALTER TABLE players ALTER COLUMN player_number SET NOT NULL;
CREATE UNIQUE INDEX IF NOT EXISTS players_player_number_key ON players(player_number);

COMMIT;
