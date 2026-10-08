# SnoreSaber BeatSaver + Vanity

- `/u/<vanity>` only uses a vanity slug when the player has explicitly claimed one; otherwise profile links use the player ID.
- The public `/maps` page is curated to the six BeatSaver map keys currently approved for SnoreSaber: `25198`, `4fdd2`, `52dfb`, `4e692`, `4d977`, and `51e10`.
- The public map listing is ranked-only and the status/verified/star filter controls have been removed from the page.
- BeatSaver automapper/AI content is excluded by the importer (`automapper=false`). Existing cached catalogs from an earlier importer run can be cleaned with `--prune-ai`.
- The website reads map metadata from Neon; browsing `/maps` does not fetch the BeatSaver catalog live.
- No Vercel Cron job is configured. Run the local importer manually or with Windows Task Scheduler.
- Map-page administrator actions are backed by SnoreSaber API endpoints for rank, unrank, qualify, love, reweight/manual PP, and PP recalculation. They require administrator permissions.

## BeatSaver import

One-time full catalog import:

```bash
node --env-file=.env scripts/import-beatsaver.mjs --full
```

Incremental update:

```bash
node --env-file=.env scripts/import-beatsaver.mjs --new
```

Remove automapper/AI maps from an already-imported catalog:

```bash
node --env-file=.env scripts/import-beatsaver.mjs --prune-ai
```

For Windows Task Scheduler, use:

```text
Program: C:\Program Files\nodejs\node.exe
Arguments: --env-file=.env scripts/import-beatsaver.mjs --new
Start in: <SnoreSaber website folder>
```

Schedule the incremental command once per day. Vercel Cron is intentionally not used.


## Curated map catalog
SnoreSaber's public catalog is intentionally limited to these six BeatSaver keys: 25198, 4fdd2, 52dfb, 4e692, 4d977, 51e10. Other maps should not be shown or imported.
