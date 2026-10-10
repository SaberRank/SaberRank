'use client';

import { useQuery } from '@tanstack/react-query';

import { getAdminReports, type AdminPlayerReport } from '@/modules/admin/actions/admin';
import { unwrapAction } from '@/shared/result/action';

const reasonLabels: Record<string, string> = {
   INAPPROPRIATE_PROFILE: 'Inappropriate profile',
   IMPERSONATION: 'Impersonation',
   HARASSMENT: 'Harassment',
   CHEATING: 'Cheating',
   OTHER: 'Other'
};

export function PlayerReports({ initialReports }: { initialReports: AdminPlayerReport[] }) {
   const query = useQuery({
      queryKey: ['admin', 'reports'],
      queryFn: async () => unwrapAction(await getAdminReports()),
      initialData: initialReports,
      staleTime: 0
   });

   return (
      <section className="flex flex-col gap-3">
         <div>
            <h2 className="font-semibold">Player reports</h2>
            <p className="text-muted-foreground text-sm">Review the reported player, report reason, details, and reporter information.</p>
         </div>
         {query.data.length === 0 ? (
            <div className="text-muted-foreground rounded-md border p-8 text-center text-sm">No player reports have been submitted.</div>
         ) : (
            <div className="flex flex-col gap-3">
               {query.data.map((report) => <ReportCard key={report.id} report={report} />)}
            </div>
         )}
      </section>
   );
}

function ReportCard({ report }: { report: AdminPlayerReport }) {
   return (
      <article className="rounded-md border p-4">
         <div className="flex flex-wrap items-center justify-between gap-2 border-b pb-3">
            <div className="font-medium">{reasonLabels[report.reason] || report.reason}</div>
            <div className="text-muted-foreground text-xs">{new Date(report.createdAt).toLocaleString()} · {report.status}</div>
         </div>
         <p className="whitespace-pre-wrap py-3 text-sm">{report.details || 'No additional details provided.'}</p>
         <div className="grid gap-3 border-t pt-3 text-sm md:grid-cols-2">
            <PlayerSummary label="Reported player" id={report.targetPlayerId} name={report.targetName} country={report.targetCountry} avatar={report.targetAvatar} role={report.targetRole} permissions={report.targetPermissions} />
            <PlayerSummary label="Reported by" id={report.reporterId} name={report.reporterName} country={report.reporterCountry} avatar={report.reporterAvatar} role={report.reporterRole} permissions={report.reporterPermissions} />
         </div>
         <div className="mt-3 border-t pt-3 text-sm">
            <span className={report.targetBanned ? 'text-destructive font-medium' : 'text-muted-foreground'}>{report.targetBanned ? 'Currently banned' : 'Not currently banned'}</span>
            {report.targetBanned && report.targetBanReason && <span className="text-muted-foreground"> · {report.targetBanReason}</span>}
            {report.targetBanned && report.targetAppealAt && <span className="text-muted-foreground"> · Appeal available {new Date(report.targetAppealAt).toLocaleString()}</span>}
         </div>
      </article>
   );
}

function PlayerSummary({ label, id, name, country, avatar, role, permissions }: { label: string; id: string; name: string; country: string; avatar: string; role: string | null; permissions: number }) {
   return (
      <div className="flex min-w-0 items-center gap-3">
         <img src={avatar || `https://ui-avatars.com/api/?name=${encodeURIComponent(name)}`} alt="" className="size-10 rounded-full object-cover" />
         <div className="min-w-0">
            <div className="text-muted-foreground text-xs">{label}</div>
            <a href={`/u/${encodeURIComponent(id)}`} className="font-medium hover:underline">{name}</a>
            <div className="text-muted-foreground truncate text-xs">{country} · ID {id}{role ? ` · ${role}` : ''} · permissions {permissions}</div>
         </div>
      </div>
   );
}
