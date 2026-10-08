<script>
  import { onMount } from 'svelte';
  import { api } from './api.js';

  let path = location.pathname || '/';
  let searchOpen = false;
  let accountOpen = false;
  let mobileOpen = false;
  let query = '';
  let me = null;
  let currentPlayer = null;
  let currentMap = null;
  let loading = false;
  let error = '';
  let apiOnline = true;

  let players = [];
  let maps = [];
  let scores = [];
  let events = [];
  let clans = [];
  let stats = { scores: 0, players: 0, maps: 0, clans: 0 };

  const nav = [
    ['/ranking/1', 'rank', 'Rankings'],
    ['/maps', 'map', 'Maps'],
    ['/events', 'calendar', 'Events'],
    ['/clans', 'users', 'Clans']
  ];

  const icon = {
    search: '<svg viewBox="0 0 24 24"><circle cx="11" cy="11" r="6.5"/><path d="m16 16 5 5"/></svg>',
    rank: '<svg viewBox="0 0 24 24"><path d="M7 20V9h4v11M13 20V4h4v16M3 20h18"/></svg>',
    map: '<svg viewBox="0 0 24 24"><path d="M4 5.5 9 3l6 3 5-2.5v15L15 21l-6-3-5 2.5zM9 3v15M15 6v15"/></svg>',
    calendar: '<svg viewBox="0 0 24 24"><rect x="3" y="5" width="18" height="16" rx="2"/><path d="M8 3v4M16 3v4M3 10h18"/></svg>',
    users: '<svg viewBox="0 0 24 24"><circle cx="9" cy="8" r="3"/><circle cx="17" cy="10" r="2.5"/><path d="M3.5 20c.6-4 2.4-6 5.5-6s4.9 2 5.5 6M14 15c3.2-.8 5.4.8 6.5 5"/></svg>',
    trophy: '<svg viewBox="0 0 24 24"><path d="M8 4h8v5c0 3-1.8 5-4 5s-4-2-4-5zM8 6H4v2c0 2 1.5 3.5 4 3.5M16 6h4v2c0 2-1.5 3.5-4 3.5M12 14v4M8 21h8M9 18h6"/></svg>',
    activity: '<svg viewBox="0 0 24 24"><path d="M3 12h4l2-7 4 14 2-7h6"/></svg>',
    clock: '<svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="8.5"/><path d="M12 7v5l3 2"/></svg>'
  };

  function go(to) {
    history.pushState({}, '', to);
    path = to;
    mobileOpen = false;
    accountOpen = false;
    searchOpen = false;
    window.scrollTo({ top: 0, behavior: 'smooth' });
    load();
  }

  function navActive(href) {
    if (href.startsWith('/ranking')) return path.startsWith('/ranking');
    return path.startsWith(href);
  }

  async function load() {
    loading = true;
    error = '';
    apiOnline = true;
    try {
      const jobs = [api('stats'), api('rankings?limit=50'), api('maps?limit=50'), api('scores?limit=25'), api('events?limit=25'), api('clans?limit=25'), api('auth/me')];
      const [s, r, m, sc, e, c, account] = await Promise.all(jobs);
      stats = s.stats || stats;
      players = r.players || [];
      maps = m.maps || [];
      scores = sc.scores || [];
      events = e.events || [];
      clans = c.clans || [];
      me = account.player || null;

      if (path.startsWith('/u/')) {
        const alias = decodeURIComponent(path.slice(3));
        currentPlayer = await api(`players/${encodeURIComponent(alias)}`);
      } else currentPlayer = null;

      if (path.startsWith('/map/')) {
        const id = decodeURIComponent(path.slice(5));
        currentMap = await api(`maps/${encodeURIComponent(id)}`);
      } else currentMap = null;
    } catch (e) {
      apiOnline = false;
      error = e?.message || 'The SnoreSaber API could not be reached.';
    } finally {
      loading = false;
    }
  }

  async function doSearch() {
    const term = query.trim();
    if (!term) return;
    try {
      const r = await api(`search?q=${encodeURIComponent(term)}`);
      if (r.result?.type === 'player') go(`/u/${encodeURIComponent(r.result.alias || r.result.name)}`);
      else if (r.result?.type === 'map') go(`/map/${encodeURIComponent(r.result.id)}`);
      else error = 'No SnoreSaber player or map matched that search.';
      searchOpen = false;
    } catch (e) { error = e.message; }
  }

  function shortcut(event) {
    if ((event.ctrlKey || event.metaKey) && event.key === '/') {
      event.preventDefault();
      searchOpen = true;
      setTimeout(() => document.querySelector('.search-modal input')?.focus(), 0);
    }
    if (event.key === 'Escape') {
      searchOpen = false;
      accountOpen = false;
    }
  }

  function initials(name) {
    return (name || '?').slice(0, 2).toUpperCase();
  }

  function number(value) {
    return Number(value || 0).toLocaleString();
  }

  function pp(value) {
    return Number(value || 0).toFixed(2);
  }

  function ago(value) {
    if (!value) return 'just now';
    const seconds = Math.max(0, Math.floor((Date.now() - new Date(value).getTime()) / 1000));
    if (seconds < 60) return `${seconds}s ago`;
    if (seconds < 3600) return `${Math.floor(seconds / 60)}m ago`;
    if (seconds < 86400) return `${Math.floor(seconds / 3600)}h ago`;
    return `${Math.floor(seconds / 86400)}d ago`;
  }

  onMount(() => {
    const pop = () => { path = location.pathname; load(); };
    window.addEventListener('popstate', pop);
    window.addEventListener('keydown', shortcut);
    load();
    return () => {
      window.removeEventListener('popstate', pop);
      window.removeEventListener('keydown', shortcut);
    };
  });
</script>

<header class="topbar">
  <div class="nav-inner">
    <a class="brand" href="/" on:click|preventDefault={() => go('/')} aria-label="SnoreSaber home">
      <img src="/assets/snoresaber-logo-transparent.png" alt="SnoreSaber" />
    </a>

    <button class="search-trigger" on:click={() => searchOpen = true} aria-label="Search">
      {@html icon.search}<span>Search players, maps, or scores...</span><kbd>Ctrl /</kbd>
    </button>

    <nav class="nav-links">
      {#each nav as [href, ico, label]}
        <a class:active={navActive(href)} href={href} on:click|preventDefault={() => go(href)}>
          {@html icon[ico]}<span>{label}</span>
        </a>
      {/each}
    </nav>

    <div class="account-wrap">
      <button class="account" on:click={() => accountOpen = !accountOpen}>
        {@html icon.users}<span>{me?.name || 'Account'}</span><b>⌄</b>
      </button>
      {#if accountOpen}
        <div class="account-menu">
          {#if me}
            <button on:click={() => go(`/u/${encodeURIComponent(me.alias || me.name)}`)}>Profile</button>
            <button on:click={() => go('/settings')}>Account settings</button>
            <button on:click={async () => { await api('auth/logout', { method: 'POST' }); me = null; accountOpen = false; }}>Sign out</button>
          {:else}
            <a href="/api/auth/steam">Sign in with Steam</a>
          {/if}
        </div>
      {/if}
    </div>
    <button class="mobile-toggle" on:click={() => mobileOpen = !mobileOpen}>☰</button>
  </div>

  {#if mobileOpen}
    <div class="mobile-menu">
      {#each nav as [href, ico, label]}<button on:click={() => go(href)}>{@html icon[ico]} {label}</button>{/each}
      <button on:click={() => searchOpen = true}>{@html icon.search} Search</button>
    </div>
  {/if}
</header>

{#if searchOpen}
  <div class="overlay" on:click={() => searchOpen = false}>
    <form class="search-modal" on:submit|preventDefault={doSearch} on:click|stopPropagation>
      {@html icon.search}<input class="search-input" bind:value={query} autofocus placeholder="Search SnoreSaber..." /><kbd>Enter</kbd>
    </form>
  </div>
{/if}

<main>
  {#if !apiOnline}
    <div class="api-warning"><span>●</span> SnoreSaber API is offline or not configured. The interface is running, but live leaderboard data is unavailable.</div>
  {/if}

  {#if path === '/'}
    <section class="hero">
      <div class="hero-image"></div>
      <div class="hero-shade"></div>
      <div class="hero-copy">
        <div class="eyebrow">BEAT SABER · SCORES · RANKINGS</div>
        <h1><span>snore</span><strong>saber</strong></h1>
        <p>Beat Saber scores, rankings, and player progress — built from the ground up.</p>
        <div class="hero-features">
          <div class="feature"><span class="feature-symbol">{@html icon.activity}</span><div><b>Track</b><small>Scores, ranks, accuracy, and progress.</small></div></div>
          <div class="feature"><span class="feature-symbol">{@html icon.map}</span><div><b>Explore</b><small>Browse SnoreSaber-ranked maps.</small></div></div>
          <div class="feature"><span class="feature-symbol">{@html icon.trophy}</span><div><b>Compete</b><small>Climb the SnoreSaber ranking.</small></div></div>
        </div>
        <div class="hero-actions">
          <button class="primary" on:click={() => go('/ranking/1')}>View Rankings <span>→</span></button>
          <button class="secondary" on:click={() => go('/maps')}>{@html icon.map} Explore Maps</button>
        </div>
      </div>
      <div class="hero-tagline"><i></i><span>PLAY.</span><span>IMPROVE.</span><span>CLIMB.</span></div>
    </section>

    <section class="stats">
      <div class="stat-card"><span class="stat-icon">{@html icon.activity}</span><div><b>{number(stats.scores)}</b><small>Scores</small></div><span class="spark spark-a"></span></div>
      <div class="stat-card"><span class="stat-icon">{@html icon.users}</span><div><b>{number(stats.players)}</b><small>Players</small></div><span class="spark spark-b"></span></div>
      <div class="stat-card"><span class="stat-icon">{@html icon.map}</span><div><b>{number(stats.maps)}</b><small>Maps</small></div><span class="spark spark-c"></span></div>
      <div class="stat-card"><span class="stat-icon">{@html icon.users}</span><div><b>{number(stats.clans)}</b><small>Clans</small></div><span class="spark spark-d"></span></div>
    </section>

    <section class="dashboard-grid">
      <article class="panel"><header><div>{@html icon.trophy}<h2>Top Players</h2></div><button on:click={() => go('/ranking/1')}>View All →</button></header>
        {#if players.length}
          {#each players.slice(0, 5) as player, i}
            <button class="list-row" on:click={() => go(`/u/${encodeURIComponent(player.alias || player.name)}`)}><span class="rank">{i + 1}</span><span class="avatar">{initials(player.name)}</span><span class="name">{player.name}</span><span class="country">{player.country || '—'}</span><strong>{pp(player.pp)}</strong></button>
          {/each}
        {:else}<div class="empty">No ranked players yet.</div>{/if}
      </article>

      <article class="panel"><header><div>{@html icon.map}<h2>Popular Maps</h2></div><button on:click={() => go('/maps')}>View All →</button></header>
        {#if maps.length}
          {#each maps.slice(0, 5) as map, i}
            <button class="list-row map-list" on:click={() => go(`/map/${encodeURIComponent(map.id)}`)}><span class="rank">{i + 1}</span><span class="map-thumb"></span><span class="map-name"><b>{map.songName}</b><small>{map.mapper || 'Unknown mapper'}</small></span><strong>{number(map.plays)}</strong></button>
          {/each}
        {:else}<div class="empty">No SnoreSaber-ranked maps yet.</div>{/if}
      </article>

      <article class="panel"><header><div>{@html icon.clock}<h2>Recent Scores</h2></div><button on:click={() => go('/scores')}>View All →</button></header>
        {#if scores.length}
          {#each scores.slice(0, 5) as score}
            <button class="list-row score-list" on:click={() => score.playerAlias && go(`/u/${encodeURIComponent(score.playerAlias)}`)}><span class="avatar small">{initials(score.playerName)}</span><span class="score-name"><b>{score.playerName || 'Unknown'}</b><small>{score.songName || 'Unknown map'}</small></span><strong>{Number(score.accuracy || 0).toFixed(2)}%</strong><span class="time">{ago(score.playedAt)}</span></button>
          {/each}
        {:else}<div class="empty">Scores submitted to SnoreSaber will appear here.</div>{/if}
      </article>
    </section>

  {:else if path.startsWith('/ranking')}
    <section class="page-title"><div class="eyebrow">SNORE SABER RANKINGS</div><h1>Global Rankings</h1><p>Every rank is calculated from SnoreSaber's own score database.</p></section>
    <section class="full-panel"><div class="toolbar"><span><b>{players.length}</b> players loaded</span><button on:click={load}>↻ Refresh</button></div>
      {#if players.length}{#each players as p, i}<button class="large-row" on:click={() => go(`/u/${encodeURIComponent(p.alias || p.name)}`)}><span class="rank">#{i + 1}</span><span class="avatar">{initials(p.name)}</span><span class="name">{p.name}</span><span class="country">{p.country || '—'}</span><span class="plays">{number(p.playCount)} scores</span><strong>{pp(p.pp)} PP</strong></button>{/each}{:else}<div class="empty">The SnoreSaber ranking is empty until scores are submitted.</div>{/if}
    </section>

  {:else if path === '/maps'}
    <section class="page-title"><div class="eyebrow">SNORE SABER MAPS</div><h1>Ranked Maps</h1><p>Maps ranked and tracked by SnoreSaber — no ScoreSaber or BeatLeader data is required.</p></section>
    <section class="map-grid">{#if maps.length}{#each maps as m}<button class="map-card" on:click={() => go(`/map/${encodeURIComponent(m.id)}`)}><div class="large-cover"></div><div class="map-card-body"><span class="map-diff">{m.difficulty || 'Ranked'}</span><h2>{m.songName}</h2><p>{m.mapper || 'Unknown mapper'}</p><div class="map-meta"><span>{number(m.plays)} plays</span><strong>{Number(m.stars || 0).toFixed(2)}★</strong></div></div></button>{/each}{:else}<div class="empty big">No maps have been ranked by SnoreSaber yet.</div>{/if}</section>

  {:else if path.startsWith('/map/')}
    <section class="page-title">{#if currentMap}<div class="eyebrow">MAP LEADERBOARD</div><h1>{currentMap.songName}</h1><p>{currentMap.mapper || 'Unknown mapper'} · {currentMap.difficulty || 'Ranked'} · {Number(currentMap.stars || 0).toFixed(2)}★</p>{:else}<div class="eyebrow">MAP</div><h1>Loading map…</h1>{/if}</section>
    <section class="full-panel">{#if currentMap?.scores?.length}{#each currentMap.scores as s, i}<div class="large-row static"><span class="rank">#{i + 1}</span><span class="avatar">{initials(s.playerName)}</span><span class="name">{s.playerName}</span><span class="plays">{Number(s.score).toLocaleString()}</span><span class="country">{Number(s.accuracy).toFixed(2)}%</span><strong>{pp(s.pp)} PP</strong></div>{/each}{:else}<div class="empty big">No scores have been submitted for this map yet.</div>{/if}</section>

  {:else if path === '/scores'}
    <section class="page-title"><div class="eyebrow">SCORE FEED</div><h1>Recent Scores</h1><p>The latest scores received by SnoreSaber.</p></section>
    <section class="full-panel">{#if scores.length}{#each scores as s}<div class="large-row static"><span class="avatar">{initials(s.playerName)}</span><span class="name">{s.playerName || 'Unknown'}</span><span class="map-name"><b>{s.songName || 'Unknown map'}</b><small>{s.difficulty || ''}</small></span><span class="country">{Number(s.accuracy || 0).toFixed(2)}%</span><strong>{pp(s.pp)} PP</strong><span class="time">{ago(s.playedAt)}</span></div>{/each}{:else}<div class="empty big">No scores have been submitted yet.</div>{/if}</section>

  {:else if path === '/events'}
    <section class="page-title"><div class="eyebrow">COMMUNITY</div><h1>Events</h1><p>SnoreSaber tournaments and community events.</p></section>
    <section class="card-grid">{#if events.length}{#each events as e}<article class="info-card"><span class="card-kicker">EVENT</span><h2>{e.name}</h2><p>{e.description || 'SnoreSaber community event.'}</p><small>{e.startsAt || 'Date TBD'}</small></article>{/each}{:else}<div class="empty big">No SnoreSaber events have been created yet.</div>{/if}</section>

  {:else if path === '/clans'}
    <section class="page-title"><div class="eyebrow">COMMUNITY</div><h1>Clans</h1><p>Teams built around the SnoreSaber community.</p></section>
    <section class="card-grid">{#if clans.length}{#each clans as c}<article class="info-card"><span class="clan-tag">[{c.tag}]</span><h2>{c.name}</h2><p>{number(c.memberCount)} members</p></article>{/each}{:else}<div class="empty big">No clans have been created yet.</div>{/if}</section>

  {:else if path.startsWith('/u/')}
    <section class="profile-head">{#if currentPlayer}<div class="profile-avatar">{initials(currentPlayer.name)}</div><div><div class="eyebrow">PLAYER PROFILE</div><h1>{currentPlayer.name}</h1><p>{currentPlayer.country || 'Country not set'} · #{currentPlayer.rank || '—'} · {pp(currentPlayer.pp)} PP</p></div>{:else}<div class="profile-avatar">?</div><div><div class="eyebrow">PLAYER PROFILE</div><h1>Loading…</h1></div>{/if}</section>
    {#if currentPlayer}<section class="profile-stats"><div><b>{pp(currentPlayer.pp)}</b><small>Performance</small></div><div><b>#{currentPlayer.rank || '—'}</b><small>Global rank</small></div><div><b>{number(currentPlayer.playCount)}</b><small>Scores</small></div><div><b>{number(currentPlayer.totalScore)}</b><small>Total score</small></div></section>{/if}
    <section class="full-panel"><header class="section-header"><h2>Recent Scores</h2></header><div class="empty big">Player score history will appear here as SnoreSaber receives scores.</div></section>

  {:else if path === '/settings'}
    <section class="page-title"><div class="eyebrow">ACCOUNT</div><h1>Settings</h1><p>Manage your SnoreSaber account.</p></section>
    <section class="settings-card">{#if me}<div class="setting-row"><div><b>Signed in</b><small>{me.name}</small></div><button class="secondary small-btn" on:click={async () => { await api('auth/logout', { method: 'POST' }); me = null; go('/'); }}>Sign out</button></div>{:else}<div class="setting-row"><div><b>Not signed in</b><small>Sign in with Steam to create your SnoreSaber account.</small></div><a class="primary small-btn" href="/api/auth/steam">Sign in with Steam</a></div>{/if}</section>

  {:else}
    <section class="page-title"><div class="eyebrow">SNORE SABER</div><h1>Not found</h1><p>That SnoreSaber page doesn't exist.</p><button class="primary back-btn" on:click={() => go('/')}>Back home</button></section>
  {/if}
</main>

<footer><strong>SNORE<span>SABER</span></strong><span>Independent Beat Saber leaderboard · Your scores. Your ranking.</span></footer>
