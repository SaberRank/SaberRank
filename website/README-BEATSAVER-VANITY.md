# SnoreSaber BeatSaver + Vanity

- `/u/<vanity>` now resolves against `players.vanity`, `players.name`, player id, and Steam id.
- The profile, score, basic-player, follow, and relationship API paths all resolve vanity slugs.
- `/api/v2/maps` automatically bootstraps the database from BeatSaver when the local map table is empty.
- `/api/v2/maps/sync` can be invoked by an admin or by Vercel Cron with `Authorization: Bearer $CRON_SECRET`.
- Vercel runs the sync daily at 04:00 UTC. Hobby plans support daily cron jobs; set `CRON_SECRET` in Vercel Project Settings > Environment Variables.
- The sync imports the newest BeatSaver maps and their published difficulty spreads into the local `maps` and `leaderboards` tables. Map packages/audio are not mirrored into SnoreSaber.
- The maps page now shows all synced maps by default; the existing Verified filter remains available when explicitly selected.

BeatSaver's public API exposes `/maps/latest` with up to 100 results per request, and map versions provide the map hash/key and downloadable map metadata. SnoreSaber stores metadata and links back to BeatSaver rather than hosting the beatmap packages.
