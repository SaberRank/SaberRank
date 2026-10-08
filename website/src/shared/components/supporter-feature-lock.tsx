'use client';

import type { ReactNode } from 'react';

interface SupporterFeatureLockProps {
   children: ReactNode;
   locked: boolean;
   patreonConnected?: boolean;
   variant?: 'floating' | 'field';
   className?: string;
   contentClassName?: string;
   title?: ReactNode;
   description?: ReactNode;
}

export function SupporterFeatureLock({ children, className, contentClassName }: SupporterFeatureLockProps) {
   return <div className={className}><div className={contentClassName}>{children}</div></div>;
}
