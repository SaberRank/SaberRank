import { useEffect, useState } from 'react';
import { CalendarClock } from 'lucide-react';

type SeasonStatus = { seasonKey: string; nextReset: string; secondsUntilReset: number };

export function SeasonalCountdown() {
   const [status, setStatus] = useState<SeasonStatus | null>(null);
   const [secondsLeft, setSecondsLeft] = useState(0);
   useEffect(() => {
      let active = true;
      fetch('/api/v2/seasons/status', { cache: 'no-store' }).then((response) => response.ok ? response.json() : null).then((data) => {
         if (!active || !data) return;
         setStatus(data);
         setSecondsLeft(Math.max(0, Number(data.secondsUntilReset || 0)));
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
