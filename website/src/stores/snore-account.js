import {writable} from 'svelte/store';

const KEY = 'snoresaber.profile';

function read() {
  if (typeof localStorage === 'undefined') return null;
  try { return JSON.parse(localStorage.getItem(KEY) || 'null'); } catch (_) { return null; }
}

const store = writable(read());

export function setSnoreProfile(profile) {
  store.set(profile || null);
  if (typeof localStorage !== 'undefined') {
    if (profile) localStorage.setItem(KEY, JSON.stringify(profile));
    else localStorage.removeItem(KEY);
  }
}

export function clearSnoreProfile() { setSnoreProfile(null); }
export default store;
