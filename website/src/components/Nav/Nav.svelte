<script>
	import {onMount, getContext} from 'svelte';
	import {navigate} from 'svelte-routing';
	import eventBus from '../../utils/broadcast-channel-pubsub';
	import createAccountStore from '../../stores/saberrank/account';
	import createPlaylistStore from '../../stores/playlists';
	import followed from '../../stores/saberrank/followed';
	import {configStore} from '../../stores/config';
	import {opt} from '../../utils/js';
	import {clickOutside} from '../../svelte-utils/actions/click-outside';
	import {isTouchDevice} from '../../utils/is-touch';
	import {search} from '../../stores/search';
	import Dropdown from '../Common/Dropdown.svelte';
	import Avatar from '../Common/Avatar.svelte';
	import PlaylistMenuItem from './PlaylistMenuItem.svelte';
	import MenuLine from '../Player/MenuLine.svelte';
	import LinkMenuItem from './LinkMenuItem.svelte';
	import PlaylistHeaderMenuItem from './PlaylistHeaderMenuItem.svelte';
	import LogOutConfirm from './LogOutConfirm.svelte';

	let className = null;
	export {className as class};

	export let openModal = null;
	export let closeModal = null;

	let player = null;
	let settingsNotificationBadge = null;
	let clansNotification = null;

	function navigateToPlayer(player) {
		if (!player) return;

		navigate(`/u/${player.alias ?? player.playerId}`);
	}

	let accountMenuShown = false;
	let mobileMenuShown = false;

	onMount(async () => {
		const settingsBadgeUnsubscribe = eventBus.on('settings-notification-badge', message => (settingsNotificationBadge = message));

		const keyDownHandler = e => {
			if (e.ctrlKey === true && e.key === '/') {
				e.preventDefault();

				$search = true;
			}
		};

		document.addEventListener('keydown', keyDownHandler);

		return () => {
			settingsBadgeUnsubscribe();
			document.removeEventListener('keydown', keyDownHandler);
		};
	});

	function checkClanInvites() {
		if ($account?.clanRequest?.length) {
			let clansText = '';
			$account.clanRequest.forEach(clan => {
				clansText += `• ${clan.name} \n`;
			});
			clansNotification =
				`You have ${$account?.clanRequest?.length} clan invite${$account?.clanRequest?.length > 1 ? 's' : ''}: \n` + clansText;
		} else {
			clansNotification = null;
		}
	}

	const playlists = createPlaylistStore();
	const account = createAccountStore();


	let signupOptions = [];

	function calculateSignUpOptions(loggedInUser) {
		signupOptions =
			isTouchDevice() && player
				? [{component: LinkMenuItem, props: {label: 'My profile', url: `/u/${player.alias ?? player.playerId}`, class: 'touch-only'}}]
				: [];

		const isStaff = $account?.player?.playerInfo?.role
			?.split(',')
			?.some(role => ['admin', 'rankedteam', 'juniorrankedteam', 'creator', 'qualityteam'].includes(role));
		const isAdmin = $account?.player?.playerInfo?.role?.split(',')?.some(role => ['admin'].includes(role));
		if (loggedInUser.player) {
			if (isStaff) signupOptions.push({component: LinkMenuItem, props: {label: 'Staff Dashboard', url: '/staff'}});
			if (isStaff) signupOptions.push({component: LinkMenuItem, props: {label: 'Score Nominations', url: '/sotwnominations'}});
			if (isAdmin) signupOptions.push({component: LinkMenuItem, props: {label: 'Admin', url: '/admin'}});

			signupOptions.push({
				component: LinkMenuItem,
				props: {
					label: 'Log Out',
					callback: () => {
						if ($configStore?.preferences?.askOnLogOut) {
							logOut();
						} else {
							account.logOut();
							navigate('/');
						}
					},
				},
			});

			// followed
			if ($followed?.length) {
				signupOptions.push({class: 'dropdown-divider'});

				signupOptions = [
					...signupOptions,
					...starredFollowed.map(player => ({
						component: MenuLine,
						props: {player, withRank: false},
						onClick: e => {
							e?.preventDefault();
							e?.stopPropagation();
							accountMenuShown = false;
							navigateToPlayer(player);
						},
					})),
				];
				signupOptions.push({
					component: LinkMenuItem,
					props: {label: starredFollowed?.length ? 'More Followed...' : 'All Followed...', url: `/followed`},
				});
			}

			// playlists
			if (signupOptions.length) signupOptions.push({class: 'dropdown-divider'});

			signupOptions.push({
				component: PlaylistHeaderMenuItem,
			});

			signupOptions.push({
				component: LinkMenuItem,
				props: {
					label: 'Add new...',
					callback: async () => {
						playlists.create();
					},
				},
			});

			($playlists ?? []).map((playlist, idx) => {
				if (!playlist.oneclick) {
					signupOptions.push({
						component: PlaylistMenuItem,
						props: {
							playlist,
						},
						class: idx === selectedPlaylist ? 'selected' : '',
						onClick: () => {
							if ($configStore?.preferences?.playlistOption == 'selected') {
								playlists.select(idx === selectedPlaylist ? null : playlist);
							} else {
								navigate(`/playlists/${idx}`);
							}
						},
					});
				}
			});
		} else {
			signupOptions.push({component: LinkMenuItem, props: {label: 'Log In', url: '/signin'}});
		}
	}


	const logOut = async () => {
		openModal(LogOutConfirm, {
			confirm: () => {
				closeModal();
				account.logOut();
				navigate('/');
			},
			cancel: () => {
				closeModal();
			},
		});
	};

	$: player = $account?.player;
	$: starredFollowedIds = player?.profileSettings?.starredFriends ?? [];
	$: starredFollowed =
		$followed?.filter(f => starredFollowedIds.includes(f?.playerId))?.sort((a, b) => a?.name?.localeCompare(b?.name)) ?? [];
	$: selectedPlaylist = $configStore?.selectedPlaylist;
	$: calculateSignUpOptions($account, $playlists, selectedPlaylist);
	$: newSettingsAvailable = $configStore ? configStore.getNewSettingsAvailable() : undefined;
	$: notificationBadgeTitle = (settingsNotificationBadge ? [settingsNotificationBadge + '\n'] : [])
		.concat(newSettingsAvailable ? ['New settings are available:'].concat(newSettingsAvailable) : [])
		.join('\n');
	$: $account?.clanRequest ? checkClanInvites() : null;
	$: clanInviteBadgeTitle = clansNotification ? clansNotification : '';
</script>

<nav class={`ssr-page-container ${className ?? ''}`}>
	<div class="nav-brand">
		<a
			class="logo-link"
			href="/"
			aria-label="snoresaber home"
			on:click|preventDefault={() => navigate('/')}>
			<img src="/assets/snoresaber-logo-hd.png" class="logo" alt="snoresaber" />
			<span class="name">snoresaber</span>
		</a>
	</div>

	{#if player}
		<div class="me nav-button">
			<a
				href={`/u/${player.alias ?? player.playerId}`}
				aria-label="My profile"
				class="me-link"
				on:click|preventDefault={() => {
					if (!isTouchDevice()) {
						navigateToPlayer(player);
					} else {
						accountMenuShown = !accountMenuShown;
					}
				}}>
				{#if opt(player, 'playerInfo.avatar')}
					<Avatar {player} />
				{:else}
					<svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor">
						<path
							stroke-linecap="round"
							stroke-linejoin="round"
							stroke-width="2"
							d="M5.121 17.804A13.937 13.937 0 0112 16c2.5 0 4.847.655 6.879 1.804M15 10a3 3 0 11-6 0 3 3 0 016 0zm6 2a9 9 0 11-18 0 9 9 0 0118 0z" />
					</svg>
				{/if}

				Me
			</a>

			<Dropdown
				items={signupOptions}
				bind:shown={accountMenuShown}
				on:select={e => (e.detail.onClick ? e.detail.onClick() : (accountMenuShown = false))}
				noItems="">
				<svelte:fragment slot="row" let:item>
					<svelte:component
						this={item.component}
						{...item.props ?? {}}
						on:click={item.onClick
							? item.onClick
							: () => {
									accountMenuShown = false;
								}} />
				</svelte:fragment>
			</Dropdown>
		</div>
	{:else}
		<a href={`/signin`} aria-label="Sign In">
			<svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor">
				<path
					stroke-linecap="round"
					stroke-linejoin="round"
					stroke-width="2"
					d="M5.121 17.804A13.937 13.937 0 0112 16c2.5 0 4.847.655 6.879 1.804M15 10a3 3 0 11-6 0 3 3 0 016 0zm6 2a9 9 0 11-18 0 9 9 0 0118 0z" />
			</svg>

			Account
		</a>
	{/if}

	<a href="/ranking/1" aria-label="Ranking" on:click|preventDefault={() => navigate('/ranking/1')}>
		<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor">
			<path
				fill-rule="evenodd"
				d="M10 18a8 8 0 100-16 8 8 0 000 16zM4.332 8.027a6.012 6.012 0 011.912-2.706C6.512 5.73 6.974 6 7.5 6A1.5 1.5 0 019 7.5V8a2 2 0 004 0 2 2 0 011.523-1.943A5.977 5.977 0 0116 10c0 .34-.028.675-.083 1H15a2 2 0 00-2 2v2.197A5.973 5.973 0 0110 16v-2a2 2 0 00-2-2 2 2 0 01-2-2 2 2 0 00-1.668-1.973z"
				clip-rule="evenodd" />
		</svg>

		Ranking
	</a>

	<a href="#" aria-label="Search" on:click|preventDefault={() => ($search = true)} class="mobile-only">
		<svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor">
			<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z" />
		</svg>

		Search
	</a>

	<div class="right mobile-menu nav-button" on:click|preventDefault={() => (mobileMenuShown = !mobileMenuShown)}>
		<div class="hamburger">
			<svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"
				><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 6h16M4 12h16M4 18h16" /></svg>
		</div>

		<div
			class="dropdown-menu left"
			class:shown={mobileMenuShown}
			use:clickOutside={{callback: () => (mobileMenuShown = false), parent: '.nav-button'}}>
			<div class="dropdown-content">
				<div class="dropdown-item"><a href="/maps" on:click|preventDefault={() => navigate('/maps')}>Maps</a></div>
				<div class="dropdown-item"><a href="/events" on:click|preventDefault={() => navigate('/events')}>Events</a></div>
				<div class="dropdown-item"><a href="/clans" on:click|preventDefault={() => navigate('/clans')}>Clans</a></div>
				<div class="dropdown-item"><a href="/settings" on:click|preventDefault={() => navigate('/settings')}>Settings</a></div>
				<div class="dropdown-item"><a href="/privacy" on:click|preventDefault={() => navigate('/privacy')}>Privacy</a></div>
				<div class="dropdown-item"><a href="/help" on:click|preventDefault={() => navigate('/help')}>Help</a></div>
				<div class="dropdown-item"><a href="/socket" on:click|preventDefault={() => navigate('/socket')}>Scores Feed</a></div>
				<div class="dropdown-item"><a href="https://github.com/SaberRank" target="_blank" rel="noreferrer">Source</a></div>
			</div>
		</div>
	</div>

	<div class="right">
		<a href="#" aria-label="Search" on:click|preventDefault={() => ($search = true)} class="search-button">
			<div class="search-box">
				<div>
					<svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor">
						<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z" />
					</svg>

					<span>Search...</span>
				</div>

				<span class="search-hint">Ctrl + /</span>
			</div>
		</a>

		<a
			href="/maps"
			aria-label="Maps"
			on:click|preventDefault={() => {
				navigate('/maps');
			}}>
			<svg
				xmlns="http://www.w3.org/2000/svg"
				xmlns:xlink="http://www.w3.org/1999/xlink"
				version="1.1"
				style="width: 1.4em; height: 1.4em"
				fill="currentColor"
				viewBox="0 -0.000021333333336315263 256 242.00004266666667"
				xml:space="preserve">
				<desc>Created with Fabric.js 5.3.0</desc>
				<defs />
				<g transform="matrix(1.1048905253 0 0 1.1048905253 129.000001 121)" id="Hv0Mln4rOQt0RfN4JTAED">
					<path
						style="stroke: none; stroke-width: 0; stroke-dasharray: none; stroke-linecap: butt; stroke-dashoffset: 0; stroke-linejoin: miter; stroke-miterlimit: 4; fill-rule: nonzero;"
						transform=" translate(-129.4592048269, -121.9449386269)"
						d="M 45.000389 178.985321 C 45.00045 137.992767 45.189491 97.496063 44.861656 57.003548 C 44.784096 47.424049 54.241299 37.780491 64.001831 37.855831 C 107.327415 38.19025 150.658096 38.15852 193.98439 37.873428 C 203.12883 37.813251 212.811661 45.977306 212.993591 55.486298 C 213.339569 73.568573 213.811829 91.650421 213.935333 109.734474 C 214.098083 133.563858 214.114029 157.39653 213.883713 181.224792 C 213.821442 187.665894 213.30394 194.314758 208.5121 199.491562 C 205.003296 203.282211 201.02977 206.04187 195.477585 206.03447 C 151.149612 205.975357 106.821526 206.00708 62.493473 205.996414 C 53.822044 205.994339 45.304428 197.709274 45.017761 188.980713 C 44.913883 185.817917 45.000626 182.64888 45.000389 178.985321 M 61.998081 74.501007 C 61.998142 110.818878 62.069172 147.13707 61.905449 183.454208 C 61.885727 187.828964 63.081978 189.114273 67.51033 189.096863 C 108.492271 188.935852 149.475342 188.938278 190.457321 189.094498 C 194.831314 189.11116 196.114288 187.925613 196.096756 183.494614 C 195.934601 142.51268 195.937805 101.52961 196.09346 60.547623 C 196.110046 56.177303 194.930069 54.872513 190.494385 54.900955 C 161.174728 55.088959 131.853287 54.998047 102.532417 54.998112 C 90.537514 54.998138 78.541191 55.101215 66.548607 54.934402 C 63.07354 54.886063 61.755344 55.97451 61.939281 59.510906 C 62.181255 64.162987 61.998314 68.837181 61.998081 74.501007 z"
						stroke-linecap="round" />
				</g>
				<g transform="matrix(1.1048905253 0 0 1.1048905253 127.9964442239 91.1615548049)" id="BtC5zti8rEr_z2OL_M3iq">
					<path
						style="stroke: none; stroke-width: 1; stroke-dasharray: none; stroke-linecap: butt; stroke-dashoffset: 0; stroke-linejoin: miter; stroke-miterlimit: 4; fill-rule: nonzero;"
						transform=" translate(-128.5509186636, -94.9391452814)"
						d="M 90.715462 98.248207 C 88.056625 96.918488 85.791435 95.59008 83.360916 94.727562 C 76.309387 92.225159 74.808846 84.376251 79.059593 77.598518 C 80.176155 75.818184 83.098114 74.17614 85.203873 74.160065 C 114.335274 73.937668 143.469437 73.941101 172.600922 74.158386 C 174.653503 74.173698 177.382858 75.808907 178.605759 77.535141 C 182.406235 82.899879 179.884796 91.398643 173.976761 94.333504 C 160.161194 101.196495 146.240448 107.852715 132.522049 114.903503 C 127.989937 117.232849 124.640701 114.898148 121.093513 113.207054 C 111.019943 108.404556 101.046471 103.392128 90.715462 98.248207 z"
						stroke-linecap="round" />
				</g>
			</svg>

			Maps
		</a>

		<a
			href="/events"
			aria-label="Events"
			on:click|preventDefault={() => {
				navigate('/events');
			}}>
			<svg fill="none" stroke="currentColor" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"
				><path
					stroke-linecap="round"
					stroke-linejoin="round"
					stroke-width="2"
					d="M8 7V3m8 4V3m-9 8h10M5 21h14a2 2 0 002-2V7a2 2 0 00-2-2H5a2 2 0 00-2 2v12a2 2 0 002 2z" /></svg>

			Events
		</a>

		<a
			href="/clans"
			aria-label="Clans"
			title={clanInviteBadgeTitle}
			on:click|preventDefault={() => navigate('/clans')}
			class="tablet-and-up">
			<svg fill="currentColor" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 640 512"
				><path
					d="M184 88C184 118.9 158.9 144 128 144C97.07 144 72 118.9 72 88C72 57.07 97.07 32 128 32C158.9 32 184 57.07 184 88zM208.4 196.3C178.7 222.7 160 261.2 160 304C160 338.3 171.1 369.8 192 394.5V416C192 433.7 177.7 448 160 448H96C78.33 448 64 433.7 64 416V389.2C26.16 371.2 0 332.7 0 288C0 226.1 50.14 176 112 176H144C167.1 176 190.2 183.5 208.4 196.3V196.3zM64 245.7C54.04 256.9 48 271.8 48 288C48 304.2 54.04 319.1 64 330.3V245.7zM448 416V394.5C468 369.8 480 338.3 480 304C480 261.2 461.3 222.7 431.6 196.3C449.8 183.5 472 176 496 176H528C589.9 176 640 226.1 640 288C640 332.7 613.8 371.2 576 389.2V416C576 433.7 561.7 448 544 448H480C462.3 448 448 433.7 448 416zM576 330.3C585.1 319.1 592 304.2 592 288C592 271.8 585.1 256.9 576 245.7V330.3zM568 88C568 118.9 542.9 144 512 144C481.1 144 456 118.9 456 88C456 57.07 481.1 32 512 32C542.9 32 568 57.07 568 88zM256 96C256 60.65 284.7 32 320 32C355.3 32 384 60.65 384 96C384 131.3 355.3 160 320 160C284.7 160 256 131.3 256 96zM448 304C448 348.7 421.8 387.2 384 405.2V448C384 465.7 369.7 480 352 480H288C270.3 480 256 465.7 256 448V405.2C218.2 387.2 192 348.7 192 304C192 242.1 242.1 192 304 192H336C397.9 192 448 242.1 448 304zM256 346.3V261.7C246 272.9 240 287.8 240 304C240 320.2 246 335.1 256 346.3zM384 261.7V346.3C393.1 335 400 320.2 400 304C400 287.8 393.1 272.9 384 261.7z" /></svg>
			Clans

			{#if clansNotification}<div class="notification-badge" />{/if}
		</a>

		<a href="/settings" aria-label="Settings" title={notificationBadgeTitle} on:click|preventDefault={() => navigate('/settings')}>
			<svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor">
				<path
					stroke-linecap="round"
					stroke-linejoin="round"
					stroke-width="2"
					d="M10.325 4.317c.426-1.756 2.924-1.756 3.35 0a1.724 1.724 0 002.573 1.066c1.543-.94 3.31.826 2.37 2.37a1.724 1.724 0 001.065 2.572c1.756.426 1.756 2.924 0 3.35a1.724 1.724 0 00-1.066 2.573c.94 1.543-.826 3.31-2.37 2.37a1.724 1.724 0 00-2.572 1.065c-.426 1.756-2.924 1.756-3.35 0a1.724 1.724 0 00-2.573-1.066c-1.543.94-3.31-.826-2.37-2.37a1.724 1.724 0 00-1.065-2.572c-1.756-.426-1.756-2.924 0-3.35a1.724 1.724 0 001.066-2.573c-.94-1.543.826-3.31 2.37-2.37.996.608 2.296.07 2.572-1.065z" />
				<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z" />
			</svg>

			Settings

			{#if settingsNotificationBadge || newSettingsAvailable}<div class="notification-badge" />{/if}
		</a>
	</div>
</nav>

<style>
	nav {
		position: sticky;
		top: 0.75rem;
		width: calc(100% - 2rem);
		max-width: 1320px;
		margin: 0.75rem auto 0;
		min-height: 4rem;
		background: rgba(14, 16, 24, 0.94);
		border: 1px solid rgba(255,255,255,0.08);
		border-radius: 1.1rem;
		box-shadow: 0 14px 40px rgba(0,0,0,0.32);
		backdrop-filter: blur(18px);
		z-index: 50;
		display: flex;
		align-items: center;
		gap: 0.7rem;
		padding: 0.45rem;
	}

	nav > *:not(.right),
	nav > .right > *:not(.dropdown-menu),
	.me > a {
		display: inline-flex;
		align-items: center;
		justify-content: center;
		height: 3.05rem;
		font-size: 0.9rem;
		padding: 0 0.9rem;
		cursor: pointer;
		position: relative;
		border-radius: 0.8rem;
		transition: background .16s ease, transform .16s ease, border-color .16s ease;
	}

	nav > *:not(.right):hover,
	nav > .right > *:not(.dropdown-menu):hover {
		background: rgba(255,255,255,0.07);
		transform: translateY(-1px);
	}

	nav a { color: inherit !important; text-decoration: none; }

	.nav-brand { flex: 0 0 auto; padding: 0 !important; }
	.logo-link { display:flex; align-items:center; gap:0.7rem; padding:0 0.65rem !important; }
	.logo { width: 2.55rem !important; height: 2.55rem !important; object-fit: contain; flex: 0 0 auto; margin: 0 !important; padding: 0 !important; }
	.name { font-family:'Audiowide', sans-serif; font-size:0.9rem; letter-spacing:0.18em; margin:0; color:#fff; }

	nav > .me { order: 1; }
	nav > a[aria-label="Ranking"] { order: 2; }

	nav svg, nav .me :global(figure) { width:1.15rem; height:1.15rem; margin-right:0.5rem; flex:0 0 auto; }

	nav > .right {
		margin-left: auto;
		padding: 0;
		display:flex;
		align-items:center;
		gap:0.35rem;
		height:auto;
		flex: 1;
		justify-content:flex-end;
	}

	.search-button { flex: 0 1 26rem; }
	.search-button:hover { background: transparent !important; transform:none !important; }
	.search-box {
		display:flex; align-items:center; justify-content:space-between; width:100%;
		height:2.8rem; padding:0 0.85rem !important;
		border:1px solid rgba(255,255,255,0.09); border-radius:0.75rem;
		background:rgba(255,255,255,0.045); color:#aeb4c4;
	}
	.search-box > :first-child { display:inline-flex; align-items:center; }
	.search-hint { display:inline-block !important; opacity:.55; border:1px solid rgba(255,255,255,.14); border-radius:.35rem; padding:.12rem .35rem; font-size:.72rem; }

	.notification-badge { position:absolute; top:.55rem; right:.55rem; width:.45rem; height:.45rem; background:#ff3b81; border-radius:50%; box-shadow:0 0 10px rgba(255,59,129,.7); }
	.right.mobile-menu { display:none; }
	.mobile-only { display:none !important; }

	.me :global(.dropdown-menu) { width:15rem !important; max-width:60vw; }
	.me :global(.dropdown-item.selected) { background-color: var(--saberrank-primary); }

	@media screen and (max-width: 1021px) {
		nav { width:calc(100% - 1rem); margin-top:.5rem; }
		.search-button { flex:1; }
	}

	@media screen and (max-width: 767px) {
		nav { top:0.4rem; min-height:3.5rem; border-radius:.9rem; }
		.name { font-size:.78rem; letter-spacing:.13em; }
		.logo { width:2.2rem !important; height:2.2rem !important; }
		nav > a[aria-label="Ranking"] { display:none; }
		.search-button { display:none !important; }
		.mobile-only { display:inline-flex !important; }
		.right.mobile-menu { display:inline-flex; flex:0 0 auto; }
		.right.mobile-menu > .hamburger { display:flex; align-items:center; justify-content:center; width:2.7rem; height:2.7rem; border-radius:.75rem; background:rgba(255,255,255,.06); }
		.right.mobile-menu .hamburger svg { margin:0; }
		nav > .right:not(.mobile-menu) { display:none; }
	}
</style>
