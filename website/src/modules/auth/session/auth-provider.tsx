'use client';

import { createContext, useCallback, useContext, useMemo } from 'react';

import { useQueryClient } from '@tanstack/react-query';
import { useRouter } from '@tanstack/react-router';

import type { UserControllerGetMeResponse } from '@/shared/api/generated/ApiParams';

type AuthContextValue = {
   user: UserControllerGetMeResponse | null;
   refreshAuth: () => Promise<void>;
};

const AuthContext = createContext<AuthContextValue>({ user: null, refreshAuth: async () => undefined });

export function AuthProvider({ initialUser, children }: { initialUser: UserControllerGetMeResponse | null; children: React.ReactNode }) {
   const router = useRouter();
   const queryClient = useQueryClient();
   const refreshAuth = useCallback(async () => {
      await queryClient.invalidateQueries({ queryKey: ['root-shell'], exact: true, refetchType: 'none' });
      await router.invalidate();
   }, [queryClient, router]);

   const value = useMemo(() => ({ user: initialUser, refreshAuth }), [initialUser, refreshAuth]);

   return <AuthContext value={value}>{children}</AuthContext>;
}

export function useAuth() {
   return useContext(AuthContext);
}
