import { useCallback, useEffect, useMemo, useState, type FormEvent } from 'react';

import { createFileRoute } from '@tanstack/react-router';
import { CalendarClock, Loader2, Plus, Search, Trash2 } from 'lucide-react';

import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { useAuth } from '@/modules/auth';
import { canUseLivePlatform } from '@/modules/live/lib/permissions';
import { SetPageBackground } from '@/shell/background/page-background-provider';
import { buildNoindexHead } from '@/shared/seo/metadata';

type SeasonStatus = { seasonKey: string; startsAt: string; endsAt: string; nextReset: string; secondsUntilReset: number; nextSeasonMapCount?: number };
type SeasonMap = { leaderboardId: number; stars: number; status?: string; hash: string; beatSaverId: string | null; songName: string; songSubName: string; songAuthorName: string; difficulty: number; gameMode: string; rawDifficulty: string };

export const Route = createFileRoute('/live/')({
   head: () => buildNoindexHead('Next Season', 'Curate the next SnoreSaber seasonal ranked map pool', '/live'),
   component: NextSeasonRoute
});

function NextSeasonRoute() {
   const { user } = useAuth();
   const canEdit = canUseLivePlatform(user?.permissions);
   const [status, setStatus] = useState<SeasonStatus | null>(null);
   const [maps, setMaps] = useState<SeasonMap[]>([]);
   const [candidates, setCandidates] = useState<SeasonMap[]>([]);
   const [search, setSearch] = useState('');
   const [searchInput, setSearchInput] = useState('');
   const [beatSaverLink, setBeatSaverLink] = useState('');
   const [importStars, setImportStars] = useState('5');
   const [importBusy, setImportBusy] = useState(false);
   const [stars, setStars] = useState<Record<number, string>>({});
   const [busy, setBusy] = useState<number | null>(null);
   const [error, setError] = useState('');
   const [secondsLeft, setSecondsLeft] = useState(0);

   const load = useCallback(async (term = '') => {
      setError('');
      try {
         const [statusResponse, mapsResponse] = await Promise.all([
            fetch('/api/v2/seasons/status', { cache: 'no-store' }),
            fetch(`/api/v2/seasons/next-maps${term ? `?search=${encodeURIComponent(term)}` : ''}`, { cache: 'no-store' })
         ]);
         const statusData = await statusResponse.json();
         const mapData = await mapsResponse.json();
         if (!statusResponse.ok) throw new Error(statusData?.message || 'Could not load season status');
         if (!mapsResponse.ok) throw new Error(mapData?.message || 'Could not load season maps');
         setStatus(statusData);
         setSecondsLeft(Number(statusData.secondsUntilReset || 0));
         setMaps(Array.isArray(mapData.maps) ? mapData.maps : []);
         setCandidates(Array.isArray(mapData.candidates) ? mapData.candidates : []);
      } catch (cause) {
         setError(cause instanceof Error ? cause.message : 'Could not load season data');
      }
   }, []);

   useEffect(() => { void load(''); }, [load]);
   useEffect(() => {
      const timer = setInterval(() => setSecondsLeft((value) => Math.max(0, value - 1)), 1000);
      return () => clearInterval(timer);
   }, []);

   const countdown = useMemo(() => {
      const days = Math.floor(secondsLeft / 86400);
      const hours = Math.floor((secondsLeft % 86400) / 3600);
      const minutes = Math.floor((secondsLeft % 3600) / 60);
      const seconds = secondsLeft % 60;
      return `${days}d ${hours}h ${minutes}m ${seconds}s`;
   }, [secondsLeft]);

   async function importBeatSaverMap(event: FormEvent<HTMLFormElement>) {
      event.preventDefault();
      setImportBusy(true);
      setError('');
      try {
         const response = await fetch('/api/v2/seasons/next-maps', {
            method: 'POST', headers: { 'content-type': 'application/json' },
            body: JSON.stringify({ action: 'import', beatSaverLink: beatSaverLink.trim(), stars: Number(importStars) })
         });
         const data = await response.json();
         if (!response.ok) throw new Error(data?.message || data?.error || 'Could not import BeatSaver map');
         setBeatSaverLink('');
         await load(search);
      } catch (cause) {
         setError(cause instanceof Error ? cause.message : 'Could not import BeatSaver map');
      } finally { setImportBusy(false); }
   }

   async function mutate(leaderboardId: number, action: 'add' | 'remove', starsValue?: number) {
      setBusy(leaderboardId);
      setError('');
      try {
         const response = await fetch('/api/v2/seasons/next-maps', {
            method: 'POST', headers: { 'content-type': 'application/json' },
            body: JSON.stringify({ leaderboardId, action, stars: starsValue })
         });
         const data = await response.json();
         if (!response.ok) throw new Error(data?.message || data?.error || 'Could not update next season maps');
         await load(search);
      } catch (cause) {
         setError(cause instanceof Error ? cause.message : 'Could not update next season maps');
      } finally { setBusy(null); }
   }

   const nextIds = new Set(maps.map((map) => map.leaderboardId));

   return (
      <>
         <SetPageBackground src="/images/banner.jpg" />
         <div className="app-container relative z-10 flex min-h-dvh flex-1 flex-col gap-5 p-4 pt-10 pb-28 md:p-8">
            <header className="flex flex-col gap-3 rounded-xl border bg-background/80 p-5 shadow-sm sm:flex-row sm:items-center sm:justify-between">
               <div>
                  <p className="text-muted-foreground text-xs font-semibold tracking-[0.18em] uppercase">SnoreSaber seasons</p>
                  <h1 className="mt-1 text-2xl font-semibold">Next Season</h1>
                  <p className="text-muted-foreground mt-1 text-sm">Choose and star the ranked maps that will go live at the next calendar-quarter reset.</p>
               </div>
               <div className="min-w-56 rounded-lg border bg-background p-3">
                  <div className="text-muted-foreground flex items-center gap-2 text-xs"><CalendarClock className="size-4" /> Next reset (UTC)</div>
                  <div className="mt-1 font-mono text-lg font-semibold tabular-nums">{countdown}</div>
                  <div className="text-muted-foreground text-xs">Current season: {status?.seasonKey ?? 'Loading…'}</div>
               </div>
            </header>

            {!user ? <p className="rounded-lg border p-4">Sign in with a Ranking Team account to curate the next season.</p> : !canEdit ? <p className="rounded-lg border p-4">Next Season is available to Ranking Team members only.</p> : (
               <>
                  {error && <p role="alert" className="rounded-md border border-destructive/50 bg-destructive/5 p-3 text-sm text-destructive">{error}</p>}
                  <section className="rounded-xl border bg-background/80 p-4">
                     <h2 className="font-semibold">Maps queued for next season ({maps.length})</h2>
                     <p className="text-muted-foreground mb-3 text-sm">These replace the current ranked map pool at the next quarter boundary. Star values become active when the season rolls over.</p>
                     {maps.length === 0 ? <p className="text-muted-foreground py-4 text-sm">No maps selected yet. Search for a map below and add its difficulty.</p> : (
                        <div className="flex flex-col divide-y">
                           {maps.map((map) => <div key={map.leaderboardId} className="flex items-center gap-3 py-3">
                              <div className="min-w-0 flex-1"><p className="truncate font-medium">{map.songName}{map.songSubName ? ` ${map.songSubName}` : ''}</p><p className="text-muted-foreground text-xs">{map.gameMode} · {map.rawDifficulty} · ID {map.leaderboardId}</p></div>
                              <Input className="w-24" type="number" min="0.01" max="100" step="0.01" aria-label={`stars for ${map.songName}`} value={stars[map.leaderboardId] ?? String(map.stars)} onChange={(event) => setStars((current) => ({ ...current, [map.leaderboardId]: event.target.value }))} />
                              <Button size="sm" disabled={busy === map.leaderboardId} onClick={() => void mutate(map.leaderboardId, 'add', Number(stars[map.leaderboardId] ?? map.stars))}>Save</Button>
                              <Button variant="outline" size="sm" disabled={busy === map.leaderboardId} onClick={() => void mutate(map.leaderboardId, 'remove')}><Trash2 data-icon /> Remove</Button>
                           </div>)}
                        </div>
                     )}
                  </section>
                  <section className="rounded-xl border bg-background/80 p-4">
                     <h2 className="font-semibold">Import a BeatSaver map</h2>
                     <p className="text-muted-foreground mt-1 text-sm">Importing adds its difficulties to the next-season queue only; it does not change the live ranked pool. All difficulties start at the chosen ZZ star value, which you can adjust individually below.</p>
                     <form className="mt-3 flex flex-col gap-2 sm:flex-row" onSubmit={importBeatSaverMap}>
                        <Input value={beatSaverLink} onChange={(event) => setBeatSaverLink(event.target.value)} placeholder="BeatSaver URL or map key" required />
                        <Input className="w-full sm:w-28" type="number" min="0.01" max="100" step="0.01" aria-label="Default star value" value={importStars} onChange={(event) => setImportStars(event.target.value)} required />
                        <Button type="submit" disabled={importBusy || !beatSaverLink.trim()}>{importBusy ? <Loader2 className="animate-spin" /> : <Plus data-icon />} Import map</Button>
                     </form>
                  </section>
                  <section className="rounded-xl border bg-background/80 p-4">
                     <h2 className="font-semibold">Find maps to add</h2>
                     <form className="mt-3 flex gap-2" onSubmit={(event) => { event.preventDefault(); setSearch(searchInput.trim()); void load(searchInput.trim()); }}>
                        <Input value={searchInput} onChange={(event) => setSearchInput(event.target.value)} placeholder="Search song name, map hash, or BeatSaver ID" />
                        <Button type="submit" variant="outline"><Search data-icon /> Search</Button>
                     </form>
                     <div className="mt-3 flex flex-col divide-y">
                        {candidates.filter((map) => !nextIds.has(map.leaderboardId)).map((map) => <div key={map.leaderboardId} className="flex flex-col gap-2 py-3 sm:flex-row sm:items-center">
                           <div className="min-w-0 flex-1"><p className="truncate font-medium">{map.songName}{map.songSubName ? ` ${map.songSubName}` : ''}</p><p className="text-muted-foreground text-xs">{map.gameMode} · {map.rawDifficulty} · {map.status ?? 'UNRANKED'} · ID {map.leaderboardId}</p></div>
                           <div className="flex items-center gap-2"><Input className="w-24" type="number" min="0.01" max="100" step="0.01" aria-label={`Star value for ${map.songName}`} value={stars[map.leaderboardId] ?? (map.stars > 0 ? String(map.stars) : '5')} onChange={(event) => setStars((current) => ({ ...current, [map.leaderboardId]: event.target.value }))} /><Button size="sm" disabled={busy === map.leaderboardId} onClick={() => void mutate(map.leaderboardId, 'add', Number(stars[map.leaderboardId] ?? (map.stars > 0 ? map.stars : 5)))}>{busy === map.leaderboardId ? <Loader2 className="animate-spin" /> : <Plus data-icon />} Add</Button></div>
                        </div>)}
                        {search && candidates.length === 0 && <p className="text-muted-foreground py-4 text-sm">No matching map difficulties found.</p>}
                     </div>
                  </section>
               </>
            )}
         </div>
      </>
   );
}
