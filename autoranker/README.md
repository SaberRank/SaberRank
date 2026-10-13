# SnoreSaber Autoranker — real BeatLeader analyzer integration

The website now calls BeatLeader's actual `RatingAPI` for every published BeatSaver difficulty. It reads the unmodified `none.star_rating` result, saves it as a pending proposal, and does not publish it until an admin approves it. The admin can still adjust a proposed value before approval.

## 1. Deploy the analyzer service

The website runs on Vercel; the .NET 10 analyzer should run as a separate Docker service (for example, on a container host). Build from `autoranker/worker`:

```sh
docker build -t snoresaber-ratingapi ./autoranker/worker
docker run --rm -p 8080:8080 \
  -e AutorankerApiKey='REPLACE_WITH_A_LONG_RANDOM_SECRET' \
  -e MapsPath=/tmp/maps \
  snoresaber-ratingapi
```

The Dockerfile clones BeatLeader's `RatingAPI` repository with its `beatleader-analyzer` submodule and parser submodule, then overlays the included small `Program.cs` security gate. The service requires .NET 10 and network access to BeatSaver to download map archives. Configure HTTPS at your container host/reverse proxy before using it outside a private test environment.

**Important:** the Docker build fetches the upstream repository at build time. For reproducible production builds, pin the `git clone` to a reviewed commit SHA before deployment and review upstream changes before rebuilding.

## 2. Configure the SnoreSaber website

Set these environment variables in the website's deployment environment:

- `BEATLEADER_RATING_API_URL` — base URL of the deployed RatingAPI, with no trailing slash (for example `https://your-private-rating-service.example`)
- `BEATLEADER_RATING_API_TOKEN` — exactly the same long random secret as `AutorankerApiKey` on the worker

Redeploy the website after setting the variables. No production deployment is performed by this source package.

## 3. Use it

1. Sign into SnoreSaber with an administrator account.
2. Open **Admin → Autoranker**.
3. Paste a BeatSaver map URL/key and choose **Analyze map**.
4. The website calls `/ppai2/link/{characteristic}/{difficultyRank}?link=https://beatsaver.com/maps/{key}` for each published difficulty and uses the `none` modifier's `star_rating`.
5. Review the calculated stars in the pending queue. Edit if desired, then approve or reject. Only approval publishes the rating and triggers SnoreSaber's existing affected-player PP recalculation.

If the service is not configured or any difficulty fails analysis, the preview request returns an explicit error and creates no new proposals. This avoids silently substituting manual or fabricated values.

## API details and caveats

- The web server enforces existing SnoreSaber admin permissions before it contacts the analyzer.
- The companion service additionally requires the shared-secret header for `/ppai2/link/*` requests.
- BeatLeader's model is designed for BeatLeader ratings; this integration imports its calculated star estimate as a **proposal**, not as an automatically approved SnoreSaber rating. Admin review remains mandatory.
- Analyzer results may differ by characteristic/difficulty and depend on the RatingAPI version/model. The source includes BeatLeader's license and model files from the provided archive; the upstream analyzer/parser submodules are fetched by Docker during build.
