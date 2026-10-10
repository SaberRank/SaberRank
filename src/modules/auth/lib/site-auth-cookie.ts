export const siteAuthCookieMaxAge = 30 * 24 * 60 * 60;

export function getSiteAuthCookieDomain(hostname: string) {
   if (process.env.NODE_ENV === 'production' && (hostname === 'snoresaber.com' || hostname.endsWith('.snoresaber.com'))) return '.snoresaber.com';

   if (process.env.NODE_ENV !== 'production') {
      if (hostname === 'snoresaber.local' || hostname.endsWith('.snoresaber.local')) return '.snoresaber.local';
      if (hostname === 'snoresaber.localhost' || hostname.endsWith('.snoresaber.localhost')) {
         return '.snoresaber.localhost';
      }
   }

   return undefined;
}
