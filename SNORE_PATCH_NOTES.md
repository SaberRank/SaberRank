# SnoreSaber source patch notes

This archive is based on the uploaded SnoreSaber source ZIP. Original uploads were left untouched. Local `.env` secrets, per-machine `*.user` settings, Git internals, dependencies, and generated build outputs are not included.

## Implemented in this patch

- Password/email authentication refresh now invalidates and refetches the root-shell query, so the signed-in user state can update immediately after login/logout without a manual page refresh.
- Removed the old 200-map playlist ceiling; playlist requests now support up to 50,000 entries and the client defaults to the full-pool limit of 50,000. Requests are still fetched in 100-map API pages.
- Map Admin now invalidates the router after a successful BeatSaver ranking save so map listings are refreshed.
- Removed automatic-unban scheduling and the Vercel cron. Bans now require manual unban, so the free Vercel deployment has no background-job dependency. Legacy database columns are left harmlessly in place for compatibility.
- Both PC game upload and API score submission reject equal/lower scores, replace the existing active PB row in place, and remove duplicate active rows. Score detail responses now use the `{ score: ... }` envelope expected by the replay viewer. Profile score queries now explicitly alias score, leaderboard, and map columns to prevent duplicate `id`/field collisions.
- Added a migration that archives duplicate/superseded active scores to `score_history`, retains the best score for each player/leaderboard, and creates the unique index needed for concurrency-safe PB submissions.
- Removed ranked-play-count displays from the profile/rankings UI. Completed ranked clears increment the remaining play count; failed/abandoned and unranked runs do not. ZZ recalculation runs immediately after an accepted PB.
- Added fixed UTC calendar-quarter seasons (Jan 1 / Apr 1 / Jul 1 / Oct 1), request-triggered rollover without cron, Seasonal Maps countdown, and Ranking Team-only Next Season curation. The queue can import a BeatSaver map without changing the live ranked pool and lets Ranking Team members set stars per difficulty. At rollover the previous ranked pool is unranked, queued maps become ranked, old active score records are archived, active scores are cleared, and all players are reset to ZZ 0. Rollover occurs on the first API request after the boundary, so it does not require cron but is not a guaranteed background execution at the exact boundary if the site receives no requests.
- Increased playlist export capacity to 50,000 maps and changed its default to the full-pool limit.

## Required deployment step

Before deploying the PB-only score handler to an existing production database, run `website/db/personal-best-scores-migration.sql` once. It is transactional and safe to rerun. Apply `website/db/quarterly-seasons-migration.sql` if you prefer to pre-create the season tables; the API also creates them lazily.

## Verification and limitations

- TypeScript/TSX syntax transpilation passed for the edited TS/TSX files, and `website/vercel.json` parsed successfully.
- A full website dependency install/build could not be run in this environment: the project requires pnpm 11.21.0 and Node 24, while this environment has Node 22 and cannot reach `registry.npmjs.org`.
- PC and Quest source changes include the in-game ZZ star/status display, Snore/ZZ visible terminology, and a more tolerant Ludus start-map match-ID check plus an eight-second room-join race wait. The PC mod, Quest mod, Ludus service, and replay viewer could not be rebuilt or runtime-tested here. The uploaded source ZIP did not contain `replay/scoresaber-watch/Library`; this patch does not fabricate that Unity-generated directory.
- This is a source patch, not a claim that every previously discussed cross-project feature has been fully implemented or verified. In particular, the Ludus menu-to-game transition remains unverified; in-game leaderboard routing and star labels were patched but could not be runtime-tested.
