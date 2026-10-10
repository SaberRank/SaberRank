import { createServerFn } from '@tanstack/react-start';

import { setAuthCookie } from '@/modules/auth/actions/session.server';
import { getRequestHeaders } from '@tanstack/react-start/server';
import { getClientRequestHeaders } from '@/shared/api/client-request.server';
import type { RequestParams } from '@/shared/api/generated/Api';
import type { PasswordAuthControllerGetPasswordCredentialResponse } from '@/shared/api/generated/ApiParams';
import { api } from '@/shared/api/server-api';
import { actionApiData, actionSuccess, type ActionResult } from '@/shared/result/action';

export type CredentialAuthActionValue = { status: 'authenticated'; playerId: string } | { status: 'support-required' };
export type PasswordCredentialSummary = PasswordAuthControllerGetPasswordCredentialResponse;

type CredentialAuthResponse = Awaited<ReturnType<typeof api.auth.passwordAuthControllerCompleteSignup>>['data'];

function requestOptions(): RequestParams {
   return { cache: 'no-store', headers: getClientRequestHeaders() };
}

async function localPasswordAuth(path: string, data: Record<string, string>): Promise<ActionResult<CredentialAuthResponse>> {
   try {
      // Always call the API on the exact host handling this request. Using a
      // hard-coded/default API_URL can send authentication to an older Vercel
      // deployment and makes signup/login appear inconsistent.
      const requestHeaders = getRequestHeaders();
      const host = requestHeaders.get('x-forwarded-host') ?? requestHeaders.get('host');
      const protocol = requestHeaders.get('x-forwarded-proto') ?? 'https';
      if (!host) return { ok: false, error: 'Unable to determine the SnoreSaber server address.' };
      const apiUrl = `${protocol}://${host}/api/v2${path}`;
      const response = await fetch(apiUrl, {
         method: 'POST',
         headers: { 'content-type': 'application/json', ...getClientRequestHeaders() },
         body: JSON.stringify(data),
         cache: 'no-store'
      });
      const payload = await response.json().catch(() => ({}));
      if (!response.ok) return { ok: false, error: String(payload?.message || 'Authentication failed') };
      return { ok: true, value: payload as CredentialAuthResponse };
   } catch (error) {
      return { ok: false, error: error instanceof Error ? error.message : 'Authentication failed' };
   }
}

function finishAuth(result: ActionResult<CredentialAuthResponse>): ActionResult<CredentialAuthActionValue> {
   if (!result.ok) return result;

   if (result.value.status === 'authenticated') {
      setAuthCookie(result.value.token);
      return actionSuccess({ status: 'authenticated', playerId: result.value.playerId });
   }

   return actionSuccess(result.value);
}

const startSignupFn = createServerFn({ method: 'POST' })
   .validator((email: string) => email)
   .handler(({ data: email }) => actionSuccess({ challengeId: 'direct', expiresAt: new Date(Date.now() + 10 * 60_000).toISOString(), resendAvailableAt: new Date().toISOString(), email }));

export async function startSignup(email: string) {
   return startSignupFn({ data: email });
}

const completeSignupFn = createServerFn({ method: 'POST' })
   .validator((data: { email: string; challengeId: string; code: string; password: string; displayName: string }) => data)
   .handler(async ({ data }) => finishAuth(await localPasswordAuth('/auth/password/signup', { email: data.email, password: data.password, displayName: data.displayName })));

export async function completeSignup(data: { email: string; challengeId: string; code: string; password: string; displayName: string }) {
   return completeSignupFn({ data });
}

const loginWithPasswordFn = createServerFn({ method: 'POST' })
   .validator((data: { email: string; password: string }) => data)
   .handler(async ({ data }) => finishAuth(await localPasswordAuth('/auth/password/login', data)));

export async function loginWithPassword(data: { email: string; password: string }) {
   return loginWithPasswordFn({ data });
}

const startPasswordResetFn = createServerFn({ method: 'POST' })
   .validator((email: string) => email)
   .handler(({ data: email }) => actionApiData(api.auth.passwordAuthControllerStartPasswordReset({ email }, requestOptions())));

export async function startPasswordReset(email: string) {
   return startPasswordResetFn({ data: email });
}

const completePasswordResetFn = createServerFn({ method: 'POST' })
   .validator((data: { email: string; challengeId: string; code: string; password: string }) => data)
   .handler(async ({ data }) => finishAuth(await actionApiData(api.auth.passwordAuthControllerCompletePasswordReset(data, requestOptions()))));

export async function completePasswordReset(data: { email: string; challengeId: string; code: string; password: string }) {
   return completePasswordResetFn({ data });
}

const getPasswordCredentialFn = createServerFn({ method: 'GET' }).handler(() =>
   actionApiData(api.auth.passwordAuthControllerGetPasswordCredential(requestOptions()))
);

export async function getPasswordCredential() {
   return getPasswordCredentialFn();
}

const startPasswordSetupFn = createServerFn({ method: 'POST' })
   .validator((email: string) => email)
   .handler(({ data: email }) => actionApiData(api.auth.passwordAuthControllerStartPasswordSetup({ email }, requestOptions())));

export async function startPasswordSetup(email: string) {
   return startPasswordSetupFn({ data: email });
}

const completePasswordSetupFn = createServerFn({ method: 'POST' })
   .validator((data: { email: string; challengeId: string; code: string; password: string }) => data)
   .handler(async ({ data }) => {
      const result = await actionApiData(api.auth.passwordAuthControllerCompletePasswordSetup(data, requestOptions()));

      if (result.ok) {
         setAuthCookie(result.value.token);
         return actionSuccess(undefined);
      }

      return result;
   });

export async function completePasswordSetup(data: { email: string; challengeId: string; code: string; password: string }) {
   return completePasswordSetupFn({ data });
}

const changePasswordFn = createServerFn({ method: 'POST' })
   .validator((data: { currentPassword: string; newPassword: string }) => data)
   .handler(async ({ data }) => {
      const result = await actionApiData(api.auth.passwordAuthControllerChangePassword(data, requestOptions()));

      if (result.ok) {
         // password change rotates every session; keep this one alive with the fresh token
         setAuthCookie(result.value.token);
         return actionSuccess(undefined);
      }

      return result;
   });

export async function changePassword(data: { currentPassword: string; newPassword: string }) {
   return changePasswordFn({ data });
}
