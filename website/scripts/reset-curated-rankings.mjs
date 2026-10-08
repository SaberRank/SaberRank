import { neon } from '@neondatabase/serverless';

const sql = neon(process.env.DATABASE_URL);
const keys = (process.env.SNORE_RANKING_RESET_KEYS || '').split(',').map((x) => x.trim()).filter(Boolean);

if (!keys.length) throw new Error('SNORE_RANKING_RESET_KEYS is required; refusing to reset rankings without an explicit map list.');

await sql`
  UPDATE leaderboards
  SET status='UNRANKED', stars=0, ranked_at=NULL
  WHERE map_id IN (SELECT id FROM maps WHERE bsid = ANY(${keys}))
`;

const rows = await sql`
  SELECT m.bsid, COUNT(l.id)::int AS difficulties
  FROM maps m
  LEFT JOIN leaderboards l ON l.map_id=m.id
  WHERE m.bsid = ANY(${keys})
  GROUP BY m.bsid
  ORDER BY m.bsid
`;

console.log('Reset SnoreSaber rankings for explicitly selected maps.');
for (const row of rows) console.log(`  ${row.bsid}: ${row.difficulties} difficulties reset`);
