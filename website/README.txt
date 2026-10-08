SnoreSaber native account authentication fix

The previous UI already exposed a SnoreSaber password login, but the custom
/api/v2 server did not implement the password endpoints it called. That made
the SnoreSaber button appear but fail when submitted.

This patch:
- implements POST /api/v2/auth/password/login
- implements POST /api/v2/auth/password/signup
- stores email + scrypt password hashes in PostgreSQL
- issues the normal SnoreSaber session token after login/signup
- changes account creation to a direct email/display-name/password flow instead
  of a verification-code flow that had no backend email challenge implementation
- keeps Steam separate
- does not add Discord or passkeys

IMPORTANT: this intentionally does not pretend to verify ownership of the email
address. Email verification/reset can be added once an email delivery provider
is configured. Passwords are never stored in plaintext.
