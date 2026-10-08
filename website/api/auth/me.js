export default async function handler(req, res) {
  res.status(200).json({ authenticated: false, player: null });
}