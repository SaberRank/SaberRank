
import { createFileRoute, Link, useRouter } from '@tanstack/react-router';
import { useState } from 'react';
import { toast } from 'sonner';
import { Avatar, AvatarFallback, AvatarImage } from '@/components/ui/avatar';
import { Card, CardContent } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { removeTeamMember } from '@/modules/team/actions/admin';
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
   const router = useRouter();
   const [removingId, setRemovingId] = useState<string | null>(null);
   const members = data.value.members;

   async function handleRemove(memberId: string, memberName: string) {
      if (!window.confirm(`Remove ${memberName} from the SnoreSaber team? Their account will not be deleted.`)) return;
      setRemovingId(memberId);
      const result = await removeTeamMember(memberId);
      setRemovingId(null);
      if (!result.ok) {
         toast.error(result.error || 'Could not remove team member.');
         return;
      }
      toast.success(`${memberName} was removed from the team.`);
      await router.invalidate();
   }

   const roleOrder = [
      'Owner',
      'Administrator',
      'Admin',
      'Developer',
      'Tester',
      'QAT Head',
      'QAT',
      'NAT',
      'RT',
      'Ranking Team',
      'RTR',
      'CAT',
      'CCT Lead',
      'CCT',
      'Tournament Organizer',
      'Replay Team',
      'SnoreSaber Staff'
   ];

   const roleRank = (role: string) => {
      const index = roleOrder.findIndex((knownRole) => knownRole.toLowerCase() === role.toLowerCase());
      return index === -1 ? roleOrder.length : index;
   };

   const groups = Array.from(
      members.reduce((map, member) => {
         const key = member.role.trim() || 'SnoreSaber Staff';
         const group = map.get(key) ?? [];
         group.push(member);
         map.set(key, group);
         return map;
      }, new Map<string, typeof members>())
   ).sort(([roleA], [roleB]) => {
      const rankDiff = roleRank(roleA) - roleRank(roleB);
      return rankDiff !== 0 ? rankDiff : roleA.localeCompare(roleB);
   });

   return (
      <div className="relative min-h-full flex-1 overflow-hidden">
         <SetPageBackground src="/images/banner.jpg" />
         <main className="app-container relative z-10 flex flex-col gap-8 p-4 md:p-8">
            <h1 className="text-2xl font-semibold">SnoreSaber Team</h1>

            {groups.length === 0 ? (
               <div className="text-muted-foreground rounded-lg border border-dashed p-10 text-center text-sm">No team roles have been assigned yet.</div>
            ) : (
               <div className="flex flex-col gap-8">
                  {groups.map(([role, roleMembers]) => (
                     <section key={role} className="flex flex-col gap-3">
                        <h2 className="text-lg font-semibold">{role}</h2>
                        <div className="grid grid-cols-1 gap-3 sm:grid-cols-2 lg:grid-cols-3">
                           {roleMembers.map((member) => (
                              <Card key={member.id} className="h-full transition-colors hover:border-primary/60">
                                 <CardContent className="flex items-center gap-3 p-4">
                                    <Link to="/u/$playerId" params={{ playerId: member.vanity || member.id }} className="group flex min-w-0 flex-1 items-center gap-4">
                                       <Avatar className="size-14 shrink-0">
                                          <AvatarImage src={member.avatar} alt="" />
                                          <AvatarFallback>{member.name.slice(0, 2).toUpperCase()}</AvatarFallback>
                                       </Avatar>
                                       <div className="min-w-0">
                                          <div className="truncate font-semibold group-hover:text-primary">{member.name}</div>
                                       </div>
                                    </Link>
                                    {data.value.viewerIsAdmin && (
                                       <Button type="button" variant="destructive" size="sm" disabled={removingId !== null} onClick={() => void handleRemove(member.id, member.name)}>
                                          {removingId === member.id ? 'Removing…' : 'Remove'}
                                       </Button>
                                    )}
                                 </CardContent>
                              </Card>
                           ))}
                        </div>
                     </section>
                  ))}
               </div>
            )}
         </main>
      </div>
   );
}
