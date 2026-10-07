<script>
	import {onMount} from 'svelte';
	import {navigate} from 'svelte-routing';
	import {search} from '../../stores/search';
	import snoreAccount, {logoutSnoreAccount} from '../../stores/snore-account';

	let accountOpen = false;
	let mobileOpen = false;
	let currentPath = '/';

	const links = [
		{label:'Rankings', path:'/ranking/1', icon:'crown'},
		{label:'Maps', path:'/maps', icon:'grid'},
		{label:'Events', path:'/events', icon:'calendar'},
		{label:'Clans', path:'/clans', icon:'users'},
	];

	function go(path) {
		accountOpen = false;
		mobileOpen = false;
		currentPath = path;
		navigate(path);
	}

	function active(path) {
		if (path === '/ranking/1') return currentPath.startsWith('/ranking');
		return currentPath === path || currentPath.startsWith(path + '/');
	}

	onMount(() => {
		currentPath = window.location.pathname;
		const handler = e => {
			if (e.ctrlKey && e.key === '/') { e.preventDefault(); $search = true; }
			if (e.key === 'Escape') { accountOpen = false; mobileOpen = false; }
		};
		window.addEventListener('keydown', handler);
		return () => window.removeEventListener('keydown', handler);
	});
</script>

<header class="snore-nav">
	<div class="nav-inner">
		<a class="brand" href="/" aria-label="Snore Saber home" on:click|preventDefault={() => go('/')}>
			<img src="/assets/snoresaber-logo-transparent.png" alt="snore saber" />
		</a>

		<button class="search" aria-label="Search" on:click={() => ($search = true)}>
			<svg viewBox="0 0 24 24" aria-hidden="true"><circle cx="10.8" cy="10.8" r="6.8"/><path d="m16 16 5 5"/></svg>
			<span>Search players, maps, or scores...</span><kbd>Ctrl /</kbd>
		</button>

		<nav class="desktop-links" aria-label="Primary">
			{#each links as link}
				<a class:active={active(link.path)} href={link.path} on:click|preventDefault={() => go(link.path)}>
					<svg viewBox="0 0 24 24" aria-hidden="true">
						{#if link.icon === 'crown'}<path d="m3 7 4 4 5-7 5 7 4-4-2 12H5L3 7Z"/><path d="M5 19h14"/>
						{:else if link.icon === 'grid'}<rect x="4" y="4" width="16" height="16" rx="1"/><path d="M4 10h16M10 4v16"/>
						{:else if link.icon === 'calendar'}<rect x="4" y="5" width="16" height="15" rx="2"/><path d="M8 3v4M16 3v4M4 10h16"/>
						{:else}<circle cx="9" cy="8" r="3"/><circle cx="17" cy="9" r="2.4"/><path d="M3.5 20c.5-4 2.5-6 5.5-6s5 2 5.5 6M14 15c3.3-.8 5.7 1.2 6.5 5"/>{/if}
					</svg><span>{link.label}</span>
				</a>
			{/each}
		</nav>

		<div class="account-wrap">
			<button class="account-button" on:click={() => (accountOpen = !accountOpen)} aria-expanded={accountOpen}>
				{#if $snoreAccount?.avatarUrl}<img src={$snoreAccount.avatarUrl} alt="" />{:else}<span class="user-dot"><svg viewBox="0 0 24 24"><circle cx="12" cy="8" r="3.2"/><path d="M5 20c.5-4 2.8-6 7-6s6.5 2 7 6"/></svg></span>{/if}
				<span class="account-name">{$snoreAccount?.displayName ?? 'Account'}</span><span class="chevron">⌄</span>
			</button>
			{#if accountOpen}
				<div class="account-menu">
					{#if $snoreAccount}
						<button on:click={() => go(`/u/${$snoreAccount.scoresaberId}`)}>Profile</button>
						<button on:click={() => { logoutSnoreAccount(); accountOpen = false; }}>Disconnect profile</button>
					{:else}<button on:click={() => go('/signin')}>Connect profile</button>{/if}
				</div>
			{/if}
		</div>

		<button class="mobile-toggle" aria-label="Open navigation" on:click={() => (mobileOpen = !mobileOpen)}>
			<svg viewBox="0 0 24 24"><path d="M4 7h16M4 12h16M4 17h16"/></svg>
		</button>
	</div>

	{#if mobileOpen}
		<div class="mobile-menu">
			{#each links as link}<button class:active={active(link.path)} on:click={() => go(link.path)}>{link.label}</button>{/each}
			<button on:click={() => ($search = true)}>Search</button>
		</div>
	{/if}
</header>

<style>
	.snore-nav{position:sticky;top:0;z-index:1000;height:82px;border-bottom:1px solid rgba(255,255,255,.08);background:rgba(7,8,15,.9);backdrop-filter:blur(22px)}
	.nav-inner{max-width:1600px;height:100%;margin:0 auto;padding:0 36px;display:flex;align-items:center;gap:24px}.brand{width:265px;flex:0 0 265px;height:100%;display:flex;align-items:center;text-decoration:none}.brand img{width:245px;height:64px;object-fit:contain;object-position:left center}
	.search{height:48px;flex:1;min-width:240px;max-width:520px;border:1px solid rgba(179,164,255,.24);border-radius:14px;background:rgba(22,22,36,.72);color:#9d9db0;display:flex;align-items:center;gap:13px;padding:0 14px;text-align:left;box-shadow:inset 0 0 24px rgba(255,102,198,.035);cursor:pointer}.search svg{width:21px;height:21px;stroke:#f5f4fb;fill:none;stroke-width:1.8;stroke-linecap:round}.search span{font-size:14px;flex:1;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}.search kbd{font:12px/1.2 inherit;color:#8d8c9e;border:1px solid rgba(255,255,255,.1);border-radius:7px;padding:6px 8px}
	.desktop-links{display:flex;align-items:center;gap:3px}.desktop-links a{color:#f5f4fb;text-decoration:none;height:48px;padding:0 14px;display:flex;align-items:center;gap:9px;border-radius:14px;font-size:14px;font-weight:650;white-space:nowrap}.desktop-links a:hover{background:rgba(255,255,255,.055)}.desktop-links a.active{background:linear-gradient(135deg,rgba(244,77,173,.17),rgba(137,88,255,.11));border:1px solid rgba(242,107,188,.22);box-shadow:0 0 30px rgba(238,75,183,.08)}.desktop-links svg{width:20px;height:20px;stroke:#fff;fill:none;stroke-width:1.6;stroke-linecap:round;stroke-linejoin:round}
	.account-wrap{position:relative;margin-left:auto}.account-button{height:48px;min-width:145px;border:1px solid rgba(185,170,255,.22);border-radius:14px;background:rgba(22,22,36,.72);color:#fff;display:flex;align-items:center;justify-content:center;gap:9px;padding:0 13px;cursor:pointer;font-size:14px}.account-button img,.user-dot{width:27px;height:27px;border-radius:50%;object-fit:cover}.user-dot{display:grid;place-items:center;background:linear-gradient(135deg,#ef72bb,#8f78ff)}.user-dot svg{width:18px;height:18px;stroke:#fff;fill:none;stroke-width:1.5}.account-name{max-width:85px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}.chevron{opacity:.7;font-size:16px}.account-menu{position:absolute;right:0;top:56px;width:200px;padding:8px;border:1px solid rgba(255,255,255,.1);border-radius:14px;background:rgba(17,17,28,.98);box-shadow:0 20px 60px rgba(0,0,0,.5)}.account-menu button,.mobile-menu button{width:100%;border:0;background:transparent;color:#eee;text-align:left;padding:11px 12px;border-radius:9px;cursor:pointer}.account-menu button:hover,.mobile-menu button:hover,.mobile-menu button.active{background:rgba(255,255,255,.07)}
	.mobile-toggle,.mobile-menu{display:none}.mobile-toggle{border:1px solid rgba(255,255,255,.12);background:rgba(255,255,255,.05);color:#fff;border-radius:12px;height:46px;width:46px}.mobile-toggle svg{width:22px;stroke:#fff;fill:none;stroke-width:2;stroke-linecap:round}
	@media(max-width:1250px){.brand{width:205px;flex-basis:205px}.brand img{width:195px}.desktop-links a{padding:0 9px}.nav-inner{gap:12px;padding:0 20px}}
	@media(max-width:930px){.desktop-links{display:none}.account-wrap{margin-left:0}.mobile-toggle{display:grid;place-items:center}.mobile-menu{display:flex;flex-direction:column;gap:4px;position:absolute;left:16px;right:16px;top:74px;padding:10px;background:rgba(12,12,21,.98);border:1px solid rgba(255,255,255,.1);border-radius:14px;box-shadow:0 20px 60px rgba(0,0,0,.55)}}
	@media(max-width:650px){.nav-inner{padding:0 12px;gap:8px}.brand{width:145px;flex-basis:145px}.brand img{width:140px;height:60px}.search{min-width:0;padding:0 10px;flex:0 0 48px;width:48px;justify-content:center}.search span,.search kbd{display:none}.account-button{min-width:46px;width:46px;padding:0}.account-name,.chevron{display:none}}
</style>
