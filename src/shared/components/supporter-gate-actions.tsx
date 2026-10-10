'use client';

import { Button } from '@/components/ui/button';

interface SupporterGateActionsProps {
   size?: 'default' | 'xs';
   align?: 'center' | 'start';
}

export function SupporterGateActions({ size = 'default', align = 'center' }: SupporterGateActionsProps) {
   return (
      <Button size={size} variant="outline" className={align === 'start' ? 'w-fit' : undefined} disabled>
         Free feature
      </Button>
   );
}
