'use client';

import { createContext, useCallback, useContext, useEffect, useMemo, useState } from 'react';

import { useQueryClient } from '@tanstack/react-query';
import { useRouter } from '@tanstack/react-router';

import type { UserControllerGetMeResponse } from '@/shared/api/generated/ApiParams';

type AuthContextValue = {
   user: UserControllerGetMeResponse | null;
   refreshAuth: () => Promise<void>;
};

type RootShellAuthData = { user: UserControllerGetMeResponse | null };

const ROOT_SHELL_QUERY_KEY = ['root-shell'];
const AuthContext = createContext<AuthContextValue>({ user: null, refreshAuth: async () => undefined });

export function AuthProvider({ initialUser, children }: { initialUser: UserControllerGetMeResponse | null; children: React.ReactNode }) {
   const router = useRouter();
   const queryClient = useQueryClient();
   const [user, setUser] = useState<UserControllerGetMeResponse | null>(initialUser);

   // Keep the context in sync if a normal route load supplies a new server user.
   useEffect(() => {
      setUser(initialUser);
   }, [initialUser]);

   const refreshAuth = useCallback(async () => {
      // root-shell is populated by ensureQueryData in the root loader, not by an
      // active useQuery observer. Invalidating alone may therefore leave its
      // cached user untouched. Remove it so router.invalidate must read the
      // session cookie again, then copy the freshly loaded user into context.
      queryClient.removeQueries({ queryKey: ROOT_SHELL_QUERY_KEY, exact: true });
      await router.invalidate();
      const refreshed = queryClient.getQueryData<RootShellAuthData>(ROOT_SHELL_QUERY_KEY);
      setUser(refreshed?.user ?? null);
   }, [queryClient, router]);

   const value = useMemo(() => ({ user, refreshAuth }), [user, refreshAuth]);

   return <AuthContext value={value}>{children}</AuthContext>;
}

export function useAuth() {
   return useContext(AuthContext);
}
