export default async function handler(req, res) {
  const returnUrl = process.env.STEAM_RETURN_URL;
  if (!returnUrl) return res.status(500).send("STEAM_RETURN_URL is not configured.");
  const u = new URL(returnUrl);
  const params = new URLSearchParams({
    "openid.ns": "http://specs.openid.net/auth/2.0",
    "openid.mode": "checkid_setup",
    "openid.return_to": returnUrl,
    "openid.realm": u.origin + "/",
    "openid.identity": "http://specs.openid.net/auth/2.0/identifier_select",
    "openid.claimed_id": "http://specs.openid.net/auth/2.0/identifier_select"
  });
  res.redirect("https://steamcommunity.com/openid/login?" + params.toString());
}