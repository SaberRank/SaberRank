import { createServerFn } from '@tanstack/react-start';

import type {
   AdminBadgeControllerGetPlayerBadgesResponse,
   AdminBadgeControllerReplacePlayerBadgesResponse,
   AdminUserControllerGetActiveBanResponse,
   AdminUserControllerUpdatePermissionsResponse
} from '@/shared/api/generated/ApiParams';
import { localApiAction } from '@/shared/api/local-action.server';

type BanPlayerInput = {
   playerId: string;
   reason: string;
   notes?: string;
   earliestAppealDate?: string;
};

const banPlayerFn = createServerFn({ method: 'POST' })
   .validator((data: BanPlayerInput) => data)
   .handler(({ data }) => {
      return localApiAction<void>(`/admin/user/${encodeURIComponent(data.playerId)}/ban`, {
         method: 'POST',
         body: {
            reason: data.reason,
            ...(data.notes && { notes: data.notes }),
            ...(data.earliestAppealDate && { earliestAppealDate: data.earliestAppealDate })
         }
      });
   });

const unbanPlayerFn = createServerFn({ method: 'POST' })
   .validator((playerId: string) => playerId)
   .handler(({ data }) => localApiAction<void>(`/admin/user/${encodeURIComponent(data)}/unban`, { method: 'POST' }));

const unsilencePlayerFn = createServerFn({ method: 'POST' })
   .validator((playerId: string) => playerId)
   .handler(({ data }) => localApiAction<void>(`/admin/user/${encodeURIComponent(data)}/unsilence`, { method: 'POST' }));

const adminResetCountryFn = createServerFn({ method: 'POST' })
   .validator((data: { playerId: string; country: string }) => data)
   .handler(({ data }) =>
      localApiAction<void>(`/admin/user/${encodeURIComponent(data.playerId)}/reset-country`, { method: 'POST', body: { country: data.country } })
   );

const updateRoleTextFn = createServerFn({ method: 'POST' })
   .validator((data: { playerId: string; roleText: string }) => data)
   .handler(({ data }) =>
      localApiAction<void>(`/admin/user/${encodeURIComponent(data.playerId)}/role-text`, { method: 'POST', body: { roleText: data.roleText } })
   );

const updatePermissionsFn = createServerFn({ method: 'POST' })
   .validator((data: { playerId: string; add?: string[]; remove?: string[] }) => data)
   .handler(({ data }) =>
      localApiAction<AdminUserControllerUpdatePermissionsResponse>(`/admin/user/${encodeURIComponent(data.playerId)}/permissions`, {
         method: 'POST',
         body: { add: data.add, remove: data.remove }
      })
   );

const getPlayerBadgeAssignmentsFn = createServerFn({ method: 'GET' })
   .validator((playerId: string) => playerId)
   .handler(({ data }) => localApiAction<AdminBadgeControllerGetPlayerBadgesResponse>(`/admin/badges/player/${encodeURIComponent(data)}`));

const replacePlayerBadgeAssignmentsFn = createServerFn({ method: 'POST' })
   .validator((data: { playerId: string; badges: { badgeId: number; descriptionOverride: string | null }[] }) => data)
   .handler(({ data }) =>
      localApiAction<AdminBadgeControllerReplacePlayerBadgesResponse>(`/admin/badges/player/${encodeURIComponent(data.playerId)}`, {
         method: 'PUT',
         body: { badges: data.badges }
      })
   );

const getActiveBanFn = createServerFn({ method: 'GET' })
   .validator((playerId: string) => playerId)
   .handler(({ data }) => localApiAction<AdminUserControllerGetActiveBanResponse | null>(`/admin/user/${encodeURIComponent(data)}/ban`));

const mergePlayerFn = createServerFn({ method: 'POST' })
   .validator((data: { targetPlayerId: string; sourcePlayerId: string; reason: string }) => data)
   .handler(({ data }) =>
      localApiAction<void>(`/admin/user/${encodeURIComponent(data.targetPlayerId)}/merge`, {
         method: 'POST',
         body: { sourcePlayerId: data.sourcePlayerId, reason: data.reason }
      })
   );

export async function banPlayer(input: BanPlayerInput) {
   return banPlayerFn({ data: input });
}

export async function unbanPlayer(playerId: string) {
   return unbanPlayerFn({ data: playerId });
}

export async function unsilencePlayer(playerId: string) {
   return unsilencePlayerFn({ data: playerId });
}

export async function adminResetCountry(playerId: string, country: string) {
   return adminResetCountryFn({ data: { playerId, country } });
}

export async function updateRoleText(playerId: string, roleText: string) {
   return updateRoleTextFn({ data: { playerId, roleText } });
}

export async function updatePermissions(playerId: string, add?: string[], remove?: string[]) {
   return updatePermissionsFn({ data: { playerId, add, remove } });
}

export async function getPlayerBadgeAssignments(playerId: string) {
   return getPlayerBadgeAssignmentsFn({ data: playerId });
}

export async function replacePlayerBadgeAssignments(playerId: string, badges: { badgeId: number; descriptionOverride: string | null }[]) {
   return replacePlayerBadgeAssignmentsFn({ data: { playerId, badges } });
}

export async function getActiveBan(playerId: string) {
   return getActiveBanFn({ data: playerId });
}

export async function mergePlayer(targetPlayerId: string, sourcePlayerId: string, reason: string) {
   return mergePlayerFn({ data: { targetPlayerId, sourcePlayerId, reason } });
}
