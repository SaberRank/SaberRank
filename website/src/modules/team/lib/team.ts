import '@tanstack/react-start/server-only';

import * as z from 'zod';

const nullableString = z.string().nullable().optional();

const teamMemberSchema = z.object({
   Name: z.string(),
   ProfilePicture: z.string(),
   Discord: nullableString,
   GitHub: nullableString,
   Twitch: nullableString,
   Twitter: nullableString,
   YouTube: nullableString
});

const teamMembersSchema = z.object({
   Backend: z.array(teamMemberSchema),
   Frontend: z.array(teamMemberSchema),
   Mod: z.array(teamMemberSchema),
   PPv3: z.array(teamMemberSchema),
   Admin: z.array(teamMemberSchema),
   NAT: z.array(teamMemberSchema),
   RT: z.array(teamMemberSchema),
   QAT: z.array(teamMemberSchema),
   CAT: z.array(teamMemberSchema),
   CCT: z.array(teamMemberSchema)
});

const teamSchema = z.object({
   TeamMembers: teamMembersSchema
});

type TeamData = z.infer<typeof teamSchema>;

const LOCAL_TEAM: TeamData = {
   TeamMembers: {
      Backend: [
         {
            Name: 'YawningSylveon',
            ProfilePicture: '/assets/snoresaber-icon.png',
            Discord: null,
            GitHub: 'SaberRank/SaberRank',
            Twitch: null,
            Twitter: null,
            YouTube: null
         }
      ],
      Frontend: [
         {
            Name: 'SnoreSaber Contributors',
            ProfilePicture: '/assets/snoresaber-icon.png',
            Discord: null,
            GitHub: 'SaberRank/SaberRank',
            Twitch: null,
            Twitter: null,
            YouTube: null
         }
      ],
      Mod: [],
      PPv3: [],
      Admin: [],
      NAT: [],
      RT: [],
      QAT: [],
      CAT: [],
      CCT: []
   }
};

export async function fetchTeam() {
   // Keep the Team page self-contained so it does not fail when the optional
   // external team repository is unavailable or has a different schema.
   return { ok: true, value: LOCAL_TEAM } as const;
}
