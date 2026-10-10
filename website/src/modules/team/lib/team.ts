import { createServerFn } from '@tanstack/react-start';

import { db } from '../../../../server/utils/db';
import { localApiPageData } from '@/shared/api/local-action.server';

export type TeamMember = {
   id: string;
   name: string;
   role: string;
   avatar: string;
   vanity: string | null;
};

export type TeamData = { members: TeamMember[]; viewerIsAdmin: boolean };

const STAFF_MASK = 1 | 2 | 4 | 8 | 16 | 256 | 1024 | 2048 | 4096 | 8192 | 32768;

function fallbackRole(permissions: number) {
   const roles: [number, string][] = [
      [16, 'Administrator'],
      [8, 'NAT'],
      [4, 'QAT Lead'],
      [2, 'QAT'],
      [32768, 'Tournament Organizer'],
      [256, 'Developer'],
      [2048, 'CCT Lead'],
      [1024, 'Content Creation Team'],
      [4096, 'CAT'],
      [8192, 'RTR'],
      [1, 'Replay Team']
   ];

   return roles.find(([bit]) => (permissions & bit) !== 0)?.[1] ?? 'SnoreSaber Staff';
}

const getTeamData = createServerFn({ method: 'GET' }).handler(async (): Promise<TeamData> => {
   const sql = db();
   const adminCheck = await localApiPageData<unknown[]>('/admin/permissions');
   const viewerIsAdmin = adminCheck.ok;
   if (!sql) return { members: [], viewerIsAdmin };

   const rows: Array<{
      id: string | number;
      vanity: string | null;
      name: string | null;
      avatar: string | null;
      role: string | null;
      permissions: number | null;
   }> = await sql`
      SELECT id, vanity, name, avatar, role, permissions
      FROM players
      WHERE (COALESCE(permissions, 0) & ${STAFF_MASK}) <> 0
         OR NULLIF(TRIM(COALESCE(role, '')), '') IS NOT NULL
      ORDER BY COALESCE(NULLIF(TRIM(role), ''), 'SnoreSaber Staff'), name ASC
   `;

   return {
      viewerIsAdmin,
      members: rows.map((row) => ({
         id: String(row.id),
         vanity: row.vanity == null ? null : String(row.vanity),
         name: String(row.name || row.id),
         role: String(row.role || fallbackRole(Number(row.permissions || 0))),
         avatar: String(row.avatar || '/assets/snoresaber-icon.png')
      }))
   };
});

export async function fetchTeam(): Promise<{ ok: true; value: TeamData }> {
   return { ok: true, value: await getTeamData() };
}
