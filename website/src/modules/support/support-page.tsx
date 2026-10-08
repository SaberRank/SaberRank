'use client';

import { getRouteApi } from '@tanstack/react-router';
import { ExternalLink, Heart, LayoutDashboard, Server } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Card } from '@/components/ui/card';
import { BeatSaberPageBackground } from '@/modules/home/beat-saber-background';

const connectionsRoute = getRouteApi('/settings/connections');

export function SupportPage() {
   return (
      <div className="dark bg-background text-foreground relative flex-1 overflow-hidden">
         <BeatSaberPageBackground />
         <main className="relative z-10 mx-auto flex w-full max-w-4xl flex-col gap-8 px-4 py-12 sm:px-6">
            <section className="flex flex-col items-center gap-3 text-center">
               <Heart className="text-primary size-10" aria-hidden />
               <h1 className="text-3xl font-bold tracking-tight">SnoreSaber Support</h1>
               <p className="text-muted-foreground max-w-2xl text-sm leading-relaxed">SnoreSaber is free. Profile customization, badges, pinned scores, and leaderboard features are available to everyone.</p>
               <div className="flex flex-wrap justify-center gap-2">
                  <Button asChild><a href="https://discord.gg/snoresaber" target="_blank" rel="noreferrer"><ExternalLink data-icon="inline-start" />Discord</a></Button>
                  <Button asChild variant="outline"><connectionsRoute.Link search={{}}>Account connections</connectionsRoute.Link></Button>
               </div>
            </section>
            <div className="grid gap-4 sm:grid-cols-2">
               <Card className="flex items-start gap-4 p-5"><LayoutDashboard className="text-primary size-6 shrink-0" aria-hidden /><div><h2 className="font-semibold">Profile customization</h2><p className="text-muted-foreground mt-1 text-sm">Customize colors, layout, badges, pinned scores, and backgrounds.</p></div></Card>
               <Card className="flex items-start gap-4 p-5"><Server className="text-primary size-6 shrink-0" aria-hidden /><div><h2 className="font-semibold">Community infrastructure</h2><p className="text-muted-foreground mt-1 text-sm">Your scores and rankings are powered by SnoreSaber.</p></div></Card>
            </div>
         </main>
      </div>
   );
}
