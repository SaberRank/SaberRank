<script>
  import { onMount } from "svelte";
  import { api } from "./api.js";

  let path = location.pathname;
  let searchOpen = false;
  let accountOpen = false;
  let mobileOpen = false;
  let query = "";
  let me = null;
  let loading = false;
  let error = "";

  const nav = [
    ["/ranking/1", "♛", "Rankings"],
    ["/maps", "⌗", "Maps"],
    ["/events", "▣", "Events"],
    ["/clans", "♟", "Clans"]
  ];

  let players = [];
  let maps = [];
  let scores = [];
  let events = [];
  let clans = [];
  let stats = { scores: 0, players: 0, maps: 0, clans: 0 };

  function go(to) {
    history.pushState({}, "", to);
    path = to;
    mobileOpen = false;
    accountOpen = false;
    searchOpen = false;
    window.scrollTo({ top: 0, behavior: "smooth" });
    load();
  }

  async function load() {
    loading = true;
    error = "";
    try {
      if (path.startsWith("/ranking")) players = (await api("rankings?limit=25")).players || [];
      if (path === "/maps") maps = (await api("maps?limit=25")).maps || [];
      if (path === "/events") events = (await api("events?limit=25")).events || [];
      if (path === "/clans") clans = (await api("clans?limit=25")).clans || [];
      if (path.startsWith("/u/")) {
        const alias = decodeURIComponent(path.split("/")[2] || "");
        window.currentPlayer = await api(`players/${encodeURIComponent(alias)}`);
      }
      const s = await api("stats");
      stats = s.stats || stats;
      me = (await api("auth/me")).player || null;
    } catch (e) {
      error = e.message;
    } finally {
      loading = false;
    }
  }

  async function doSearch() {
    if (!query.trim()) return;
    try {
      const r = await api(`search?q=${encodeURIComponent(query.trim())}`);
      if (r.result?.type === "player") go(`/u/${encodeURIComponent(r.result.alias || r.result.name)}`);
      else if (r.result?.type === "map") go(`/map/${encodeURIComponent(r.result.id)}`);
      else error = "No SnoreSaber player or map matched that search.";
    } catch (e) { error = e.message; }
  }

  onMount(() => {
    const pop = () => { path = location.pathname; load(); };
    window.addEventListener("popstate", pop);
    load();
    return () => window.removeEventListener("popstate", pop);
  });
</script>

<header class="snore-nav">
  <div class="nav-inner">
    <a class="brand" href="/" on:click|preventDefault={() => go("/")}>
      <img src="/assets/snoresaber-logo-transparent.png" alt="SnoreSaber">
    </a>

    <button class="search" on:click={() => searchOpen = true} aria-label="Search">
      <svg viewBox="0 0 24 24"><path d="m21 21-4.35-4.35m2.1-5.4a7.5 7.5 0 1 1-15 0Z"/></svg>
      <span>Search players, maps, or scores...</span><kbd>Ctrl /</kbd>
    </button>

    <nav class="desktop-links">
      {#each nav as [href, icon, label]}
        <a class:active={path.startsWith(href.split("/")[1] === "ranking" ? "/ranking" : href)} href={href} on:click|preventDefault={() => go(href)}>
          <span class="icon">{icon}</span><span>{label}</span>
        </a>
      {/each}
    </nav>

    <div class="account-wrap">
      <button class="account-button" on:click={() => accountOpen = !accountOpen}>
        <span class="user-icon">♙</span><span>{me?.name || "Account"}</span><span class="chevron">⌄</span>
      </button>
      {#if accountOpen}
        <div class="account-menu">
          {#if me}
            <button on:click={() => go(`/u/${me.alias || me.name}`)}>Profile</button>
            <button on:click={() => go("/settings")}>Settings</button>
            <button on:click={async () => { await api("auth/logout", { method: "POST" }); me = null; accountOpen = false; }}>Sign out</button>
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
      {#each nav as [href, icon, label]}<button on:click={() => go(href)}>{icon} {label}</button>{/each}
      <button on:click={() => searchOpen = true}>⌕ Search</button>
    </div>
  {/if}
</header>

{#if searchOpen}
  <div class="overlay" on:click={() => searchOpen = false}>
    <form class="search-modal" on:submit|preventDefault={doSearch} on:click|stopPropagation>
      <input bind:value={query} autofocus placeholder="Search SnoreSaber..." />
      <button>Search</button>
    </form>
  </div>
{/if}

<main>
  {#if path === "/"}
    <section class="hero">
      <div class="hero-art"></div>
      <div class="hero-copy">
        <div class="eyebrow">BEAT SABER · SCORES · RANKINGS</div>
        <h1><span>snore</span><strong>saber</strong></h1>
        <p>Beat Saber scores, rankings, and player progress.</p>
        <div class="feature-row">
          <div class="feature"><div class="feature-icon">▮▮▮</div><div><b>Track</b><small>View scores, ranks, and improvement over time.</small></div></div>
          <div class="feature"><div class="feature-icon">⌗</div><div><b>Explore</b><small>Browse SnoreSaber maps and leaderboards.</small></div></div>
          <div class="feature"><div class="feature-icon">♟</div><div><b>Compete</b><small>See where you stand.</small></div></div>
        </div>
        <div class="hero-actions">
          <button class="primary" on:click={() => go("/ranking/1")}>View Rankings <span>→</span></button>
          <button class="secondary" on:click={() => go("/maps")}>⌗ &nbsp; Explore Maps</button>
        </div>
      </div>
    </section>

    <section class="stats">
      <div><span class="stat-icon pink">▮▮▮</span><div><b>{Number(stats.scores||0).toLocaleString()}</b><small>Scores</small></div><i></i></div>
      <div><span class="stat-icon purple">♟</span><div><b>{Number(stats.players||0).toLocaleString()}</b><small>Players</small></div><i></i></div>
      <div><span class="stat-icon blue">⌗</span><div><b>{Number(stats.maps||0).toLocaleString()}</b><small>Maps</small></div><i></i></div>
      <div><span class="stat-icon pink">♟</span><div><b>{Number(stats.clans||0).toLocaleString()}</b><small>Clans</small></div><i></i></div>
    </section>

    <section class="dashboard-grid">
      <div class="panel">
        <header><h2>Top Players</h2><button on:click={() => go("/ranking/1")}>View All →</button></header>
        {#if players.length}
          {#each players.slice(0,5) as player, i}
            <button class="row player-row" on:click={() => go(`/u/${encodeURIComponent(player.alias || player.name)}`)}>
              <span class="rank">{i+1}</span><span class="avatar">{player.name?.[0] || "?"}</span><span class="row-name">{player.name}</span><span>{player.country || ""}</span><span class="row-value">{Number(player.pp || 0).toFixed(2)} PP</span>
            </button>
          {/each}
        {:else}<div class="empty">{error ? "SnoreSaber API is not available yet." : "No SnoreSaber scores yet."}</div>{/if}
      </div>
      <div class="panel">
        <header><h2>Popular Maps</h2><button on:click={() => go("/maps")}>View All →</button></header>
        {#if maps.length}
          {#each maps.slice(0,5) as map, i}
            <button class="row map-row" on:click={() => go(`/map/${map.id}`)}>
              <span class="rank">{i+1}</span><span class="cover"></span><span class="map-info"><b>{map.songName}</b><small>{map.mapper || "Unknown mapper"}</small></span><span class="row-value">{Number(map.plays || 0).toLocaleString()}</span>
            </button>
          {/each}
        {:else}<div class="empty">No SnoreSaber maps yet.</div>{/if}
      </div>
      <div class="panel">
        <header><h2>Recent Scores</h2><button on:click={() => go("/scores")}>View All →</button></header>
        <div class="empty">Scores submitted to SnoreSaber will appear here.</div>
      </div>
    </section>

  {:else if path.startsWith("/ranking")}
    <section class="page-title"><div class="eyebrow">SNORE SABER RANKINGS</div><h1>Global Rankings</h1><p>The official SnoreSaber ranking, calculated only from SnoreSaber scores.</p></section>
    <section class="full-panel">
      <div class="toolbar"><span>{players.length} players loaded</span><button on:click={load}>↻ Refresh</button></div>
      {#if players.length}{#each players as p, i}<button class="large-row" on:click={() => go(`/u/${encodeURIComponent(p.alias || p.name)}`)}><span class="rank">#{i+1}</span><span class="avatar">{p.name?.[0]}</span><span class="row-name">{p.name}</span><span>{p.country || "—"}</span><strong>{Number(p.pp||0).toFixed(2)} PP</strong></button>{/each}{:else}<div class="empty">The SnoreSaber ranking is empty until scores are submitted.</div>{/if}
    </section>

  {:else if path === "/maps"}
    <section class="page-title"><div class="eyebrow">SNORE SABER MAPS</div><h1>Maps</h1><p>Ranked maps and map performance from SnoreSaber's own database.</p></section>
    <section class="cards">{#if maps.length}{#each maps as m}<button class="map-card" on:click={() => go(`/map/${m.id}`)}><div class="map-cover"></div><div><h2>{m.songName}</h2><p>{m.mapper || "Unknown mapper"} · {m.difficulty || "Unknown difficulty"}</p></div><strong>{Number(m.stars||0).toFixed(2)}★</strong></button>{/each}{:else}<Empty text="No maps have been ranked by SnoreSaber yet."/>{/if}</section>

  {:else if path === "/events"}
    <section class="page-title"><div class="eyebrow">COMMUNITY EVENTS</div><h1>Events</h1><p>SnoreSaber events live here. No external leaderboard data is imported.</p></section>
    <section class="cards">{#if events.length}{#each events as e}<article class="event-card"><b>{e.name}</b><p>{e.description || "SnoreSaber community event."}</p><small>{e.startsAt || "Date TBD"}</small></article>{/each}{:else}<Empty text="No SnoreSaber events have been created yet."/>{/if}</section>

  {:else if path === "/clans"}
    <section class="page-title"><div class="eyebrow">SNORE SABER CLANS</div><h1>Clans</h1><p>Community teams built on the SnoreSaber platform.</p></section>
    <section class="cards">{#if clans.length}{#each clans as c}<article class="event-card"><b>[{c.tag}] {c.name}</b><p>{c.memberCount || 0} members</p></article>{/each}{:else}<Empty text="No clans have been created yet."/>{/if}</section>

  {:else if path.startsWith("/u/")}
    {@const p = window.currentPlayer}
    <section class="page-title"><div class="eyebrow">PLAYER PROFILE</div><h1>{p?.name || "Player"}</h1><p>{p ? `${Number(p.pp||0).toFixed(2)} PP · Rank #${p.rank || "—"}` : "Player data could not be loaded."}</p></section>
    {#if p}<section class="profile-grid"><div class="full-panel profile-card"><div class="avatar big">{p.name?.[0]}</div><h2>{p.name}</h2><p>{p.country || "Country not set"}</p></div><div class="full-panel"><h2>Recent Scores</h2><Empty text="Score history will appear here as SnoreSaber receives scores."/></div></section>{/if}

  {:else}
    <section class="page-title"><div class="eyebrow">SNORE SABER</div><h1>Not found</h1><p>That SnoreSaber page doesn't exist.</p></section>
  {/if}

  {#if loading}<div class="loading">Loading SnoreSaber…</div>{/if}
</main>

<footer><strong>SNORE<span>SABER</span></strong><span>Independent Beat Saber leaderboard · Your scores, your ranking.</span></footer>

