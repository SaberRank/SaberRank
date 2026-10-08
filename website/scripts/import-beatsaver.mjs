#!/usr/bin/env node
/**
 * SnoreSaber BeatSaver catalog importer
 *
 * Full import:
 *   node scripts/import-beatsaver.mjs --full
 *
 * Incremental import:
 *   node scripts/import-beatsaver.mjs --new
 *
 * Requires Node 24+ and DATABASE_URL in the environment (or .env when
 * invoked with: node --env-file=.env scripts/import-beatsaver.mjs ...).
 *
 * This intentionally runs outside Vercel. It is designed to populate Neon
 * with the complete BeatSaver catalog once, then only import new/changed
 * maps when --new is run.
 */

import { neon } from '@neondatabase/serverless';

const API = process.env.BEATSAVER_API_URL || 'https://api.beatsaver.com';
const DATABASE_URL = process.env.DATABASE_URL;
const PAGE_SIZE = 100;
const DEFAULT_DELAY_MS = Number(process.env.BEATSAVER_DELAY_MS || 250);
const LOG_EVERY = Number(process.env.BEATSAVER_LOG_EVERY || 10);

if (!DATABASE_URL) {
  console.error('Missing DATABASE_URL. Put it in .env or set it in your shell.');
  process.exit(1);
}

const sql = neon(DATABASE_URL);

const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

function dateOrNull(value) {
  if (!value) return null;
  const d = new Date(value);
  return Number.isNaN(d.getTime()) ? null : d;
}

function difficultyValue(value) {
  const key = String(value || '').toLowerCase();
  const map = {
    easy: 1,
    normal: 3,
    hard: 5,
    expert: 7,
    expertplus: 9,
    'expert+': 9,
  };
  return map[key] || Number(value) || 0;
}

function gameMode(value) {
  const key = String(value || '').toLowerCase();
  if (key.includes('one saber') || key === 'onesaber') return 'OneSaber';
  if (key.includes('no arrows') || key === 'noarrows') return 'NoArrows';
  if (key.includes('360')) return '360Degree';
  if (key.includes('90')) return '90Degree';
  if (key.includes('lawless')) return 'Lawless';
  if (key.includes('lightshow')) return 'Lightshow';
  if (key.includes('standard')) return 'Standard';
  return String(value || 'Standard');
}

function mapStatus(map) {
  if (map.ranked) return 'RANKED';
  if (map.qualified) return 'QUALIFIED';
  return 'UNRANKED';
}

async function ensureSyncState() {
  await sql`
    CREATE TABLE IF NOT EXISTS beatsaver_sync_state (
      id INTEGER PRIMARY KEY CHECK (id=1),
      last_sync_at TIMESTAMPTZ NOT NULL DEFAULT now(),
      maps_synced INTEGER NOT NULL DEFAULT 0
    )`;
  await sql`ALTER TABLE beatsaver_sync_state ADD COLUMN IF NOT EXISTS newest_uploaded_at TIMESTAMPTZ`;
  await sql`ALTER TABLE beatsaver_sync_state ADD COLUMN IF NOT EXISTS last_map_hash TEXT`;
  const rows = await sql`SELECT * FROM beatsaver_sync_state WHERE id=1 LIMIT 1`;
  if (!rows.length) {
    await sql`
      INSERT INTO beatsaver_sync_state
        (id, last_sync_at, maps_synced, newest_uploaded_at, last_map_hash)
      VALUES (1, now(), 0, NULL, NULL)
    `;
  }
}

async function fetchPage(before = null) {
  const params = new URLSearchParams({ pageSize: String(PAGE_SIZE) });
  if (before) params.set('before', before);

  for (let attempt = 1; attempt <= 6; attempt++) {
    try {
      const response = await fetch(`${API}/maps/latest?${params}`, {
        headers: {
          accept: 'application/json',
          'user-agent': 'SnoreSaber/3.0 BeatSaver catalog importer',
        },
      });

      if (response.ok) return response.json();

      if (response.status === 429 || response.status >= 500) {
        const retryAfter = Number(response.headers.get('retry-after') || 0);
        const wait = retryAfter > 0 ? retryAfter * 1000 : Math.min(30000, 1000 * 2 ** (attempt - 1));
        console.warn(`BeatSaver HTTP ${response.status}; retrying in ${Math.round(wait / 1000)}s (${attempt}/6)`);
        await sleep(wait);
        continue;
      }

      const body = await response.text().catch(() => '');
      throw new Error(`BeatSaver HTTP ${response.status}: ${body.slice(0, 300)}`);
    } catch (error) {
      if (attempt === 6) throw error;
      const wait = Math.min(30000, 1000 * 2 ** (attempt - 1));
      console.warn(`BeatSaver request failed; retrying in ${Math.round(wait / 1000)}s (${attempt}/6): ${error.message}`);
      await sleep(wait);
    }
  }
}

async function upsertMap(map) {
  const version = Array.isArray(map?.versions)
    ? (map.versions.find((v) => String(v?.state || '').toLowerCase() === 'published') || map.versions[0])
    : null;

  const hash = String(version?.hash || '').trim();
  const bsid = String(map?.id || version?.key || '').trim();
  if (!hash || !bsid) return { skipped: true, hash: null, uploaded: null };

  const metadata = map.metadata || {};
  const uploaded = dateOrNull(map.uploaded);
  const coverUrl = String(
    version?.coverURL ||
    map.coverURL ||
    `https://eu.cdn.beatsaver.com/${hash}.jpg`
  ).trim();

  const inserted = await sql`
    INSERT INTO maps
      (hash, bsid, song_name, song_sub_name, song_author_name, level_author_name,
       bpm, cover_url, verified, created_at)
    VALUES
      (${hash}, ${bsid},
       ${String(metadata.songName || map.name || 'Unknown')},
       ${String(metadata.songSubName || '')},
       ${String(metadata.songAuthorName || '')},
       ${String(metadata.levelAuthorName || map.uploader?.name || '')},
       ${Number(metadata.bpm || 0)},
       ${coverUrl},
       ${Boolean(map.verified || map.uploader?.verifiedMapper)},
       COALESCE(${uploaded ? uploaded.toISOString() : null}::timestamptz, now()))
    ON CONFLICT (hash) DO UPDATE SET
      bsid=EXCLUDED.bsid,
      song_name=EXCLUDED.song_name,
      song_sub_name=EXCLUDED.song_sub_name,
      song_author_name=EXCLUDED.song_author_name,
      level_author_name=EXCLUDED.level_author_name,
      bpm=EXCLUDED.bpm,
      cover_url=EXCLUDED.cover_url,
      verified=EXCLUDED.verified
    RETURNING id
  `;

  const mapId = Number(inserted[0]?.id);
  if (!mapId) return { skipped: true, hash, uploaded };

  const status = mapStatus(map);
  const diffs = Array.isArray(version?.diffs) ? version.diffs : [];

  for (const diff of diffs) {
    const difficulty = difficultyValue(diff?.difficulty);
    if (!difficulty) continue;

    const mode = gameMode(diff?.characteristic);
    const rawDifficulty = String(diff?.difficulty || 'ExpertPlus');
    const stars = Number(diff?.stars ?? diff?.starsBeatLeader ?? 0);
    const maxScore = Number(diff?.maxScore || 1000000);

    await sql`
      INSERT INTO leaderboards
        (map_id, difficulty, game_mode, raw_difficulty, max_score, stars, status, ranked_at)
      VALUES
        (${mapId}, ${difficulty}, ${mode}, ${rawDifficulty},
         ${Number.isFinite(maxScore) ? maxScore : 1000000},
         ${Number.isFinite(stars) ? stars : 0},
         ${status},
         ${status === 'RANKED' ? (uploaded ? uploaded.toISOString() : new Date().toISOString()) : null})
      ON CONFLICT (map_id, difficulty, game_mode) DO UPDATE SET
        raw_difficulty=EXCLUDED.raw_difficulty,
        max_score=EXCLUDED.max_score,
        stars=EXCLUDED.stars,
        status=EXCLUDED.status,
        ranked_at=EXCLUDED.ranked_at
    `;
  }

  return { skipped: false, hash, uploaded };
}

async function fullImport() {
  console.log('Starting FULL BeatSaver import. This will continue until the oldest page is reached.');
  let before = null;
  let pages = 0;
  let maps = 0;
  let skipped = 0;
  let oldest = null;
  let newest = null;

  while (true) {
    const payload = await fetchPage(before);
    const docs = Array.isArray(payload?.docs) ? payload.docs : [];

    if (!docs.length) break;

    pages++;
    for (const map of docs) {
      const result = await upsertMap(map);
      if (result.skipped) skipped++;
      else maps++;

      if (result.uploaded) {
        if (!oldest || result.uploaded < oldest) oldest = result.uploaded;
        if (!newest || result.uploaded > newest) newest = result.uploaded;
      }
    }

    const oldestUploaded = dateOrNull(docs[docs.length - 1]?.uploaded);
    if (!oldestUploaded) break;

    const nextBefore = oldestUploaded.toISOString();
    if (nextBefore === before) {
      console.warn('BeatSaver returned the same pagination cursor twice; stopping safely.');
      break;
    }

    before = nextBefore;

    if (pages % LOG_EVERY === 0) {
      console.log(
        `Imported ${maps.toLocaleString()} maps across ${pages.toLocaleString()} pages. ` +
        `Oldest seen: ${oldest?.toISOString() || 'unknown'}`
      );
    }

    await sleep(DEFAULT_DELAY_MS);

    if (docs.length < PAGE_SIZE) break;
  }

  await sql`
    UPDATE beatsaver_sync_state
    SET last_sync_at=now(),
        maps_synced=${maps},
        newest_uploaded_at=COALESCE(${newest ? newest.toISOString() : null}::timestamptz, newest_uploaded_at),
        last_map_hash=${newest ? String(newest.getTime()) : null}
    WHERE id=1
  `;

  console.log(`FULL import finished: ${maps.toLocaleString()} maps imported/updated, ${skipped.toLocaleString()} skipped, ${pages.toLocaleString()} pages.`);
}

async function incrementalImport() {
  await ensureSyncState();

  const rows = await sql`SELECT newest_uploaded_at FROM beatsaver_sync_state WHERE id=1 LIMIT 1`;
  const cutoff = rows[0]?.newest_uploaded_at ? new Date(rows[0].newest_uploaded_at) : null;

  console.log(
    cutoff
      ? `Checking BeatSaver for maps newer than ${cutoff.toISOString()}`
      : 'No previous cutoff found; this behaves like a full import.'
  );

  let before = null;
  let pages = 0;
  let imported = 0;
  let checked = 0;
  let newest = cutoff;

  while (true) {
    const payload = await fetchPage(before);
    const docs = Array.isArray(payload?.docs) ? payload.docs : [];
    if (!docs.length) break;

    pages++;
    let pageHasNew = false;

    for (const map of docs) {
      const uploaded = dateOrNull(map?.uploaded);
      if (uploaded && (!newest || uploaded > newest)) newest = uploaded;

      // Keep scanning until an entire page is at/before the cutoff. This handles
      // multiple maps sharing the exact same upload timestamp without missing them.
      if (cutoff && uploaded && uploaded < cutoff) {
        checked++;
        continue;
      }

      const result = await upsertMap(map);
      checked++;
      if (!result.skipped) {
        imported++;
        pageHasNew = true;
      }
    }

    const oldestUploaded = dateOrNull(docs[docs.length - 1]?.uploaded);
    if (!oldestUploaded) break;

    if (cutoff && docs.every((doc) => {
      const d = dateOrNull(doc?.uploaded);
      return d && d <= cutoff;
    })) {
      break;
    }

    const nextBefore = oldestUploaded.toISOString();
    if (nextBefore === before) break;
    before = nextBefore;

    if (docs.length < PAGE_SIZE) break;

    if (pages % LOG_EVERY === 0) {
      console.log(`Checked ${checked.toLocaleString()} maps across ${pages.toLocaleString()} pages; imported ${imported.toLocaleString()}.`);
    }

    await sleep(DEFAULT_DELAY_MS);
  }

  await sql`
    UPDATE beatsaver_sync_state
    SET last_sync_at=now(),
        maps_synced=${imported},
        newest_uploaded_at=COALESCE(${newest ? newest.toISOString() : null}::timestamptz, newest_uploaded_at)
    WHERE id=1
  `;

  console.log(`Incremental sync finished: ${imported.toLocaleString()} maps imported/updated.`);
}

async function main() {
  await ensureSyncState();

  const mode = process.argv.includes('--full') ? 'full' : 'new';
  if (mode === 'full') await fullImport();
  else await incrementalImport();
}

main().catch((error) => {
  console.error(error?.stack || error);
  process.exit(1);
});
