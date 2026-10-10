import Permissions from '@/shared/permissions';

const NEXT_SEASON_PERMISSION_MASK = Permissions.security.RT | Permissions.security.RTR | Permissions.security.ADMIN | Permissions.security.PANDA;

export function canUseLivePlatform(permissions: number | undefined) {
   return Permissions.checkPermissionNumber(permissions ?? 0, NEXT_SEASON_PERMISSION_MASK);
}
