<script>
  let page = "home";
  let search = "";

  const nav = [
    ["home", "Home"],
    ["rankings", "Rankings"],
    ["maps", "Maps"],
    ["events", "Events"],
    ["clans", "Clans"]
  ];

  const players = [
    { rank: 1, name: "Yawn", country: "CA", pp: "12,482.31", avatar: "YS" },
    { rank: 2, name: "Snore", country: "US", pp: "12,106.77", avatar: "SN" },
    { rank: 3, name: "Dreamy", country: "JP", pp: "11,984.12", avatar: "DR" },
    { rank: 4, name: "Sylph", country: "GB", pp: "11,771.50", avatar: "SY" },
    { rank: 5, name: "Dozer", country: "CA", pp: "11,504.82", avatar: "DO" }
  ];

  const maps = [
    { name: "Neon Dream", mapper: "SnoreSaber Team", stars: "10.42★", plays: "18.2K" },
    { name: "Sleepwalker", mapper: "yawn", stars: "9.87★", plays: "15.7K" },
    { name: "Moonlit Circuit", mapper: "Dreamy", stars: "8.91★", plays: "12.4K" }
  ];

  const recent = [
    { player: "Yawn", map: "Neon Dream", acc: "98.74%", pp: "412.8", ago: "2m" },
    { player: "Snore", map: "Sleepwalker", acc: "97.91%", pp: "389.2", ago: "6m" },
    { player: "Dreamy", map: "Moonlit Circuit", acc: "99.11%", pp: "375.6", ago: "11m" }
  ];
</script>

<svelte:head>
  <title>SnoreSaber</title>
  <meta name="description" content="The independent Beat Saber leaderboard." />
</svelte:head>

<header class="topbar">
  <a class="brand" href="/" on:click|preventDefault={() => page = "home"}>
    <img src="/assets/snoresaber-icon.svg" alt="" />
    <span>SNORE<span>SABER</span></span>
  </a>

  <nav>
    {#each nav as [key, label]}
      <button class:active={page === key} on:click={() => page = key}>{label}</button>
    {/each}
  </nav>

  <div class="right">
    <label class="search">
      <span>⌕</span>
      <input bind:value={search} placeholder="Search players, maps..." />
    </label>
    <a class="steam" href="/api/auth/steam">Sign in with Steam</a>
  </div>
</header>

<main>
  {#if page === "home"}
    <section class="hero">
      <div class="hero-copy">
        <div class="eyebrow">THE INDEPENDENT BEAT SABER LEADERBOARD</div>
        <h1>Play hard.<br /><em>Sleep easy.</em></h1>
        <p>SnoreSaber is a community-built leaderboard for competitive Beat Saber. Your scores. Your rank. Your community.</p>
        <div class="actions">
          <button class="primary" on:click={() => page = "rankings"}>View Rankings →</button>
          <button class="secondary" on:click={() => page = "maps"}>Explore Maps</button>
        </div>
      </div>
      <div class="hero-art">
        <div class="mascot">✦</div>
        <div class="glow"></div>
      </div>
    </section>

    <section class="stats">
      <div><strong>—</strong><span>PLAYERS</span></div>
      <div><strong>—</strong><span>RANKED MAPS</span></div>
      <div><strong>—</strong><span>SCORES SUBMITTED</span></div>
      <div><strong>100%</strong><span>SNOREPOWERED</span></div>
    </section>

    <section class="grid3">
      <article><b>TRACK</b><h2>Your scores.</h2><p>Submit scores directly to SnoreSaber and keep your complete competitive history in one place.</p></article>
      <article><b>EXPLORE</b><h2>Find your next grind.</h2><p>Browse SnoreSaber-ranked maps, player scores, leaderboards and performance stats.</p></article>
      <article><b>COMPETE</b><h2>Climb the ranks.</h2><p>Earn performance points, chase personal bests and compete for the top spot.</p></article>
    </section>

    <section class="columns">
      <div class="panel">
        <div class="panel-head"><h2>Top Players</h2><button on:click={() => page = "rankings"}>View all →</button></div>
        {#each players as p}
          <div class="player-row">
            <span class="rank">#{p.rank}</span><span class="avatar">{p.avatar}</span>
            <span class="player-name">{p.name}<small>{p.country}</small></span>
            <strong>{p.pp} PP</strong>
          </div>
        {/each}
      </div>

      <div class="panel">
        <div class="panel-head"><h2>Popular Maps</h2><button on:click={() => page = "maps"}>View all →</button></div>
        {#each maps as m}
          <div class="map-row">
            <div><strong>{m.name}</strong><small>{m.mapper}</small></div>
            <span>{m.stars}</span><em>{m.plays}</em>
          </div>
        {/each}
      </div>
    </section>

    <section class="panel recent">
      <div class="panel-head"><h2>Recent Scores</h2><button>Live feed →</button></div>
      {#each recent as s}
        <div class="score-row">
          <span class="player-name"><strong>{s.player}</strong><small>{s.map}</small></span>
          <span>{s.acc}</span><strong>{s.pp} PP</strong><em>{s.ago}</em>
        </div>
      {/each}
    </section>
  {:else if page === "rankings"}
    <section class="page"><div class="eyebrow">SNOREPOWERED RANKINGS</div><h1>Global Rankings</h1><p class="lead">The official SnoreSaber player ranking, calculated from SnoreSaber scores.</p>
      <div class="panel">{#each players as p}<div class="player-row"><span class="rank">#{p.rank}</span><span class="avatar">{p.avatar}</span><span class="player-name">{p.name}<small>{p.country}</small></span><strong>{p.pp} PP</strong></div>{/each}</div>
    </section>
  {:else if page === "maps"}
    <section class="page"><div class="eyebrow">RANKED MAPS</div><h1>Maps</h1><p class="lead">Maps ranked by SnoreSaber, with scores stored in our own database.</p>
      <div class="cards">{#each maps as m}<article class="map-card"><div class="cover"></div><h2>{m.name}</h2><p>{m.mapper}</p><strong>{m.stars}</strong><small>{m.plays} plays</small></article>{/each}</div>
    </section>
  {:else}
    <section class="page"><div class="eyebrow">SNORE SABER</div><h1>{nav.find(x => x[0] === page)?.[1] ?? "SnoreSaber"}</h1><p class="lead">This SnoreSaber section is ready for its database-backed implementation.</p><div class="empty">Nothing here yet — and that's intentional. This is SnoreSaber's own data layer, not imported leaderboard data.</div></section>
  {/if}
</main>

<footer><strong>SNORE<span>SABER</span></strong><span>Independent Beat Saber leaderboard.</span></footer>