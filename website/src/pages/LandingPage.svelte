<script>
	import {onMount} from 'svelte';
	import {navigate} from 'svelte-routing';
	import snoreAccount from '../stores/snore-account';

	let data = null;
	let loading = true;
	let error = '';

	const go = path => navigate(path);
	const format = value => Number(value || 0).toLocaleString('en-US');
	const pp = value => Number(value || 0).toLocaleString('en-US', {minimumFractionDigits:2, maximumFractionDigits:2});
	const acc = value => `${Number(value || 0).toFixed(2)}%`;
	const relative = value => {
		if (!value) return '—';
		const date = new Date(value);
		if (Number.isNaN(date.getTime())) return value;
		const seconds = Math.max(0, Math.floor((Date.now() - date.getTime()) / 1000));
		if (seconds < 60) return `${seconds}s ago`;
		const minutes = Math.floor(seconds / 60);
		if (minutes < 60) return `${minutes}m ago`;
		const hours = Math.floor(minutes / 60);
		return `${hours}h ago`;
	};

	async function load() {
		loading = true;
		error = '';
		try {
			const response = await fetch('/api/scoresaber-home');
			if (!response.ok) throw new Error(`Live data returned ${response.status}`);
			data = await response.json();
		} catch (e) {
			error = e?.message || 'Unable to load live data.';
		} finally { loading = false; }
	}

	onMount(load);
</script>

<svelte:head><title>snore saber</title></svelte:head>

<div class="snore-home">
	<section class="hero">
		<div class="hero-art"></div><div class="hero-shade"></div>
		<div class="hero-copy">
			<div class="eyebrow">BEAT SABER · SCORES · RANKINGS</div>
			<h1><span>snore</span> <strong>saber</strong></h1>
			<p>ScoreSaber-powered data with a SnoreSaber interface, profile layer, and features of our own.</p>
			<div class="feature-row">
				<div class="feature"><span class="feature-icon"><svg viewBox="0 0 24 24"><path d="M5 19V9m7 10V5m7 14v-7"/></svg></span><div><b>Track</b><small>Ranks, PP, scores and player progress.</small></div></div>
				<div class="feature"><span class="feature-icon"><svg viewBox="0 0 24 24"><circle cx="11" cy="11" r="6.5"/><path d="m16 16 5 5"/></svg></span><div><b>Explore</b><small>Find maps, players and leaderboards.</small></div></div>
				<div class="feature"><span class="feature-icon"><svg viewBox="0 0 24 24"><path d="m12 3 2.4 5 5.6.8-4 3.9.9 5.5-4.9-2.6-4.9 2.6.9-5.5-4-3.9 5.6-.8L12 3Z"/></svg></span><div><b>Compete</b><small>Compare yourself with the ranked field.</small></div></div>
			</div>
			<div class="hero-actions"><button class="primary" on:click={() => go('/ranking/1')}>View Rankings <span>→</span></button><button class="secondary" on:click={() => go('/maps')}>Explore Maps</button></div>
		</div>
	</section>

	{#if loading}
		<div class="loading-card"><span class="spinner"></span><span>Loading live ScoreSaber data…</span></div>
	{:else if error}
		<div class="error-card"><div><b>Live data is temporarily unavailable.</b><small>{error}</small></div><button on:click={load}>Retry</button></div>
	{:else}
		<section class="live-strip"><span class="live-dot"></span><span>LIVE DATA</span><small>ScoreSaber · refreshed {relative(data.updatedAt)}</small></section>

		<section class="stats">
			<div><span class="stat-icon pink"><svg viewBox="0 0 24 24"><circle cx="9" cy="8" r="3"/><path d="M3.5 20c.5-4 2.3-6 5.5-6s5 2 5.5 6M15 7.5c2.7-.5 4.8 1.2 5.5 4"/></svg></span><div><b>{format(data.stats.players)}</b><small>Players</small></div><i></i></div>
			<div><span class="stat-icon purple"><svg viewBox="0 0 24 24"><rect x="4" y="4" width="16" height="16" rx="2"/><path d="M4 10h16M10 4v16"/></svg></span><div><b>{format(data.stats.maps)}</b><small>Maps in index</small></div><i></i></div>
			<div><span class="stat-icon blue"><svg viewBox="0 0 24 24"><path d="M4 18 9 12l4 3 7-9"/><path d="M4 20h16"/></svg></span><div><b>{data.players[0] ? pp(data.players[0].pp) : '—'}</b><small>Top PP</small></div><i></i></div>
			<div><span class="stat-icon pink"><svg viewBox="0 0 24 24"><path d="M12 3v18M5 8h10a4 4 0 1 1 0 8H7"/></svg></span><div><b>{data.players[0]?.rank ? `#${format(data.players[0].rank)}` : '—'}</b><small>Current #1</small></div><i></i></div>
		</section>

		<section class="dashboard-grid">
			<div class="panel">
				<header><h2>Top Players</h2><button on:click={() => go('/ranking/1')}>View All →</button></header>
				{#each data.players as player, i}
					<button class="row player-row" on:click={() => player.id && go(`/u/${player.id}`)}>
						<span class="rank">{player.rank || i + 1}</span>
						{#if player.avatar}<img class="avatar-image" src={player.avatar} alt="" />{:else}<span class="avatar">{player.name[0]}</span>{/if}
						<span class="row-name gradient-name">{player.name}</span><span class="country">{player.country}</span><span class="row-value">{pp(player.pp)}</span>
					</button>
				{/each}
			</div>

			<div class="panel">
				<header><h2>Popular Maps</h2><button on:click={() => go('/maps')}>View All →</button></header>
				{#each data.maps as map, i}
					<button class="row map-row" on:click={() => go('/maps')}>
						<span class="rank">{i + 1}</span>{#if map.cover}<img src={map.cover} alt="" />{:else}<span class="cover-fallback">♪</span>{/if}
						<span class="map-info"><b>{map.name}{map.subName ? ` ${map.subName}` : ''}</b><small>{map.mapper} · {map.stars ? `${map.stars.toFixed(2)}★` : map.status}</small></span><span class="row-value">{format(map.plays)}</span>
					</button>
				{/each}
			</div>

			<div class="panel">
				<header><h2>Recent Scores</h2><button on:click={() => go('/scores/1')}>View All →</button></header>
				{#if data.scores.length}
					{#each data.scores as score}
						<button class="row score-row" on:click={() => score.playerId && go(`/u/${score.playerId}`)}>
							<span class="avatar">{score.playerName[0]}</span><span class="row-name gradient-name">{score.playerName}</span><span class="map-info"><b>{score.mapName}</b><small>{score.pp ? `${pp(score.pp)} PP` : `Score ${format(score.score)}`}</small></span><span class="acc">{acc(score.accuracy)}</span><span class="time">{relative(score.timeset)}</span>
						</button>
					{/each}
				{:else}<div class="empty">No recent score entries were returned by the live API.</div>{/if}
			</div>
		</section>

		{#if $snoreAccount?.scoresaberId}
			<section class="connected"><div><span class="live-dot"></span> Connected profile</div><button on:click={() => go(`/u/${$snoreAccount.scoresaberId}`)}><strong class="gradient-name">{$snoreAccount.displayName}</strong><span>Steam-linked account · ScoreSaber profile connected</span> →</button></section>
		{:else}
			<section class="connect-banner"><div><span class="eyebrow">YOUR PROFILE</span><h3>Make SnoreSaber yours.</h3><p>Connect a public ScoreSaber profile and keep your own SnoreSaber profile shortcut.</p></div><button on:click={() => go('/signin')}>Connect profile →</button></section>
		{/if}
	{/if}
</div>

<style>
	:global(body){margin:0;background:#07080f}.snore-home{max-width:1600px;margin:0 auto;padding:0 36px 42px;color:#f7f4fb}.hero{min-height:455px;position:relative;overflow:hidden;border-bottom:1px solid rgba(255,255,255,.05)}.hero-art{position:absolute;inset:0 0 0 39%;background:url('/assets/snoresaber-hero-art.png') right center/cover no-repeat}.hero-shade{position:absolute;inset:0;background:linear-gradient(90deg,#090a11 0%,rgba(9,10,17,.94) 29%,rgba(9,10,17,.52) 57%,rgba(9,10,17,.06) 100%)}.hero-copy{position:relative;z-index:2;width:65%;padding:76px 0 30px 92px}.eyebrow{color:#ff91d4;font-size:12px;font-weight:800;letter-spacing:.27em;margin-bottom:17px}h1{margin:0;font-size:76px;line-height:.95;letter-spacing:.02em;font-weight:300;font-family:Arial,sans-serif}h1 span{color:#f8f8fb}h1 strong{font-weight:400;background:linear-gradient(90deg,#f179bf,#b37cff);-webkit-background-clip:text;background-clip:text;color:transparent}.hero-copy>p{margin:22px 0 32px;color:#b7b6ca;font-size:22px;max-width:720px;line-height:1.4}.feature-row{display:flex;gap:12px}.feature{min-width:0;width:31%;display:flex;align-items:center;gap:13px;padding:13px 15px;border:1px solid rgba(255,255,255,.08);border-radius:16px;background:rgba(11,12,21,.56);backdrop-filter:blur(12px)}.feature-icon{width:45px;height:45px;flex:0 0 45px;display:grid;place-items:center;border-radius:13px;color:#ff8dd0;background:linear-gradient(135deg,rgba(242,79,177,.18),rgba(132,78,255,.11));border:1px solid rgba(240,110,194,.2)}.feature-icon svg{width:23px;height:23px;stroke:currentColor;fill:none;stroke-width:1.7;stroke-linecap:round;stroke-linejoin:round}.feature b{display:block;color:#f49bd4;font-size:15px;margin-bottom:5px}.feature small{display:block;color:#b2b0c2;line-height:1.3;font-size:12px}.hero-actions{display:flex;gap:14px;margin-top:31px}.hero-actions button{height:53px;padding:0 31px;border-radius:14px;font-size:15px;font-weight:800;cursor:pointer}.primary{border:0;background:linear-gradient(90deg,#ef72bb,#9b79ff);color:#170d18;box-shadow:0 12px 35px rgba(237,102,186,.22)}.primary span{font-size:23px;margin-left:16px}.secondary{border:1px solid #535269;background:rgba(12,13,22,.72);color:#f4f2f8}.live-strip{height:35px;margin:0 14px 8px;display:flex;align-items:center;gap:9px;color:#b7b4c3;font-size:10px;letter-spacing:.15em}.live-strip small{letter-spacing:0;color:#777688;margin-left:5px}.live-dot{display:inline-block;width:7px;height:7px;border-radius:50%;background:#ef72bb;box-shadow:0 0 12px #ef72bb}.stats{display:grid;grid-template-columns:repeat(4,1fr);gap:15px;margin:0 14px 17px}.stats>div{min-height:76px;border:1px solid rgba(255,255,255,.08);background:linear-gradient(135deg,rgba(19,20,31,.94),rgba(11,12,20,.94));border-radius:15px;display:flex;align-items:center;padding:0 20px;gap:14px;box-sizing:border-box}.stat-icon{width:34px;font-size:25px;text-align:center}.stat-icon svg{width:25px;height:25px;stroke:currentColor;fill:none;stroke-width:1.5;stroke-linecap:round;stroke-linejoin:round}.stat-icon.pink{color:#ff83ce}.stat-icon.purple{color:#a894ff}.stat-icon.blue{color:#7ea2ff}.stats b{display:block;font-size:18px}.stats small{display:block;color:#9e9cac;font-size:12px;margin-top:3px}.stats i{margin-left:auto;width:145px;height:24px;border-bottom:2px solid #e983c7;border-radius:50%;transform:skewY(-7deg);opacity:.9}.dashboard-grid{display:grid;grid-template-columns:1.03fr 1fr 1fr;gap:22px;margin:0 14px}.panel{border:1px solid rgba(255,255,255,.08);background:linear-gradient(145deg,rgba(17,18,28,.97),rgba(10,11,19,.97));border-radius:15px;padding:15px 18px 12px;overflow:hidden}.panel header{display:flex;align-items:center;justify-content:space-between;padding:4px 4px 13px}.panel h2{font-size:16px;margin:0}.panel header button{border:0;background:none;color:#ef7dc6;cursor:pointer;font-weight:700}.row{width:100%;border:0;color:#f2eff7;background:transparent;display:flex;align-items:center;gap:10px;height:42px;padding:0 7px;border-radius:9px;text-align:left;cursor:pointer}.row:hover{background:rgba(255,255,255,.045)}.rank{width:20px;color:#dedbe6;text-align:center;font-size:12px}.avatar,.avatar-image{width:29px;height:29px;flex:0 0 29px;border-radius:50%;object-fit:cover}.avatar{display:grid;place-items:center;background:linear-gradient(135deg,#ff6eb9,#8e68ff);color:#fff;font-size:11px;font-weight:900}.row-name{font-size:13px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}.gradient-name{background:linear-gradient(90deg,#fff,#f28acb,#a889ff);-webkit-background-clip:text;background-clip:text;color:transparent}.country{font-size:10px;color:#7f7d8d}.row-value{margin-left:auto;color:#dedbe6;font-size:13px;white-space:nowrap}.map-row img,.cover-fallback{width:29px;height:29px;object-fit:cover;border-radius:6px}.cover-fallback{display:grid;place-items:center;background:linear-gradient(135deg,#302047,#172047);color:#f18bc9}.map-info{min-width:0;flex:1}.map-info b,.map-info small{display:block;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}.map-info b{font-size:12px}.map-info small{font-size:10px;color:#858492;margin-top:2px}.acc{color:#e98bc7;font-size:12px}.time{color:#858492;font-size:10px;white-space:nowrap}.empty{color:#777688;font-size:12px;padding:20px 8px}.loading-card,.error-card{margin:20px 14px;padding:25px;border:1px solid rgba(255,255,255,.08);border-radius:15px;background:#10111a;display:flex;align-items:center;justify-content:center;gap:12px;color:#aaa8b8}.spinner{width:17px;height:17px;border:2px solid rgba(255,255,255,.15);border-top-color:#ef72bb;border-radius:50%;animation:spin .8s linear infinite}@keyframes spin{to{transform:rotate(360deg)}}.error-card{justify-content:space-between}.error-card small{display:block;color:#777688;margin-top:5px}.error-card button,.connect-banner button,.connected button{border:0;border-radius:10px;background:linear-gradient(90deg,#ef72bb,#9279ff);color:#170d18;font-weight:800;padding:11px 17px;cursor:pointer}.connected,.connect-banner{margin:18px 14px 0;border:1px solid rgba(255,255,255,.08);border-radius:16px;background:linear-gradient(135deg,rgba(19,20,31,.95),rgba(12,12,21,.95));padding:18px 20px;display:flex;align-items:center;justify-content:space-between}.connected>div{font-size:12px;color:#aaa8b7;display:flex;align-items:center;gap:8px}.connected button{background:rgba(255,255,255,.04);border:1px solid rgba(255,255,255,.09);color:#ddd;display:flex;gap:14px;align-items:center}.connected button span{color:#777688;font-weight:500}.connect-banner h3{margin:3px 0;font-size:22px}.connect-banner p{margin:0;color:#888797;font-size:12px}.connect-banner .eyebrow{margin:0;font-size:9px}
	@media(max-width:1100px){.snore-home{padding:0 18px 30px}.hero-copy{padding-left:35px;width:70%}h1{font-size:62px}.feature-row{flex-wrap:wrap}.feature{width:calc(50% - 6px)}.dashboard-grid{grid-template-columns:1fr 1fr}.dashboard-grid .panel:last-child{grid-column:1/-1}.stats{margin-left:0;margin-right:0}.stats i{display:none}}
	@media(max-width:720px){.snore-home{padding:0 10px 25px}.hero{min-height:650px}.hero-art{inset:38% 0 0;background-position:center bottom;opacity:.8}.hero-shade{background:linear-gradient(180deg,#090a11 0%,rgba(9,10,17,.92) 42%,rgba(9,10,17,.25) 100%)}.hero-copy{width:auto;padding:55px 16px}.eyebrow{font-size:10px}h1{font-size:53px}.hero-copy>p{font-size:18px}.feature-row{display:grid;grid-template-columns:1fr}.feature{width:auto}.hero-actions{flex-direction:column}.hero-actions button{width:100%}.stats{grid-template-columns:1fr 1fr}.dashboard-grid{grid-template-columns:1fr;margin:0}.dashboard-grid .panel:last-child{grid-column:auto}.panel{padding:13px 9px}.row{gap:7px}.score-row .time,.country{display:none}.connect-banner,.connected{margin-left:0;margin-right:0;gap:15px;align-items:flex-start;flex-direction:column}.connected button{width:100%;justify-content:center}}
</style>
