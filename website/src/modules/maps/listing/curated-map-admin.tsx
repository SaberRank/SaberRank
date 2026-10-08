'use client';

import { useMemo, useState } from 'react';

import { Loader2, Settings2 } from 'lucide-react';
import { toast } from 'sonner';

import { Button } from '@/components/ui/button';
import { Dialog, DialogContent, DialogDescription, DialogFooter, DialogHeader, DialogTitle } from '@/components/ui/dialog';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { ScrollArea } from '@/components/ui/scroll-area';
import { useAuth } from '@/modules/auth';
import Permissions from '@/shared/permissions';

type Diff = {
   id: number;
   difficulty: number;
   gameMode: string;
   rawDifficulty: string;
   stars: number;
};

const DIFFICULTY_ORDER: Record<number, number> = { 1: 1, 3: 2, 5: 3, 7: 4, 9: 5 };

function difficultyLabel(value: number) {
   return value === 1 ? 'Easy' : value === 3 ? 'Normal' : value === 5 ? 'Hard' : value === 7 ? 'Expert' : 'Expert+' ;
}

export function CuratedMapAdmin() {
   const { user } = useAuth();
   const canAdmin =
      Permissions.checkPermissionNumber(user?.permissions ?? 0, Permissions.security.ADMIN) ||
      Permissions.checkPermissionNumber(user?.permissions ?? 0, Permissions.security.PANDA);

   const [open, setOpen] = useState(false);
   const [link, setLink] = useState('');
   const [mapName, setMapName] = useState('');
   const [diffs, setDiffs] = useState<Diff[]>([]);
   const [loading, setLoading] = useState(false);
   const [saving, setSaving] = useState(false);

   const orderedDiffs = useMemo(
      () => [...diffs].sort((a, b) => (DIFFICULTY_ORDER[a.difficulty] ?? 99) - (DIFFICULTY_ORDER[b.difficulty] ?? 99) || a.gameMode.localeCompare(b.gameMode)),
      [diffs]
   );

   if (!canAdmin) return null;

   async function loadMap() {
      const value = link.trim();
      if (!value) return;
      setLoading(true);
      try {
         const response = await fetch('/api/v2/admin/maps/rank-from-beatsaver', {
            method: 'POST',
            headers: { 'content-type': 'application/json' },
            body: JSON.stringify({ beatSaverLink: value, rankings: [], preview: true })
         });
         const data = await response.json();
         if (!response.ok) throw new Error(data?.message || 'Failed to load BeatSaver map');

         const loaded = Array.isArray(data.leaderboards) ? data.leaderboards : [];
         setMapName(String(data.map?.songName || data.map?.bsid || 'BeatSaver map'));
         setDiffs(loaded.map((d: any) => ({
            id: Number(d.id),
            difficulty: Number(d.difficulty),
            gameMode: String(d.gameMode || 'Standard'),
            rawDifficulty: String(d.rawDifficulty || ''),
            stars: Number(d.stars || 0)
         })));
         toast.success('Map loaded');
      } catch (error) {
         toast.error(error instanceof Error ? error.message : 'Failed to load map');
      } finally {
         setLoading(false);
      }
   }

   async function saveRankings() {
      if (!link.trim() || diffs.length === 0) return;
      setSaving(true);
      try {
         const response = await fetch('/api/v2/admin/maps/rank-from-beatsaver', {
            method: 'POST',
            headers: { 'content-type': 'application/json' },
            body: JSON.stringify({
               beatSaverLink: link.trim(),
               rankings: diffs.map((diff) => ({
                  leaderboardId: diff.id,
                  difficulty: diff.difficulty,
                  gameMode: diff.gameMode,
                  stars: Number(diff.stars || 0)
               }))
            })
         });
         const data = await response.json();
         if (!response.ok) throw new Error(data?.message || 'Failed to save rankings');
         setDiffs((data.leaderboards || []).map((d: any) => ({
            id: Number(d.id),
            difficulty: Number(d.difficulty),
            gameMode: String(d.gameMode || 'Standard'),
            rawDifficulty: String(d.rawDifficulty || ''),
            stars: Number(d.stars || 0)
         })));
         toast.success('SnoreSaber rankings saved');
      } catch (error) {
         toast.error(error instanceof Error ? error.message : 'Failed to save rankings');
      } finally {
         setSaving(false);
      }
   }

   function reset() {
      setLink('');
      setMapName('');
      setDiffs([]);
   }

   return (
      <>
         <Button type="button" variant="outline" size="sm" onClick={() => setOpen(true)}>
            <Settings2 data-icon="inline-start" />
            Map Admin
         </Button>

         <Dialog open={open} onOpenChange={(value) => { setOpen(value); if (!value) reset(); }}>
            <DialogContent className="max-w-lg">
               <DialogHeader>
                  <DialogTitle>SnoreSaber Map Ranking</DialogTitle>
                  <DialogDescription>
                     Paste one of the six curated BeatSaver links, load its difficulties, then enter your own SnoreSaber star value for each difficulty.
                  </DialogDescription>
               </DialogHeader>

               <div className="flex flex-col gap-3">
                  <div className="flex gap-2">
                     <div className="min-w-0 flex-1">
                        <Label htmlFor="beatsaver-admin-link">BeatSaver link</Label>
                        <Input
                           id="beatsaver-admin-link"
                           className="mt-1.5"
                           placeholder="https://beatsaver.com/maps/4fdd2"
                           value={link}
                           onChange={(event) => setLink(event.target.value)}
                           disabled={loading || saving}
                        />
                     </div>
                     <Button type="button" className="mt-6" onClick={loadMap} disabled={loading || saving || !link.trim()}>
                        {loading ? <Loader2 className="animate-spin" /> : 'Load'}
                     </Button>
                  </div>

                  {mapName && (
                     <div className="rounded-md border px-3 py-2">
                        <p className="font-medium">{mapName}</p>
                        <p className="text-muted-foreground text-xs">Blank or 0 stars = unranked</p>
                     </div>
                  )}

                  {orderedDiffs.length > 0 && (
                     <ScrollArea className="max-h-72 pr-3">
                        <div className="flex flex-col gap-2">
                           {orderedDiffs.map((diff) => (
                              <div key={`${diff.id}-${diff.gameMode}`} className="grid grid-cols-[1fr_7rem] items-center gap-3 rounded-md border px-3 py-2">
                                 <div className="min-w-0">
                                    <p className="text-sm font-medium">{difficultyLabel(diff.difficulty)}</p>
                                    <p className="text-muted-foreground text-xs">{diff.gameMode}</p>
                                 </div>
                                 <Input
                                    type="number"
                                    min={0}
                                    max={100}
                                    step="0.01"
                                    value={diff.stars === 0 ? '' : diff.stars}
                                    placeholder="Stars"
                                    onChange={(event) => {
                                       const next = event.target.value === '' ? 0 : Number(event.target.value);
                                       setDiffs((current) => current.map((item) => item.id === diff.id ? { ...item, stars: Number.isFinite(next) ? next : 0 } : item));
                                    }}
                                 />
                              </div>
                           ))}
                        </div>
                     </ScrollArea>
                  )}
               </div>

               <DialogFooter>
                  <Button type="button" variant="outline" onClick={() => setOpen(false)} disabled={saving}>Cancel</Button>
                  <Button type="button" onClick={saveRankings} disabled={saving || loading || diffs.length === 0}>
                     {saving && <Loader2 data-icon="inline-start" className="animate-spin" />}
                     Save Rankings
                  </Button>
               </DialogFooter>
            </DialogContent>
         </Dialog>
      </>
   );
}
