# SaberRank

## Components
- `mod/` — ScoreSaber PC mod code adapted for SaberRank and Beat Saber 1.40.x.
- `website/` — BeatLeader website source adapted for SaberRank; configured for Vercel via `SABERRANK_API_URL`.
- `server/` — BeatLeader server source adapted for SaberRank. This is the backend and is not a Vercel frontend deployment.
- `replay/` — supplied replay projects retained as supporting components.

## Before building
1. Set the SaberRank API domain in the mod endpoint configuration.
2. Set `SABERRANK_API_URL` in the website deployment.
3. Configure the server database, storage, authentication providers, and secrets using environment/appsettings values; never commit credentials.
4. Review `THIRD-PARTY-NOTICES.md`.

## Beat Saber target
The PC mod is configured around the ScoreSaber compatibility generation that supports Beat Saber 1.40.x, including 1.40.8.

## Deployment

### Website / Vercel
Set the Vercel environment variable `SABERRANK_API_URL` to the deployed SaberRank backend URL. The site calls `/api/*`, and `website/api/[...path].js` proxies those requests server-side.

### PC mod
Create `mod/Directory.Build.local.props` from the example and set `SaberRankGameDirectory1_40_0` to your Beat Saber instance and `SaberRankApiBaseUrl` / `SaberRankWebsiteBaseUrl` to the deployed domains.

### Important
The supplied server is a full database-backed ASP.NET backend. It requires its database/storage/authentication configuration before it can accept production traffic. The source is included and branded for SaberRank; credentials are intentionally not included.
