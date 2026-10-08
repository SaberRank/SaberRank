export async function api(path, options = {}) {
  const response = await fetch(`/api/${path.replace(/^\/+/, "")}`, {
    credentials: "include",
    headers: { "Content-Type": "application/json", ...(options.headers || {}) },
    ...options
  });
  const text = await response.text();
  let data = {};
  try { data = text ? JSON.parse(text) : {}; } catch { data = { error: text }; }
  if (!response.ok) throw new Error(data.error || `Request failed (${response.status})`);
  return data;
}