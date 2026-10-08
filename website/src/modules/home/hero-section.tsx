import { ArrowRight, Map, Search, Trophy, Users } from 'lucide-react';
import { useTranslations } from 'use-intl';

import { Button } from '@/components/ui/button';
import { useAuth } from '@/modules/auth';

export function HeroSection() {
   const t = useTranslations('home');
   const { user } = useAuth();

   return (
      <section className="relative z-10 overflow-hidden border-b border-white/8 bg-[#09070d]">
         <div className="absolute inset-0 bg-[radial-gradient(circle_at_72%_48%,rgba(241,105,184,.20),transparent_35%),radial-gradient(circle_at_30%_20%,rgba(151,91,255,.10),transparent_32%)]" />
         <div className="relative mx-auto grid min-h-[31rem] max-w-[1280px] grid-cols-1 items-center px-5 py-10 sm:px-8 lg:grid-cols-[.9fr_1.1fr] lg:px-10 lg:py-0">
            <div className="relative z-10 flex max-w-2xl flex-col gap-6 py-10 lg:py-16">
               <div className="flex items-center gap-2 text-[11px] font-semibold tracking-[.28em] text-[#f06ab7] uppercase">
                  <span className="h-px w-10 bg-[#f06ab7]" />
                  {t('hero.eyebrow')}
               </div>
               <h1 className="text-5xl leading-[.95] font-semibold tracking-[-.045em] text-white sm:text-6xl lg:text-7xl">
                  <span>snore</span><span className="text-[#f06ab7]">saber</span>
               </h1>
               <p className="max-w-xl text-base leading-7 text-white/62 sm:text-lg">{t('hero.description')}</p>

               <div className="grid max-w-2xl grid-cols-1 gap-2.5 sm:grid-cols-3">
                  <Feature icon={<Search />} title={t('hero.trackTitle')} text={t('hero.trackDescription')} />
                  <Feature icon={<Map />} title={t('hero.exploreTitle')} text={t('hero.exploreDescription')} />
                  <Feature icon={<Users />} title={t('hero.competeTitle')} text={t('hero.competeDescription')} />
               </div>

               <div className="flex flex-col gap-3 pt-1 sm:flex-row">
                  <Button asChild size="lg" className="h-12 rounded-xl bg-[#f06ab7] px-6 text-[#160b13] hover:bg-[#ff83c7]">
                     <a href="/rankings">
                        <Trophy data-icon />
                        {t('hero.primaryAction')}
                        <ArrowRight data-icon />
                     </a>
                  </Button>
                  <Button asChild size="lg" variant="outline" className="h-12 rounded-xl border-white/15 bg-white/4 px-6 text-white hover:bg-white/8">
                     <a href="/maps"><Map data-icon />{t('hero.exploreAction')}</a>
                  </Button>
                  {!user && (
                     <Button asChild size="lg" variant="ghost" className="h-12 rounded-xl text-white/70 hover:bg-white/5 hover:text-white">
                        <a href="/login">{t('hero.secondaryAction')}</a>
                     </Button>
                  )}
               </div>
            </div>

            <div className="relative hidden min-h-[31rem] overflow-hidden lg:block">
               <div className="absolute inset-0 bg-[linear-gradient(90deg,#09070d_0%,transparent_28%)]" />
               <img src="/assets/snoresaber-hero-art.png" alt="" className="absolute inset-0 size-full object-cover object-center opacity-95" />
               <div className="absolute inset-y-0 right-0 w-1/3 bg-gradient-to-l from-[#09070d]/20 to-transparent" />
            </div>
         </div>
      </section>
   );
}

function Feature({ icon, title, text }: { icon: React.ReactNode; title: string; text: string }) {
   return (
      <div className="flex min-w-0 items-center gap-3 rounded-xl border border-white/8 bg-white/[.025] px-3.5 py-3 backdrop-blur-sm">
         <div className="flex size-9 shrink-0 items-center justify-center rounded-lg border border-[#f06ab7]/25 bg-[#f06ab7]/8 text-[#f06ab7]">{icon}</div>
         <div className="min-w-0">
            <div className="text-sm font-semibold text-white">{title}</div>
            <div className="mt-0.5 text-[11px] leading-4 text-white/45">{text}</div>
         </div>
      </div>
   );
}
