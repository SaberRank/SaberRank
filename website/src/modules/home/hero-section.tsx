import { useEffect, useRef, useState } from 'react';

import { getRouteApi } from '@tanstack/react-router';
import { Download } from 'lucide-react';
import { useTranslations } from 'use-intl';

import { Button } from '@/components/ui/button';
import { useAuth } from '@/modules/auth';
import { Image } from '@/shared/components/image';

const loginRoute = getRouteApi('/login');
const playerRoute = getRouteApi('/u/$playerId');

const MS_PER_DAY = 24 * 60 * 60 * 1000;
const STAT_FEED_EVENT_MS = 1600;
const STAT_FEED_LIMIT = 3;
const STAT_FEED_MIN_INTERVAL_MS = 2600;
const STAT_FEED_MAX_INTERVAL_MS = 7200;
const STAT_FEED_MIN_RAW_INTERVAL_MS = 2_800;
const STAT_FEED_MAX_RAW_INTERVAL_MS = 900_000;

type HomeStatKey = 'players' | 'scores' | 'leaderboards';
type HomeStat = {
   key: HomeStatKey;
   value: string;
   label: string;
   avgGrowthPerDay: number;
};

const statGrowthPerDay = { players: 500, scores: 30000, leaderboards: 100 };

export function HeroSection() {
   const t = useTranslations('home');
   const { user } = useAuth();
   const stats: HomeStat[] = [
      { key: 'players', value: t('stats.playersValue'), label: t('stats.playersLabel'), avgGrowthPerDay: statGrowthPerDay.players },
      { key: 'scores', value: t('stats.scoresValue'), label: t('stats.scoresLabel'), avgGrowthPerDay: statGrowthPerDay.scores },
      { key: 'leaderboards', value: t('stats.leaderboardsValue'), label: t('stats.leaderboardsLabel'), avgGrowthPerDay: statGrowthPerDay.leaderboards }
   ];

   return (
      <section className="relative z-10 overflow-hidden px-4 pt-16 pb-12 sm:px-6 sm:pt-20 sm:pb-16 lg:px-10 lg:pb-14">
         <div className="pointer-events-none absolute inset-x-0 top-0 -z-10 mx-auto h-[30rem] max-w-[1180px] overflow-hidden rounded-b-3xl opacity-90">
            <img src="/assets/snoresaber-hero-art@2x.png" alt="" className="absolute inset-0 h-full w-full object-cover object-center" />
            <div className="absolute inset-0 bg-[linear-gradient(90deg,hsl(var(--background))_0%,hsl(var(--background)/.94)_28%,hsl(var(--background)/.72)_58%,hsl(var(--background)/.30)_100%)]" />
            <div className="absolute inset-0 bg-[linear-gradient(180deg,hsl(var(--background)/.05),hsl(var(--background))_96%)]" />
         </div>

         <div className="relative mx-auto flex max-w-3xl flex-col items-center gap-6 text-center">
            <div className="flex flex-col gap-4">
               <h1 className="text-4xl leading-tight font-medium sm:text-5xl">SnoreSaber</h1>
               <p className="text-muted-foreground mx-auto max-w-2xl text-base leading-relaxed sm:text-[16.5px]">
                  Your independent Beat Saber leaderboard for scores, rankings, maps, and player progress.
               </p>
            </div>

            <div className="grid w-full max-w-xl grid-cols-3 gap-3 sm:gap-8">
               {stats.map((stat) => <HeroStat key={stat.key} stat={stat} />)}
            </div>

            <div className="flex flex-col items-stretch justify-center gap-3 sm:flex-row sm:items-center">
               <Button asChild size="lg">
                  <a href="/rankings"><Download data-icon />View Rankings</a>
               </Button>
               <Button asChild variant="menu-filled" size="lg" className="cursor-pointer border-white/20">
                  <a href="/maps">Explore Maps</a>
               </Button>
               {!user && <Button asChild variant="ghost" size="lg"><loginRoute.Link search={{}}>Sign in with Steam</loginRoute.Link></Button>}
               {user && <Button asChild variant="ghost" size="lg"><playerRoute.Link params={{ playerId: user.vanity || user.id }}>View Profile</playerRoute.Link></Button>}
            </div>
         </div>
      </section>
   );
}

function HeroStat({ stat }: { stat: HomeStat }) {
   const feed = useStatFeed(stat.avgGrowthPerDay);
   return (
      <div className="flex min-w-0 flex-col items-center gap-0.5">
         <div className="relative flex min-w-0 items-center justify-center">
            <span className="text-primary text-2xl leading-tight font-bold sm:text-[34px]">{stat.value}</span>
            <span className="home-stat-feed absolute top-1/2 left-full ml-1.5 h-5 w-5 -translate-y-1/2 overflow-hidden text-left sm:h-6 sm:w-6" aria-hidden>
               {feed.map((event) => <span key={event.id} className="home-stat-feed-tick text-primary/75 absolute top-1/2 left-0 text-[10px] font-bold tabular-nums sm:text-xs">+1</span>)}
            </span>
         </div>
         <div className="text-muted-foreground text-center text-xs sm:text-sm">{stat.label}</div>
      </div>
   );
}

function useStatFeed(avgGrowthPerDay: number) {
   const [feed, setFeed] = useState<{ id: number }[]>([]);
   const nextEventId = useRef(0);
   useEffect(() => {
      if (window.matchMedia('(prefers-reduced-motion: reduce)').matches) return;
      const removeTimers: number[] = [];
      const intervalMs = getStatFeedInterval(avgGrowthPerDay);
      const pushEvent = () => {
         const event = { id: ++nextEventId.current };
         setFeed((current) => [event, ...current].slice(0, STAT_FEED_LIMIT));
         const timer = window.setTimeout(() => setFeed((current) => current.filter((item) => item.id !== event.id)), STAT_FEED_EVENT_MS);
         removeTimers.push(timer);
      };
      const firstTick = window.setTimeout(pushEvent, intervalMs * 0.45);
      const interval = window.setInterval(pushEvent, intervalMs);
      removeTimers.push(firstTick);
      return () => { window.clearInterval(interval); for (const timer of removeTimers) window.clearTimeout(timer); };
   }, [avgGrowthPerDay]);
   return feed;
}

function getStatFeedInterval(avgGrowthPerDay: number) {
   const rawInterval = MS_PER_DAY / avgGrowthPerDay;
   const minRaw = Math.log10(STAT_FEED_MIN_RAW_INTERVAL_MS);
   const maxRaw = Math.log10(STAT_FEED_MAX_RAW_INTERVAL_MS);
   const rawPosition = (Math.log10(rawInterval) - minRaw) / (maxRaw - minRaw);
   const displayPosition = Math.min(1, Math.max(0, rawPosition));
   return STAT_FEED_MIN_INTERVAL_MS + (STAT_FEED_MAX_INTERVAL_MS - STAT_FEED_MIN_INTERVAL_MS) * displayPosition;
}
