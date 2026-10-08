# SnoreSaber auth fix

This patch fixes the two auth problems shown on the login screen.

## 1. Email / Meta PC login

The Meta button uses SnoreSaber's email one-time-code flow. The frontend was already calling:

- POST `/api/v2/auth/email/start`
- POST `/api/v2/auth/email/verify`

but the custom SnoreSaber API did not implement those routes, so it returned `endpoint ... is not implemented yet`.

The patch adds both endpoints, stores hashed one-time codes in PostgreSQL, expires them after 10 minutes, invalidates previous codes, and signs the user in after successful verification.

### Required Vercel environment variables

Set these in the SnoreSaber Vercel project:

- `RESEND_API_KEY` = your Resend API key
- `RESEND_FROM_EMAIL` = a sender on a domain verified in Resend, e.g. `SnoreSaber <noreply@snoresaber.com>`

The existing `DATABASE_URL` and `SESSION_SECRET` are still required.

Without an email provider, there is no safe way for a production website to send the six-digit code. Do not expose the code in the API response as a workaround.

## 2. SnoreSaber password login

The previous patch's `/api/v2/auth/password/login` and `/api/v2/auth/password/signup` routes remain in the server. Passwords continue to use scrypt hashes.

## Important Meta behavior

The Meta button is the **Meta PC email-proof/login flow** already described by the UI. It is not a public Meta/Oculus OAuth account-creation API. New Quest standalone players should create a normal SnoreSaber account first, then use the Quest/in-game linking flow.
