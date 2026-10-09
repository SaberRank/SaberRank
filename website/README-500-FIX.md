# SnoreSaber website 500/runtime fix

This patch fixes two regressions in the previous website build:

1. `/server/api/v2/[...path].ts` called `ensureScoreAccuracyScale(sql)` before `const sql = db()` existed. That put `sql` in the temporal dead zone and caused every API request to fail with HTTP 500. The database client is now initialized before the migration check.
2. `src/modules/quest/components/quest-wizard.tsx` called `getRouteApi('/quest')` without importing `getRouteApi`, causing the browser `ReferenceError: getRouteApi is not defined`.

The score-stats-detail 404 shown in the browser is also consistent with a stale/mismatched Vercel asset deployment. Redeploy this corrected source as a fresh deployment; if Vercel offers a cache option, use a clean rebuild so the HTML and hashed JS assets are from the same build.
