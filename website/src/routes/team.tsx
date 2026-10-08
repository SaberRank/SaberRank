
import { createFileRoute, Link } from '@tanstack/react-router';
import { Avatar, AvatarFallback, AvatarImage } from '@/components/ui/avatar';
import { Card, CardContent } from '@/components/ui/card';
import { fetchTeam } from '@/modules/team/lib/team';
import { buildSeoHead } from '@/shared/seo/metadata';
import { SetPageBackground } from '@/shell/background/page-background-provider';

export const Route = createFileRoute('/team')({
   loader: () => fetchTeam(),
   head: () => buildSeoHead({ title: 'Team', description: 'Meet the SnoreSaber team', path: '/team' }),
   component: TeamRoute
});

function TeamRoute() {
   const data = Route.useLoaderData();
   const members = data.value.members;

   return (
      <div className="relative min-h-full flex-1 overflow-hidden">
         <SetPageBackground src="/images/banner.jpg" />
         <main className="app-container relative z-10 flex flex-col gap-6 p-4 md:p-8">
            <div>
               <h1 className="text-2xl font-semibold">SnoreSaber Team</h1>
               <p className="text-muted-foreground mt-1 text-sm">People with a SnoreSaber staff role.</p>
            </div>
            {members.length === 0 ? (
               <div className="text-muted-foreground rounded-lg border border-dashed p-10 text-center text-sm">No team roles have been assigned yet.</div>
            ) : (
               <div className="grid grid-cols-1 gap-3 sm:grid-cols-2 lg:grid-cols-3">
                  {members.map((member) => (
                     <Link key={member.id} to="/u/$playerId" params={{ playerId: member.id }} className="block">
                        <Card className="group h-full transition-colors hover:border-primary/60">
                           <CardContent className="flex items-center gap-4 p-4">
                              <Avatar className="size-14 shrink-0">
                                 <AvatarImage src={member.avatar} alt="" />
                                 <AvatarFallback>{member.name.slice(0, 2).toUpperCase()}</AvatarFallback>
                              </Avatar>
                              <div className="min-w-0">
                                 <div className="truncate font-semibold group-hover:text-primary">{member.name}</div>
                                 <div className="text-muted-foreground mt-1 truncate text-sm">{member.role}</div>
                              </div>
                           </CardContent>
                        </Card>
                     </Link>
                  ))}
               </div>
            )}
         </main>
      </div>
   );
}
