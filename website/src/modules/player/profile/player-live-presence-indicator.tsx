'use client';

import type { ReactNode } from 'react';

// SnoreSaber does not currently provide the ScoreSaber/Ludus live service.
// Keep the UI API intact, but make live presence explicitly unavailable so
// opening a profile never triggers requests to /v1/connect or /api/v2/live/ludus/session.

export function PlayerLivePresenceProvider({ children }: { children: ReactNode; enabled?: boolean }) {
   return <>{children}</>;
}

export function useLivePlayersState() {
   return 'unavailable' as const;
}

export function PlayerLivePresenceIndicator(_props: { playerId: string; className?: string; size?: 'default' | 'compact' }) {
   return null;
}

export function PlayerListLivePresenceIndicator(_props: { playerId: string; className?: string }) {
   return null;
}
