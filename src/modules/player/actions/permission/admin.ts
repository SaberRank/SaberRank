import { createServerFn } from '@tanstack/react-start';

import { localApiAction } from '@/shared/api/local-action.server';

const getPermissionsListFn = createServerFn({ method: 'GET' }).handler(() => localApiAction<{ name: string; value: number }[]>('/admin/permissions'));

export async function getPermissionsList() {
   return getPermissionsListFn();
}
