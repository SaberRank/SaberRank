export default async function handler(req, res) {
  res.status(200).json({ source: "snoresaber", players: 0, rankedMaps: 0, scores: 0 });
}