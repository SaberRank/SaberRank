import { createFileRoute } from '@tanstack/react-router';
import { createServerFn } from '@tanstack/react-start';
import { z } from 'zod';

import { AccountSection } from '@/modules/settings/sections/account-section';
import { SettingsShell } from '@/modules/settings/settings-shell';
import { getClientRequestHeaders } from '@/shared/api/client-request.server';
import { api } from '@/shared/api/server-api';
import { optionalApi } from '@/shared/result/api';
import { buildNoindexHead } from '@/shared/seo/metadata';
import { requestOrNotFound } from '@/shared/url-state/params';
import { SetPageBackground } from '@/shell/background/page-background-provider';

const accountSettingsSearchSchema = z.object({});

const getAccountSettingsData = createServerFn({ method: 'GET' }).handler(async () => {
   const [countryReset, vanity] = await Promise.all([
      optionalApi(api.user.userControllerCanResetCountry({ headers: getClientRequestHeaders() }).then((r) => r.data)),
      optionalApi(api.user.userControllerGetVanity({ cache: 'no-store' }).then((r) => r.data))
   ]);

   return {
      countryReset,
      vanity,
   };
});

export const Route = createFileRoute('/settings/account')({
   validateSearch: (search) => requestOrNotFound(accountSettingsSearchSchema.safeParse(search)),
   loader: () => getAccountSettingsData(),
   head: () => buildNoindexHead('Account Settings', 'Manage your SnoreSaber account settings', '/settings/account'),
   component: SettingsAccountRoute
});

function SettingsAccountRoute() {
   const data = Route.useLoaderData();
   return (
      <>
         <SetPageBackground src="/images/banner.jpg" />
         <SettingsShell activeTab="account">
            <AccountSection
               countryReset={data.countryReset}
               vanity={data.vanity}
            />
         </SettingsShell>
      </>
   );
}
