erm what the freak


## SnoreSaber source restoration notes

- Before deploying the PB-only score handler to an existing database, run `db/personal-best-scores-migration.sql` once. It archives superseded scores in `score_history`, retains the best score as the active row, and adds the unique player/leaderboard index required for concurrency-safe submissions. The migration is transactional and can be rerun safely.
- The automatic-unban sweep runs during API traffic and is also triggered by the Vercel cron at `/api/v2/health` every five minutes. Existing databases must have the `ban_auto_unban` and `ban_auto_unbans_at` columns from `db/social-moderation-migration.sql`.
- Playlist downloads can request up to 5,000 entries; the server fetches map listings in pages of at most 100.
