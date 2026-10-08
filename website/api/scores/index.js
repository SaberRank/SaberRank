export default async function handler(req, res) {
  if (req.method !== "POST") return res.status(405).json({ error: "POST required" });
  if (!process.env.SNORE_INGEST_KEY || req.headers["x-snoresaber-key"] !== process.env.SNORE_INGEST_KEY) {
    return res.status(401).json({ error: "Unauthorized" });
  }
  res.status(202).json({ accepted: true, source: "snoresaber" });
}