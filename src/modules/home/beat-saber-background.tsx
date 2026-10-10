import type { CSSProperties } from 'react';

import { BeatSaberParticles } from './beat-saber-particles';

const MENU_BACKGROUND_STYLE: CSSProperties = {
   background: [
      'radial-gradient(ellipse 72% 48% at 74% 18%, rgba(240,106,183,0.18) 0%, rgba(240,106,183,0.07) 38%, rgba(240,106,183,0) 72%)',
      'radial-gradient(ellipse 65% 55% at 20% 58%, rgba(151,91,255,0.12) 0%, rgba(151,91,255,0.04) 42%, rgba(151,91,255,0) 76%)',
      'radial-gradient(ellipse 90% 45% at 50% 100%, rgba(100,70,170,0.12) 0%, rgba(100,70,170,0) 70%)',
      'linear-gradient(180deg, #08060c 0%, #0a0710 42%, #0b0912 100%)'
   ].join(', ')
};

const FLOOR_HAZE_STYLE: CSSProperties = {
   background: 'linear-gradient(180deg, rgba(240,106,183,0) 0%, rgba(240,106,183,0.06) 38%, rgba(31,18,45,0.36) 100%)'
};

const PAGE_DARKENING_STYLE: CSSProperties = {
   background: [
      'radial-gradient(ellipse 155% 76% at 50% 0%, rgba(0, 0, 0, 0) 0%, rgba(0, 0, 0, 0) 42%, rgba(0, 0, 0, 0.24) 68%, rgba(0, 0, 0, 0.64) 100%)',
      'linear-gradient(180deg, rgba(0, 0, 0, 0) 0%, rgba(0, 0, 0, 0.18) 34%, rgba(0, 0, 0, 0.78) 100%)'
   ].join(', ')
};

export function BeatSaberPageBackground() {
   return (
      <div className="pointer-events-none absolute inset-0 z-0 overflow-hidden" aria-hidden>
         <div className="absolute inset-0 overflow-hidden" style={MENU_BACKGROUND_STYLE}>
            <BeatSaberParticles />
            <div className="absolute inset-x-[-20%] bottom-[-18%] h-[58%]" style={FLOOR_HAZE_STYLE} />
            <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_center,transparent_0%,transparent_45%,rgba(0,0,0,0.62)_100%)]" />
         </div>
         <div className="absolute inset-x-0 top-[10rem] bottom-0" style={PAGE_DARKENING_STYLE} />
      </div>
   );
}
