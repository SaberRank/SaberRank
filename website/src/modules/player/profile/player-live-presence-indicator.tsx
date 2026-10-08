'use client';

import type { ReactNode } from 'react';
import { createContext, useContext } from 'react';

interface PlayerLivePresenceIndicatorProps {
   playerId: string;
   className?: string;
   size?: 'default' | 'compact';
}

/**
 * SnoreSaber currently does not expose the ScoreSaber/Ludus live-presence service.
 * Keep the profile/ranking UI independent from that optional feature so a missing
 * Ludus endpoint can never make an otherwise normal player page fail.
 */
const PlayerLivePresenceContext = createContext(false);

export function PlayerLivePresenceProvider({ children }: { children: ReactNode; enabled?: boolean }) {
   return <PlayerLivePresenceContext.Provider value={false}>{children}</PlayerLivePresenceContext.Provider>;
}

export function useLivePlayersState() {
   // Live presence is intentionally unavailable until SnoreSaber implements its
   // own live service. Consumers use this state to hide live-only UI.
   const available = useContext(PlayerLivePresenceContext);
   return available ? 'available' : 'unavailable';
}

export function PlayerLivePresenceIndicator(_props: PlayerLivePresenceIndicatorProps) {
   return null;
}

export function PlayerListLivePresenceIndicator(_props: PlayerLivePresenceIndicatorProps) {
   return null;
}
