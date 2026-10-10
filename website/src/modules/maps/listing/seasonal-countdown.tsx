import { useEffect, useState } from 'react';
import { CalendarClock } from 'lucide-react';

type SeasonStatus = { seasonKey: string; nextReset: string; secondsUntilReset: number };

function fallbackSeasonStatus(now = new Date()): SeasonStatus {
   const year = now.getUTCFullYear();
   const quarterMonth = Math.floor(now.getUTCMonth() / 3) * 3;
   const nextMonth = quarterMonth + 3;
   const reset = new Date(Date.UTC(year + Math.floor(nextMonth / 12), nextMonth % 12, 1));
   const seasonStart = new Date(Date.UTC(year, quarterMonth, 1));
   const seasonKey = `${seasonStart.getUTCFullYear()}-Q${Math.floor(quarterMonth / 3) + 1}`;
   return { seasonKey, nextReset: reset.toISOString(), secondsUntilReset: Math.max(0, Math.floor((reset.getTime() - now.getTime()) / 1000)) };
}

export function SeasonalCountdown() {
   const [status, setStatus] = useState<SeasonStatus | null>(null);
   const [secondsLeft, setSecondsLeft] = useState(0);
   useEffect(() => {
      let active = true;
      // Render a local UTC-quarter countdown immediately, then prefer the API value.
      // If the endpoint fails, the card still works instead of remaining on "Loading".
      const fallback = fallbackSeasonStatus();
      setStatus(fallback);
      setSecondsLeft(fallback.secondsUntilReset);
      fetch('/api/v2/seasons/status', { cache: 'no-store' }).then(async (response) => {
         if (!response.ok) throw new Error(`Season status returned HTTP ${response.status}`);
         return response.json();
      }).then((data) => {
         if (!active || !data || typeof data.nextReset !== 'string') return;
         const nextReset = new Date(data.nextReset);
         if (!Number.isFinite(nextReset.getTime())) return;
         const secondsUntilReset = Number.isFinite(Number(data.secondsUntilReset))
            ? Math.max(0, Number(data.secondsUntilReset))
            : Math.max(0, Math.floor((nextReset.getTime() - Date.now()) / 1000));
         setStatus({ seasonKey: String(data.seasonKey || fallback.seasonKey), nextReset: nextReset.toISOString(), secondsUntilReset });
         setSecondsLeft(secondsUntilReset);
      }).catch(() => undefined);
      return () => { active = false; };
   }, []);
   useEffect(() => {
      const timer = setInterval(() => setSecondsLeft((value) => Math.max(0, value - 1)), 1000);
      return () => clearInterval(timer);
   }, []);
   const days = Math.floor(secondsLeft / 86400);
   const hours = Math.floor((secondsLeft % 86400) / 3600);
   const minutes = Math.floor((secondsLeft % 3600) / 60);
   const seconds = secondsLeft % 60;
   return <div className="flex items-center gap-3 rounded-lg border bg-background/75 px-4 py-3"><CalendarClock className="text-primary size-5 shrink-0" /><div><p className="text-sm font-semibold">Next season reset</p><p className="text-muted-foreground text-sm tabular-nums">{status ? `${days}d ${hours}h ${minutes}m ${seconds}s` : 'Loading countdown…'}</p><p className="text-muted-foreground text-xs">{status ? `Current season ${status.seasonKey} · ${new Date(status.nextReset).toLocaleString(undefined, { timeZone: 'UTC', timeZoneName: 'short' })}` : 'Resets on Jan 1, Apr 1, Jul 1, and Oct 1 (UTC)'}</p></div></div>;
}
