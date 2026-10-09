-- SnoreSaber: remove every map except the six curated BeatSaver maps.
-- leaderboards and scores cascade from maps via the project schema.
BEGIN;
DELETE FROM maps
WHERE COALESCE(bsid, '') NOT IN
  ('25198','4fdd2','52dfb','4e692','4d977','51e10');
COMMIT;
