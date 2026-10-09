# BeatSaver catalog import

SnoreSaber no longer uses a Vercel Cron job for BeatSaver synchronization.

## One-time full catalog import

From the website project directory:

```bash
pnpm install
node --env-file=.env scripts/import-beatsaver.mjs --full
```

This walks BeatSaver's `/maps/latest` cursor until it reaches the oldest available page and writes the maps and difficulties into Neon. It is resumable at the database level by using the unique map hash/ID, so re-running it is safe.

## Later incremental syncs

Run:

```bash
node --env-file=.env scripts/import-beatsaver.mjs --new
```

This checks BeatSaver from the newest maps forward and imports new/changed maps into the existing Neon cache.

## Automatic nightly sync without Cron

On Windows, use Task Scheduler to run the incremental command once per day. This is outside Vercel and does not consume Vercel Cron jobs.

Program:

```text
C:\Program Files\nodejs\node.exe
```

Arguments:

```text
--env-file=.env scripts/import-beatsaver.mjs --new
```

Start in:

```text
<your SnoreSaber website folder>
```

Set the trigger to daily at midnight.

The website itself reads the cached map catalog from Neon, so browsing `/maps` does not need to contact BeatSaver.

## Sync exactly the six curated maps

After installing dependencies and restoring `.env`, run:

    node --env-file=.env scripts/import-beatsaver.mjs --sync-curated

This directly fetches these six BeatSaver map keys:
25198, 4fdd2, 52dfb, 4e692, 4d977, 51e10

It upserts each one, deletes every other cached map, and verifies that all six
are present in Neon before exiting successfully.

## Reset the six maps so SnoreSaber can use its own rankings

Run this once after deploying the ranking-source change:

    npm run beatsaver:reset-rankings

This sets every leaderboard on the six curated maps to UNRANKED with 0 stars.
After that, use the **Map Admin** button on the Maps page to paste a curated
BeatSaver link and assign SnoreSaber star values per difficulty.
