import { createServerFn } from '@tanstack/react-start';

import { localApiAction } from '@/shared/api/local-action.server';

const removeTeamMemberFn = createServerFn({ method: 'POST' })
   .validator((playerId: string) => playerId)
   .handler(({ data }) =>
      localApiAction<void>(`/admin/team/member/${encodeURIComponent(data)}/remove`, { method: 'POST' })
   );

export async function removeTeamMember(playerId: string) {
   return removeTeamMemberFn({ data: playerId });
}
