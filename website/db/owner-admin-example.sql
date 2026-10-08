-- Optional: make your Steam account a SnoreSaber administrator for testing.
-- Replace the ID with your Steam ID.
UPDATE players SET permissions = permissions | 16 WHERE id = 'YOUR_STEAM_ID';
