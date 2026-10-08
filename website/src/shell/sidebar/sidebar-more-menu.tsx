'use client';

import { useEffect, useState, useTransition } from 'react';

import { useQueryClient } from '@tanstack/react-query';
import { getRouteApi, Link, useRouter } from '@tanstack/react-router';
import { ChevronRight, Cookie, Copyright, Loader2, LogOut, Scale, Settings, Shield, ShieldCheck } from 'lucide-react';
import { useTranslations } from 'use-intl';

import { Button } from '@/components/ui/button';
import { Popover, PopoverContent, PopoverTrigger } from '@/components/ui/popover';
import { Separator } from '@/components/ui/separator';

import { useAuth } from '@/modules/auth';
import { logout } from '@/modules/auth/actions/member';
import { cn } from '@/shared/format/helpers';
import Permissions from '@/shared/permissions';
import { SidebarAppSettings } from '@/shell/sidebar/sidebar-app-settings';

const privacyRoute = getRouteApi('/legal/privacy');
const copyrightRoute = getRouteApi('/legal/copyright');
const cookiesPolicyRoute = getRouteApi('/legal/cookies-policy');
const settingsAccountRoute = getRouteApi('/settings/account');

type SidebarMoreMenuProps = {
   trigger: React.ReactNode;
   side?: 'top' | 'right' | 'bottom' | 'left';
   align?: 'start' | 'center' | 'end';
};

export function SidebarMoreMenu({ trigger, side = 'top', align = 'end' }: SidebarMoreMenuProps) {
   const { user } = useAuth();
   const router = useRouter();
   const queryClient = useQueryClient();
   const [mounted, setMounted] = useState(false);
   const [open, setOpen] = useState(false);
   const [isLanguageOpen, setIsLanguageOpen] = useState(false);
   const [isLegalOpen, setIsLegalOpen] = useState(false);
   const [pending, startTransition] = useTransition();
   const tNav = useTranslations();
   const tSidebar = useTranslations();
   const menuActionClass = 'h-8 w-full cursor-pointer justify-start rounded-md px-2.5 text-[13px]';
   const isAdmin = !!user && Permissions.checkPermissionNumber(user.permissions, Permissions.security.ADMIN);

   useEffect(() => {
      setMounted(true);
   }, []);

   function handleLogout() {
      startTransition(async () => {
         await logout();
         queryClient.removeQueries({ queryKey: ['root-shell'] });
         await router.invalidate();
      });
   }

   function handleOpenChange(nextOpen: boolean) {
      if (!nextOpen && (isLanguageOpen || isLegalOpen)) {
         return;
      }

      setOpen(nextOpen);

      if (!nextOpen) {
         setIsLanguageOpen(false);
         setIsLegalOpen(false);
      }
   }

   function closeMenu() {
      setIsLegalOpen(false);
      setOpen(false);
   }

   // avoid radix id drift on fresh loads:
   // https://github.com/radix-ui/primitives/issues/3700
   // https://github.com/shadcn-ui/ui/issues/1018
   if (!mounted) {
      return <>{trigger}</>;
   }

   return (
      <Popover open={open} onOpenChange={handleOpenChange}>
         <PopoverTrigger asChild>{trigger}</PopoverTrigger>
         <PopoverContent
            side={side}
            align={align}
            collisionPadding={16}
            onInteractOutside={(event) => {
               if (isLanguageOpen || isLegalOpen) {
                  event.preventDefault();
               }
            }}
            className="w-68 p-0 sm:w-70"
         >
            <div className="flex flex-col gap-3 p-3">
               <SidebarAppSettings isLanguageOpen={isLanguageOpen} onLanguageOpenChangeAction={setIsLanguageOpen} />

               <Separator />
               <div className="flex flex-col gap-1">
                  <Popover open={isLegalOpen} onOpenChange={setIsLegalOpen}>
                     <PopoverTrigger asChild>
                        <Button variant="menu" size="sm" className={cn(menuActionClass, 'cursor-default')}>
                           <Scale data-icon />
                           <span className="flex-1 text-left">{tSidebar('sidebar.legal')}</span>
                           <ChevronRight data-icon className="ml-auto" aria-hidden="true" />
                        </Button>
                     </PopoverTrigger>
                     <PopoverContent side="right" align="start" collisionPadding={16} className="w-56 p-2">
                        <div className="flex flex-col gap-1">
                           <Button asChild variant="menu" size="sm" className={menuActionClass}>
                              <privacyRoute.Link onClick={closeMenu}>
                                 <span className="flex min-w-0 items-center gap-2">
                                    <Shield data-icon />
                                    <span className="truncate">{tSidebar('common.privacyPolicy')}</span>
                                 </span>
                              </privacyRoute.Link>
                           </Button>
                           <Button asChild variant="menu" size="sm" className={menuActionClass}>
                              <cookiesPolicyRoute.Link onClick={closeMenu}>
                                 <span className="flex min-w-0 items-center gap-2">
                                    <Cookie data-icon />
                                    <span className="truncate">{tSidebar('common.cookiesPolicy')}</span>
                                 </span>
                              </cookiesPolicyRoute.Link>
                           </Button>
                           <Button asChild variant="menu" size="sm" className={menuActionClass}>
                              <copyrightRoute.Link onClick={closeMenu}>
                                 <span className="flex min-w-0 items-center gap-2">
                                    <Copyright data-icon />
                                    <span className="truncate">{tSidebar('sidebar.copyrightTakedowns')}</span>
                                 </span>
                              </copyrightRoute.Link>
                           </Button>
                        </div>
                     </PopoverContent>
                  </Popover>
               </div>
            </div>

            {user && (
               <>
                  <Separator />
                  <div className="flex flex-col gap-1 p-2">
                     {isAdmin && (
                        <Button asChild variant="menu" size="sm" className={menuActionClass}>
                           <Link to="/admin" onClick={() => setOpen(false)}>
                              <ShieldCheck data-icon />
                              {tSidebar('sidebar.administration')}
                           </Link>
                        </Button>
                     )}
                     <Button asChild variant="menu" size="sm" className={menuActionClass}>
                        <settingsAccountRoute.Link onClick={() => setOpen(false)}>
                           <Settings data-icon />
                           {tSidebar('sidebar.settings')}
                        </settingsAccountRoute.Link>
                     </Button>
                     <Button variant="menu" size="sm" onClick={handleLogout} disabled={pending} className={menuActionClass}>
                        {pending ? <Loader2 data-icon className="animate-spin" /> : <LogOut data-icon />}
                        {tSidebar('sidebar.logOut')}
                     </Button>
                  </div>
               </>
            )}
         </PopoverContent>
      </Popover>
   );
}
