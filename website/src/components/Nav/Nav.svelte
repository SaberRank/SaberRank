<script>
	import {onMount} from 'svelte';
	import {navigate} from 'svelte-routing';
	import {search} from '../../stores/search';
	import createAccountStore from '../../stores/saberrank/account';

	const account = createAccountStore();
	let accountOpen = false;
	let mobileOpen = false;

	const go = path => {
		accountOpen = false;
		mobileOpen = false;
		navigate(path);
	};

	onMount(() => {
		const handler = e => {
			if (e.ctrlKey && e.key === '/') {
				e.preventDefault();
				$search = true;
			}
		};
		window.addEventListener('keydown', handler);
		return () => window.removeEventListener('keydown', handler);
	});
</script>

<header class="snore-nav">
	<div class="nav-inner">
		<a class="brand" href="/" aria-label="SnoreSaber home" on:click|preventDefault={() => go('/') }>
			<img src="/assets/snoresaber-logo-transparent.png" alt="SnoreSaber" />
		</a>

		<button class="search" aria-label="Search" on:click={() => ($search = true)}>
			<svg viewBox="0 0 24 24" aria-hidden="true"><path d="m21 21-4.35-4.35m2.1-5.4a7.5 7.5 0 1 1-15 0 7.5 7.5 0 0 1 15 0Z"/></svg>
			<span>Search players, maps, or scores...</span>
			<kbd>Ctrl /</kbd>
		</button>

		<nav class="desktop-links" aria-label="Primary">
			<a class="active" href="/ranking/1" on:click|preventDefault={() => go('/ranking/1')}>
				<span class="icon">♛</span><span>Rankings</span>
			</a>
			<a href="/maps" on:click|preventDefault={() => go('/maps')}>
				<span class="icon">⌗</span><span>Maps</span>
			</a>
			<a href="/events" on:click|preventDefault={() => go('/events')}>
				<span class="icon">▣</span><span>Events</span>
			</a>
			<a href="/clans" on:click|preventDefault={() => go('/clans')}>
				<span class="icon">♟</span><span>Clans</span>
			</a>
		</nav>

		<div class="account-wrap">
			<button class="account-button" on:click={() => (accountOpen = !accountOpen)} aria-expanded={accountOpen}>
				<span class="user-icon">♙</span>
				<span>{ $account?.player?.name ?? 'Account' }</span>
				<span class="chevron">⌄</span>
			</button>
			{#if accountOpen}
				<div class="account-menu">
					{#if $account?.player}
						<button on:click={() => go(`/u/${$account.player.alias ?? $account.player.playerId}`)}>Profile</button>
						<button on:click={() => go('/settings')}>Settings</button>
					{:else}
						<button on:click={() => go('/signin')}>Sign in</button>
					{/if}
				</div>
			{/if}
		</div>

		<button class="mobile-toggle" aria-label="Open navigation" on:click={() => (mobileOpen = !mobileOpen)}>☰</button>
	</div>

	{#if mobileOpen}
		<div class="mobile-menu">
			<button on:click={() => go('/ranking/1')}>♛ Rankings</button>
			<button on:click={() => go('/maps')}>⌗ Maps</button>
			<button on:click={() => go('/events')}>▣ Events</button>
			<button on:click={() => go('/clans')}>♟ Clans</button>
			<button on:click={() => ($search = true)}>⌕ Search</button>
		</div>
	{/if}
</header>

<style>
	.snore-nav { position: sticky; top: 0; z-index: 1000; height: 82px; border-bottom: 1px solid rgba(255,255,255,.08); background: rgba(7,8,15,.88); backdrop-filter: blur(22px); }
	.nav-inner { max-width: 1600px; height: 100%; margin: 0 auto; padding: 0 36px; display: flex; align-items: center; gap: 34px; }
	.brand { width: 330px; flex: 0 0 330px; display:flex; align-items:center; text-decoration:none; }
	.brand img { width: 310px; height: 70px; object-fit: contain; object-position:left center; }
	.search { height: 48px; flex: 1; min-width: 280px; max-width: 550px; border: 1px solid rgba(179,164,255,.25); border-radius: 14px; background: rgba(22,22,36,.72); color:#9d9db0; display:flex; align-items:center; gap:14px; padding: 0 14px; text-align:left; box-shadow: inset 0 0 24px rgba(255,102,198,.035); cursor:pointer; }
	.search svg { width: 23px; height:23px; stroke:#f5f4fb; fill:none; stroke-width:1.8; }
	.search span { font-size:14px; flex:1; white-space:nowrap; overflow:hidden; text-overflow:ellipsis; }
	.search kbd { font: 12px/1.2 inherit; color:#8d8c9e; border:1px solid rgba(255,255,255,.1); border-radius:7px; padding:6px 8px; }
	.desktop-links { display:flex; align-items:center; gap:4px; }
	.desktop-links a { color:#f5f4fb; text-decoration:none; height:48px; padding:0 17px; display:flex; align-items:center; gap:10px; border-radius:14px; font-size:14px; font-weight:600; white-space:nowrap; }
	.desktop-links a:hover { background:rgba(255,255,255,.055); }
	.desktop-links a.active { background:linear-gradient(135deg,rgba(244,77,173,.16),rgba(137,88,255,.10)); border:1px solid rgba(242,107,188,.2); box-shadow:0 0 30px rgba(238,75,183,.08); }
	.desktop-links .icon { font-size:22px; color:#fff; line-height:1; }
	.account-wrap { position:relative; margin-left:auto; }
	.account-button { height:48px; min-width:140px; border:1px solid rgba(185,170,255,.22); border-radius:14px; background:rgba(22,22,36,.72); color:#fff; display:flex; align-items:center; justify-content:center; gap:10px; padding:0 14px; cursor:pointer; font-size:14px; }
	.user-icon { font-size:24px; }
	.chevron { opacity:.7; font-size:18px; }
	.account-menu { position:absolute; right:0; top:56px; width:180px; padding:8px; border:1px solid rgba(255,255,255,.1); border-radius:14px; background:rgba(17,17,28,.97); box-shadow:0 20px 60px rgba(0,0,0,.5); }
	.account-menu button, .mobile-menu button { width:100%; border:0; background:transparent; color:#eee; text-align:left; padding:11px 12px; border-radius:9px; cursor:pointer; }
	.account-menu button:hover, .mobile-menu button:hover { background:rgba(255,255,255,.07); }
	.mobile-toggle, .mobile-menu { display:none; }
	@media(max-width:1250px) { .brand { width:230px; flex-basis:230px; } .brand img { width:220px; } .desktop-links a { padding:0 11px; } .nav-inner { gap:16px; padding:0 20px; } }
	@media(max-width:900px) { .desktop-links { display:none; } .account-wrap { margin-left:0; } .mobile-toggle { display:block; border:1px solid rgba(255,255,255,.12); background:rgba(255,255,255,.05); color:#fff; border-radius:12px; height:46px; width:46px; font-size:21px; } .mobile-menu { display:flex; flex-direction:column; gap:4px; position:absolute; left:16px; right:16px; top:74px; padding:10px; background:rgba(12,12,21,.98); border:1px solid rgba(255,255,255,.1); border-radius:14px; box-shadow:0 20px 60px rgba(0,0,0,.55); } }
	@media(max-width:650px) { .nav-inner { padding:0 12px; gap:8px; } .brand { width:150px; flex-basis:150px; } .brand img { width:145px; height:60px; } .search { min-width:0; padding:0 10px; } .search span, .search kbd { display:none; } .search { flex:0 0 48px; width:48px; justify-content:center; } .account-button { min-width:46px; width:46px; padding:0; } .account-button span:not(.user-icon) { display:none; } }
</style>
