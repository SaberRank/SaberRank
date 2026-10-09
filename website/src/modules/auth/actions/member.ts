import { createServerFn } from '@tanstack/react-start';

import { clearAuthCookie, setAuthCookie } from '@/modules/auth/actions/session.server';
import { localApiAction } from '@/shared/api/local-action.server';
import { actionSuccess, type ActionResult } from '@/shared/result/action';

type EmailLoginVerificationActionValue =
   | { status: 'authenticated'; playerId: string }
   | { status: 'pending-game-auth' }
   | { status: 'support-required' };

const logoutFn = createServerFn({ method: 'POST' }).handler(async () => {
   await localApiAction<void>('/auth/logout', { method: 'POST' });
   clearAuthCookie();
});

export async function logout() {
   return logoutFn();
}

const startEmailLoginFn = createServerFn({ method: 'POST' })
   .validator((email: string) => email)
   .handler(({ data: email }) => localApiAction<{ challengeId: string; expiresAt: string; resendAvailableAt: string }>('/auth/email/start', { method: 'POST', body: { email } }));

export async function startEmailLogin(email: string) {
   return startEmailLoginFn({ data: email });
}

const verifyEmailLoginFn = createServerFn({ method: 'POST' })
   .validator((data: { challengeId: string; code: string }) => data)
   .handler(async ({ data }): Promise<ActionResult<EmailLoginVerificationActionValue>> => {
      const result = await localApiAction<{ status: string; token: string; playerId: string }>('/auth/email/verify', {
         method: 'POST',
         body: { challengeId: data.challengeId, code: data.code }
      });

      if (result.ok && result.value.status === 'authenticated') {
         setAuthCookie(result.value.token);

         return actionSuccess({
            status: 'authenticated',
            playerId: result.value.playerId
         });
      }

      return result;
   });

export async function verifyEmailLogin(challengeId: string, code: string): Promise<ActionResult<EmailLoginVerificationActionValue>> {
   return verifyEmailLoginFn({ data: { challengeId, code } });
}
