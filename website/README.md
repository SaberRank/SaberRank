# SnoreSaber 3.0

SnoreSaber is an independent Beat Saber leaderboard built from the ScoreSaber website's MIT-licensed frontend foundation and redesigned around the SnoreSaber brand.

## Architecture

- React + TanStack Start + Nitro
- SnoreSaber-owned `/api/v2/*` compatibility API
- Steam OpenID authentication
- PostgreSQL/Neon support through `DATABASE_URL`
- SnoreSaber score submission endpoint at `POST /api/v2/scores/submit`
- Ranked maps, players, leaderboards, scores, and profiles use SnoreSaber data

## Development

This project uses the Vite+ / pnpm toolchain defined by the upstream project.

```sh
pnpm install
pnpm dev
```

## Production

Vercel can deploy the Nitro Vercel preset directly. Set these environment variables for persistence:

```text
NEXT_PUBLIC_SITE_URL=https://snoresaber.vercel.app
NEXT_PUBLIC_API_URL=https://snoresaber.vercel.app
API_URL=https://snoresaber.vercel.app
SESSION_SECRET=<long random secret>
DATABASE_URL=<PostgreSQL connection string>
```

Apply `db/schema.sql` to the database before enabling persistent score storage.
