import '@tanstack/react-start/server-only';

import { getRequestHeaders } from '@tanstack/react-start/server';

import { readAuthCookie } from '@/modules/auth/actions/session.server';
import { getClientRequestHeaders } from '@/shared/api/client-request.server';
import { actionFailure, actionSuccess, type ActionResult } from '@/shared/result/action';

type LocalRequestOptions = {
   method?: 'GET' | 'POST' | 'PUT' | 'DELETE';
   body?: unknown;
};

export type LocalPageData<T> = { ok: true; data: T } | { ok: false; status: number | null; message: string };

export async function localApiPageData<T>(path: string): Promise<LocalPageData<T>> {
   try {
      const origin = getCurrentOrigin();
      const token = readAuthCookie();
      const headers: Record<string, string> = { accept: 'application/json', ...getClientRequestHeaders() };
      if (token) headers.cookie = `token=${encodeURIComponent(token)}`;
      const response = await fetch(`${origin}/api/v2${path}`, { headers, cache: 'no-store' });
      const payload: unknown = await response.json().catch(() => null);
      if (!response.ok) {
         const message =
            typeof payload === 'object' && payload !== null && 'message' in payload && typeof payload.message === 'string'
               ? payload.message
               : `Request failed (${response.status})`;
         return { ok: false, status: response.status, message };
      }
      return { ok: true, data: payload as T };
   } catch (error) {
      return { ok: false, status: null, message: error instanceof Error ? error.message : 'Request failed' };
   }
}

/** Call this deployment's own API so writes cannot drift to a stale API_URL host. */
export async function localApiAction<T>(path: string, options: LocalRequestOptions = {}): Promise<ActionResult<T>> {
   try {
      const origin = getCurrentOrigin();
      const token = readAuthCookie();
      const headers: Record<string, string> = {
         accept: 'application/json',
         ...getClientRequestHeaders()
      };

      if (token) headers.cookie = `token=${encodeURIComponent(token)}`;
      if (options.body !== undefined) headers['content-type'] = 'application/json';

      const response = await fetch(`${origin}/api/v2${path}`, {
         method: options.method ?? 'GET',
         headers,
         ...(options.body !== undefined && { body: JSON.stringify(options.body) }),
         cache: 'no-store'
      });
      const payload: unknown = await response.json().catch(() => null);

      if (!response.ok) {
         const message =
            typeof payload === 'object' && payload !== null && 'message' in payload && typeof payload.message === 'string'
               ? payload.message
               : `Request failed (${response.status})`;
         return actionFailure(message);
      }

      return actionSuccess(payload as T);
   } catch (error) {
      return actionFailure(error instanceof Error ? error.message : 'Request failed');
   }
}

function getCurrentOrigin() {
   const headers = getRequestHeaders();
   const host = (headers.get('x-forwarded-host') ?? headers.get('host') ?? '').split(',')[0].trim();
   const protocol = (headers.get('x-forwarded-proto') ?? 'https').split(',')[0].trim();
   if (!host) throw new Error('Unable to determine the SnoreSaber server address.');
   if (protocol !== 'http' && protocol !== 'https') throw new Error('Invalid SnoreSaber server protocol.');
   return `${protocol}://${host}`;
}

export async function localApiOptionalData<T>(path: string): Promise<T | null> {
   const result = await localApiAction<T>(path);
   return result.ok ? result.value : null;
}
