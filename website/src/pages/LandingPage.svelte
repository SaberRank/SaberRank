<script>
	import {navigate} from 'svelte-routing';
	import {MetaTags} from 'svelte-meta-tags';
	import ssrConfig from '../ssr-config';

	const players = [
		{name:'YawningSylveon', flag:'🇨🇦', value:'16,284.32', tone:'pink'},
		{name:'Lunaa', flag:'🇺🇸', value:'16,112.07'},
		{name:'Kyouki', flag:'🇯🇵', value:'15,998.44'},
		{name:'Rho', flag:'🇺🇸', value:'15,781.20'},
		{name:'Astra', flag:'🇬🇧', value:'15,662.11'}
	];
	const maps = [
		{name:'Imprinting', author:'Sotarks', value:'382,144', cover:'/assets/defaultcover.jpg'},
		{name:'B.K.K.B.K.K.B.K.', author:'nora2r', value:'301,552', cover:'/assets/Discover/extra_sensory_thumbnail.webp'},
		{name:'Kimi no Bouken', author:'Sotarks', value:'298,771', cover:'/assets/Discover/cc-summer-highlights.jpg'},
		{name:'Ghost', author:'Rustic', value:'286,905', cover:'/assets/Discover/pokesaber2.jpg'},
		{name:'Machine Gun', author:'Sotarks', value:'274,663', cover:'/assets/defaultcover.jpg'}
	];
	const scores = [
		{name:'YawningSylveon', map:'Imprinting', acc:'98.42%', time:'2m ago'},
		{name:'Lunaa', map:'Kimi no Bouken', acc:'97.15%', time:'5m ago'},
		{name:'Kyouki', map:'Ghost', acc:'96.88%', time:'6m ago'},
		{name:'Rho', map:'B.K.K.B.K.K.B.K.', acc:'97.63%', time:'8m ago'},
		{name:'Astra', map:'Machine Gun', acc:'95.21%', time:'11m ago'}
	];

	const go = path => navigate(path);
</script>

<svelte:head><title>SnoreSaber — Beat Saber rankings</title></svelte:head>
<MetaTags title="SnoreSaber — Beat Saber rankings" description="Beat Saber scores, rankings, maps, and player progress." />

<div class="snore-home">
	<section class="hero">
		<div class="hero-art"></div>
		<div class="hero-copy">
			<div class="eyebrow">BEAT SABER · SCORES · RANKINGS</div>
			<h1><span>snore</span><strong>saber</strong></h1>
			<p>Beat Saber scores, rankings, and player progress.</p>
			<div class="feature-row">
				<div class="feature"><div class="feature-icon">▮▮▮</div><div><b>Track</b><small>View scores, ranks, and improvement over time.</small></div></div>
				<div class="feature"><div class="feature-icon">⌗</div><div><b>Explore</b><small>Browse maps and leaderboards.</small></div></div>
				<div class="feature"><div class="feature-icon">♟</div><div><b>Compete</b><small>See where you stand.</small></div></div>
			</div>
			<div class="hero-actions">
				<button class="primary" on:click={() => go('/ranking/1')}>View Rankings <span>→</span></button>
				<button class="secondary" on:click={() => go('/maps')}>⌗ &nbsp; Explore Maps</button>
			</div>
		</div>
	</section>

	<section class="stats">
		<div><span class="stat-icon pink">▮▮▮</span><div><b>8,214,632</b><small>Scores</small></div><i></i></div>
		<div><span class="stat-icon purple">♟</span><div><b>152,908</b><small>Players</small></div><i></i></div>
		<div><span class="stat-icon blue">⌗</span><div><b>24,317</b><small>Maps</small></div><i></i></div>
		<div><span class="stat-icon pink">♟</span><div><b>412</b><small>Clans</small></div><i></i></div>
	</section>

	<section class="dashboard-grid">
		<div class="panel">
			<header><h2>Top Players</h2><button on:click={() => go('/ranking/1')}>View All →</button></header>
			{#each players as player, i}
				<button class="row player-row" on:click={() => player.name === 'YawningSylveon' ? go('/u/YawningSylveon') : go('/ranking/1')}>
					<span class="rank">{i + 1}</span><span class="avatar">{player.name[0]}</span><span class="row-name">{player.name}</span><span>{player.flag}</span><span class="row-value">{player.value}</span>
				</button>
			{/each}
		</div>
		<div class="panel">
			<header><h2>Popular Maps</h2><button on:click={() => go('/maps')}>View All →</button></header>
			{#each maps as map, i}
				<button class="row map-row" on:click={() => go('/maps')}>
					<span class="rank">{i + 1}</span><img src={map.cover} alt=""/><span class="map-info"><b>{map.name}</b><small>{map.author}</small></span><span class="row-value">{map.value}</span>
				</button>
			{/each}
		</div>
		<div class="panel">
			<header><h2>Recent Scores</h2><button on:click={() => go('/scores/1')}>View All →</button></header>
			{#each scores as score}
				<button class="row score-row" on:click={() => go('/scores/1')}>
					<span class="avatar">{score.name[0]}</span><span class="row-name">{score.name}</span><span class="map-info"><b>{score.map}</b></span><span class="acc">{score.acc}</span><span class="time">{score.time}</span>
				</button>
			{/each}
		</div>
	</section>
</div>

<style>
	:global(body) { margin:0; background:#07080f; }
	.snore-home { max-width:1600px; margin:0 auto; padding:0 36px 42px; color:#f7f4fb; }
	.hero { min-height:455px; position:relative; overflow:hidden; border-bottom:1px solid rgba(255,255,255,.05); }
	.hero:after { content:""; position:absolute; inset:0; background:linear-gradient(90deg,#090a11 0%,rgba(9,10,17,.92) 28%,rgba(9,10,17,.45) 55%,rgba(9,10,17,.06) 100%); pointer-events:none; }
	.hero-art { position:absolute; inset:0 0 0 40%; background:url('/assets/snoresaber-hero-art.png') right center/cover no-repeat; opacity:.98; }
	.hero-copy { position:relative; z-index:2; width:61%; padding:76px 0 30px 92px; }
	.eyebrow { color:#ff91d4; font-size:12px; font-weight:800; letter-spacing:.27em; margin-bottom:17px; }
	h1 { margin:0; font-size:76px; line-height:.95; letter-spacing:.04em; font-weight:300; font-family:Arial,sans-serif; }
	h1 span { color:#f8f8fb; } h1 strong { color:#ed6dbd; font-weight:300; }
	.hero-copy > p { margin:22px 0 32px; color:#b7b6ca; font-size:24px; }
	.feature-row { display:flex; gap:12px; }
	.feature { min-width:0; width:31%; display:flex; align-items:center; gap:13px; padding:13px 15px; border:1px solid rgba(255,255,255,.08); border-radius:16px; background:rgba(11,12,21,.56); backdrop-filter:blur(12px); }
	.feature-icon { width:48px; height:48px; flex:0 0 48px; display:grid; place-items:center; border-radius:13px; color:#ff8dd0; background:linear-gradient(135deg,rgba(242,79,177,.18),rgba(132,78,255,.11)); border:1px solid rgba(240,110,194,.2); font-weight:900; }
	.feature b { display:block; color:#f49bd4; font-size:15px; margin-bottom:5px; } .feature small { display:block; color:#b2b0c2; line-height:1.3; font-size:12px; }
	.hero-actions { display:flex; gap:14px; margin-top:31px; }
	.hero-actions button { height:53px; padding:0 31px; border-radius:14px; font-size:15px; font-weight:800; cursor:pointer; }
	.primary { border:0; background:linear-gradient(90deg,#ef72bb,#ee6ebd); color:#1b0c18; box-shadow:0 12px 35px rgba(237,102,186,.22); } .primary span { font-size:23px; margin-left:16px; }
	.secondary { border:1px solid #535269; background:rgba(12,13,22,.72); color:#f4f2f8; }
	.stats { display:grid; grid-template-columns:repeat(4,1fr); gap:15px; margin:0 14px 17px; transform:translateY(-1px); }
	.stats > div { min-height:76px; border:1px solid rgba(255,255,255,.08); background:linear-gradient(135deg,rgba(19,20,31,.94),rgba(11,12,20,.94)); border-radius:15px; display:flex; align-items:center; padding:0 20px; gap:14px; box-sizing:border-box; }
	.stat-icon { width:34px; font-size:25px; text-align:center; } .stat-icon.pink{color:#ff83ce}.stat-icon.purple{color:#a894ff}.stat-icon.blue{color:#7ea2ff}
	.stats b { display:block; font-size:18px; } .stats small { display:block; color:#9e9cac; font-size:12px; margin-top:3px; }
	.stats i { margin-left:auto; width:150px; height:24px; border-bottom:2px solid #e983c7; border-radius:50%; transform:skewY(-7deg); opacity:.9; }
	.dashboard-grid { display:grid; grid-template-columns:1.03fr 1fr 1fr; gap:22px; margin:0 14px; }
	.panel { border:1px solid rgba(255,255,255,.08); background:linear-gradient(145deg,rgba(17,18,28,.97),rgba(10,11,19,.97)); border-radius:15px; padding:15px 18px 12px; overflow:hidden; }
	.panel header { display:flex; align-items:center; justify-content:space-between; padding:4px 4px 13px; } .panel h2 { font-size:16px; margin:0; } .panel header button { border:0; background:none; color:#ef7dc6; cursor:pointer; font-weight:700; }
	.row { width:100%; border:0; color:#f2eff7; background:transparent; display:flex; align-items:center; gap:12px; height:39px; padding:0 7px; border-radius:9px; text-align:left; cursor:pointer; }
	.row:hover { background:rgba(255,255,255,.045); } .rank { width:20px; color:#dedbe6; text-align:center; font-size:13px; } .avatar { width:28px; height:28px; flex:0 0 28px; border-radius:50%; display:grid; place-items:center; background:linear-gradient(135deg,#ff6eb9,#8e68ff); color:#fff; font-size:11px; font-weight:900; } .row-name { font-size:13px; white-space:nowrap; overflow:hidden; text-overflow:ellipsis; } .row-value { margin-left:auto; color:#dedbe6; font-size:13px; white-space:nowrap; } .map-row img { width:28px; height:28px; object-fit:cover; border-radius:6px; } .map-info { min-width:0; flex:1; } .map-info b,.map-info small { display:block; overflow:hidden; text-overflow:ellipsis; white-space:nowrap; } .map-info b{font-size:12px}.map-info small{font-size:10px;color:#858492;margin-top:2px}.score-row .map-info{flex:1}.acc{color:#e98bc7;font-size:12px}.time{color:#858492;font-size:11px;white-space:nowrap}
	@media(max-width:1100px){.snore-home{padding:0 18px 30px}.hero-copy{padding-left:35px;width:67%}h1{font-size:62px}.feature-row{flex-wrap:wrap}.feature{width:calc(50% - 6px)}.dashboard-grid{grid-template-columns:1fr 1fr}.dashboard-grid .panel:last-child{grid-column:1/-1}.stats{margin-left:0;margin-right:0}}
	@media(max-width:720px){.snore-home{padding:0 10px 25px}.hero{min-height:650px}.hero-art{inset:38% 0 0 0;background-position:center bottom;opacity:.8}.hero:after{background:linear-gradient(180deg,#090a11 0%,rgba(9,10,17,.92) 42%,rgba(9,10,17,.25) 100%)}.hero-copy{width:auto;padding:55px 16px}.eyebrow{font-size:10px}h1{font-size:53px}.hero-copy>p{font-size:18px}.feature-row{display:grid;grid-template-columns:1fr}.feature{width:auto}.hero-actions{flex-direction:column}.hero-actions button{width:100%}.stats{grid-template-columns:1fr 1fr}.stats i{display:none}.dashboard-grid{grid-template-columns:1fr;margin:0}.dashboard-grid .panel:last-child{grid-column:auto}.panel{padding:13px 9px}.row{gap:7px}.score-row .time{display:none}}
</style>
