import { createFileRoute } from '@tanstack/react-router';

import { getAdminReports } from '@/modules/admin/actions/admin';
import { PlayerReports } from '@/modules/admin/player-reports';
import { PageError } from '@/shared/components/error/page-error';

export const Route = createFileRoute('/admin/reports')({
   loader: () => getAdminReports(),
   component: AdminReportsRoute
});

function AdminReportsRoute() {
   const result = Route.useLoaderData();
   if (!result.ok) return <PageError status={null} />;
   return <PlayerReports initialReports={result.value} />;
}
