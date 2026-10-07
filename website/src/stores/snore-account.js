import {writable} from 'svelte/store';

const store = writable(null);
let initialized = false;

export async function loadSnoreAccount() {
  if (initialized) return;
  initialized = true;
  try {
    const response = await fetch('/api/auth/me', {credentials: 'include'});
    const json = await response.json();
    store.set(json?.user || null);
  } catch (_) {
    store.set(null);
  }
}

export async function connectScoreSaber(scoresaberId) {
  const response = await fetch('/api/account/link', {
    method: 'POST',
    headers: {'content-type': 'application/json'},
    credentials: 'include',
    body: JSON.stringify({scoresaberId}),
  });
  const json = await response.json();
  if (!response.ok) throw new Error(json?.error || 'Unable to link ScoreSaber profile.');
  await loadSnoreAccount();
  return json;
}

export async function logoutSnoreAccount() {
  await fetch('/api/auth/logout', {method: 'POST', credentials: 'include'});
  store.set(null);
}

export const clearSnoreProfile = logoutSnoreAccount;
export function setSnoreProfile(profile) { store.set(profile || null); }
export default store;
