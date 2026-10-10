import { neon } from '@neondatabase/serverless';

let sqlClient: ReturnType<typeof neon> | null = null;

export function db() {
  if (!process.env.DATABASE_URL) return null;
  if (!sqlClient) sqlClient = neon(process.env.DATABASE_URL);
  return sqlClient;
}
