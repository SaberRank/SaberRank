import { neon } from '@neondatabase/serverless';

const sql = neon(process.env.DATABASE_URL);
const keys = ['25198', '4fdd2', '52dfb', '4e692', '4d977', '51e10'];

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

console.log('Reset SnoreSaber rankings for curated maps.');
for (const row of rows) console.log(`  ${row.bsid}: ${row.difficulties} difficulties reset`);
