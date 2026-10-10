'use client';

import type { ReactNode } from 'react';

interface SupporterRequiredOverlayProps {
   patreonConnected?: boolean;
   title?: ReactNode;
   description?: ReactNode;
}

export function SupporterRequiredOverlay({ title, description }: SupporterRequiredOverlayProps) {
   return (
      <div className="mx-auto flex max-w-sm flex-col items-center gap-2 text-center">
         <h3 className="font-semibold">{title ?? 'Feature available'}</h3>
         <p className="text-muted-foreground text-sm">{description ?? 'This SnoreSaber feature is free for everyone.'}</p>
      </div>
   );
}
