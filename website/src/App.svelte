<script>
	import {onMount, setContext} from 'svelte';
	import {Router, Route, navigate} from 'svelte-routing';
	import Notifications from 'svelte-notifications';
	import {configStore} from './stores/config';
	import createAccountStore from './stores/saberrank/account';
	import {search} from './stores/search';
	import createContainerStore from './stores/container';
	import {isTouchDevice} from './utils/is-touch';
	import Nav from './components/Nav/Nav.svelte';
	import {importFonts, setGlobalCSSValue} from './utils/color';
	import ContentBox from './components/Common/ContentBox.svelte';
	import PlaylistCart from './components/Playlists/PlaylistCart.svelte';
	import Search from './components/Search/Search.svelte';
	import NotificationComponent from './components/Common/NotificationComponent.svelte';
	import GlobalClansMapHistory from './components/Clans/GlobalClansMapHistory.svelte';
	import rewindTimer from './stores/rewind-timer';
	import {padNumber} from './utils/format';
	import SimpleModal from './components/Common/SimpleModal.svelte';
	import {Svrollbar} from 'svrollbar';
	import {produce} from 'immer';
	import TournamentTopBanner from './components/Common/TournamentTopBanner.svelte';
	import {initReturnMorph} from './utils/view-transition';

	// Dynamic imports for pages
	const pageImports = {
		RankingPage: () => import('./pages/Ranking.svelte'),
		EventPage: () => import('./pages/Event.svelte'),
		LoveLivePage: () => import('./pages/LoveLive.svelte'),
		RocketLeaguePackPage: () => import('./pages/RocketLeaguePack.svelte'),
		LeaderboardPage: () => import('./pages/Leaderboard.svelte'),
		LeaderboardsPage: () => import('./pages/Leaderboards.svelte'),
		LeaderboardsLoloppe: () => import('./pages/LeaderboardsLoloppe.svelte'),
		ClanPage: () => import('./pages/Clan.svelte'),
		ClansPage: () => import('./pages/Clans.svelte'),
		FollowedPage: () => import('./pages/Followed.svelte'),
		PlayerPage: () => import('./pages/Player.svelte'),
		NotFoundPage: () => import('./pages/NotFound.svelte'),
		PrivacyPage: () => import('./pages/Privacy.svelte'),
		AboutPage: () => import('./pages/About.svelte'),
		DashboardPage: () => import('./pages/Dashboard.svelte'),
		PlaylistsPage: () => import('./pages/Playlists.svelte'),
		PlaylistPage: () => import('./pages/Playlist.svelte'),
		FeaturedPlaylists: () => import('./pages/FeaturedPlaylists.svelte'),
		SigninPage: () => import('./pages/SignIn.svelte'),
		OauthSignInPage: () => import('./pages/OauthSignIn.svelte'),
		SupportPage: () => import('./pages/Support.svelte'),
		StaffDashboard: () => import('./pages/StaffDashboard.svelte'),
		EventsPage: () => import('./pages/Events.svelte'),
		Socket: () => import('./pages/Socket.svelte'),
		Settings: () => import('./pages/Settings.svelte'),
		LandingPage: () => import('./pages/LandingPage.svelte'),
		CensusPage: () => import('./pages/Census.svelte'),
		SurveyAchievementPage: () => import('./pages/SurveyAchievement.svelte'),
		PatreonPage: () => import('./pages/Patreon.svelte'),
		DeveloperPortalPage: () => import('./pages/DeveloperPortal.svelte'),
		MapsPortal: () => import('./pages/MapsPortal.svelte'),
		MapsPage: () => import('./pages/MapsList.svelte'),
		MapsTrending: () => import('./pages/MapsTrending.svelte'),
		Replayed: () => import('./pages/Replayed.svelte'),
		ReplayedLanding: () => import('./pages/ReplayedLanding.svelte'),
		ClansMap: () => import('./pages/ClansMap.svelte'),
		SongSuggestMap: () => import('./pages/SongSuggestMap.svelte'),
		GigaMap: () => import('./pages/GigaMap.svelte'),
		AdminPage: () => import('./pages/Admin.svelte'),
		Week100Page: () => import('./pages/Week100.svelte'),
		TibytesPresets: () => import('./pages/TibytesPresets.svelte'),
		BuildingBlocks2024: () => import('./pages/BuildingBlocks2024.svelte'),
		ProjectTree: () => import('./pages/ProjectTree.svelte'),
		ScoresPage: () => import('./pages/Scores.svelte'),
		ScorePage: () => import('./pages/Score.svelte'),
		NominatedScores: () => import('./pages/NominatedScores.svelte'),
		BadgesPage: () => import('./pages/Badges.svelte'),
		BeastiesNominations: () => import('./pages/BeastiesNominations.svelte'),
		GamifiedVivifyPack: () => import('./pages/GamifiedVivifyPack.svelte'),
	};

	export let url = '';

	let mainEl = null;

	const containerStore = createContainerStore();
	const account = createAccountStore();

	setContext('pageContainer', containerStore);

	let mobileTooltip = null;
	onMount(() => {
		initReturnMorph();
		const hideTooltip = () => (mobileTooltip ? (mobileTooltip.style.display = 'none') : null);
		const showTooltip = (contents, x, y) => {
			if (!mobileTooltip) return;

			mobileTooltip.innerHTML = contents;
			mobileTooltip.style.display = 'inline-block';

			const windowWidth = window.innerWidth;
			const windowHeight = window.innerHeight;
			const rect = mobileTooltip.getBoundingClientRect();

			const posX = x + rect.width > windowWidth ? x - (x + rect.width - windowWidth) : x;
			const posY = y + rect.height > windowHeight ? y - (y + rect.height - windowHeight) : y;

			mobileTooltip.style.left = `${posX}px`;
			mobileTooltip.style.top = `${posY}px`;
		};

		const mobileTooltipHandler = e => {
			hideTooltip();

			const closestTitle = e?.target?.title?.length ? e.target : e?.target?.closest("[title]:not([title=''])");
			if (closestTitle) {
				showTooltip(closestTitle.title.split('\n').join('<br />'), e.clientX, e.clientY);
			}
		};

		if (isTouchDevice()) {
			document.body.addEventListener('click', mobileTooltipHandler, {passive: true});
			document.addEventListener('scroll', hideTooltip, {passive: true});

			return () => {
				document.body.removeEventListener('click', mobileTooltipHandler);
				document.removeEventListener('scroll', hideTooltip);
			};
		}

		window.addEventListener('keydown', event => {
			if (event.altKey && event.code === 'KeyF') {
				$search = true;
				event.preventDefault();
				event.stopPropagation();
			}
		});
	});

	let openModal = null;
	let closeModal = null;

	$: if (mainEl) containerStore.observe(mainEl);

	if ($configStore.preferences.theme != 'default' && $configStore.preferences.theme != 'ree-dark') {
		setGlobalCSSValue('background-image', 'url(' + $configStore.preferences.bgimage + ')');
		setGlobalCSSValue('customizable-color-1', $configStore.preferences.bgColor);
		setGlobalCSSValue('customizable-color-2', $configStore.preferences.headerColor);

		setGlobalCSSValue('font-names', $configStore.preferences.fontNames);

		setGlobalCSSValue('bg-color', $configStore.preferences.buttonColor);
		setGlobalCSSValue('color', $configStore.preferences.labelColor);
		setGlobalCSSValue('ppColour', $configStore.preferences.ppColor);
		setGlobalCSSValue('selected', $configStore.preferences.selectedColor);

		importFonts($configStore.preferences.fontNames);
	}
</script>

<div bind:this={mobileTooltip} class="mobile-tooltip" />
<div class="main-background" />
<TournamentTopBanner />

<!-- <BeatCancerTopBanner /> -->
<!-- {#if $account?.player && $configStore.preferences.followersBecomingPublic}
	<div class="reebanner">
		<a class="reelink" href="/settings#profile" />
		<span class="link-text">Followers will be public, adjust your preferences!</span>
		<button
			class="close-banner"
			title="Hide banner"
			on:click|preventDefault|stopPropagation={() => {
				$configStore = produce($configStore, draft => {
					draft.preferences.followersBecomingPublic = false;
				});
			}}><i class="fas fa-xmark" /></button>
	</div>
{/if} -->
<!-- {#if $configStore.preferences.replayedbanner}
	<div class="replayedbanner">
		<a class="reelink" href="/replayed" />
		<div class="banner-spacer" />
		<img class="cover-1" src="https://eu.cdn.beatsaver.com/dd90f0e236c73c1a0565634bfd7eb168c0e81b58.jpg" />
		<span class="replayed-link-text">Your 2023 rePlayed is here!</span>

		<img class="cover-2" src="https://eu.cdn.beatsaver.com/4d03502003602e8e8f0d9f8c84a3e70da0c389f6.jpg" />
		<img class="cover-3" src="https://eu.cdn.beatsaver.com/3b2005183b2ac76c2a65f34540ebb6fd293ddb38.jpg" />
		<img class="cover-4" src="https://eu.cdn.beatsaver.com/6be290e33fd748cd678d4c38f1d59e582a44045f.jpg" />
		<img class="cover-5" src="https://eu.cdn.beatsaver.com/8931f3f17f8f5259c076660c986b816d7e86d708.jpg" />
		<img class="cover-6" src="https://eu.cdn.beatsaver.com/b97774a83bdfd669e1bd231039cccfa1162efa13.jpg" />
		<button
			class="close-banner"
			title="Hide banner"
			on:click|preventDefault|stopPropagation={() => {
				$configStore = produce($configStore, draft => {
					draft.preferences.replayedbanner = false;
				});
			}}><i class="fas fa-xmark" /></button>
	</div>
{/if} -->
<!--{#if $rewindTimer && $configStore.preferences.ccWinterHighlights24}
	<div class="rewindbanner">
		<a class="reelink" href="https://www.youtube.com/watch?v=9dr-M1hfCLo" />
		<div class="banner-spacer" />
		<img class="cc-cover-1" src="/assets/cc-logo-left.webp" />

		<div class="banner-center-text">
			{#if $rewindTimer.seconds > 0}
				<span class="replayed-link-text desktop-only">Cube Community Winter Highlights in</span>
				<span class="replayed-link-text mobile-only">CC Winter Highlights in</span>

				<div class="timer">
					<div class="rewind-time">
						<span>{padNumber($rewindTimer.hours)}</span>
						<label class="desktop-only">Hours</label>
						<label class="mobile-only">H</label>
					</div>

					<div class="rewind-time">
						<span>{padNumber($rewindTimer.minutes)}</span>
						<label class="desktop-only">Minutes</label>
						<label class="mobile-only">M</label>
					</div>

					<div class="rewind-time">
						<span>{padNumber(parseInt($rewindTimer.seconds, 10))}</span>
						<label class="desktop-only">Seconds!</label>
						<label class="mobile-only">S!</label>
					</div>
				</div>
			{:else}
				<span class="replayed-link-text">Cube Community Winter Highlights NOW! 🔴</span>
			{/if}
		</div>
		<img class="cc-cover-2" src="/assets/cc-logo-right.webp" />

		<button
			class="close-banner"
			title="Hide banner"
			on:click|preventDefault|stopPropagation={() => {
				$configStore = produce($configStore, draft => {
					draft.preferences.ccWinterHighlights24 = false;
				});
			}}><i class="fas fa-xmark" /></button>
	</div>
{/if}-->
<!-- {#if $rewindTimer && $configStore.preferences.beastiesbanner2026}
	<div class="rewindbanner">
		<a class="reelink" href="https://www.youtube.com/live/tZpEPTaWuxA" />
		<div class="banner-spacer" />
		<img class="cc-cover-1" src="/assets/beastsabericonbig.webp" />

		<div class="rewind-text-and-timer">
			{#if $rewindTimer.seconds > 0}
				<span class="replayed-link-text desktop-only">2025 BeastSaber Awards in</span>
				<span class="replayed-link-text mobile-only">Mapping Awards in</span>

				<div class="timer">
					<div class="rewind-time">
						<span>{padNumber($rewindTimer.hours)}</span>
						<label class="desktop-only">Hour{$rewindTimer.hours > 1 ? 's' : ''}</label>
						<label class="mobile-only">H</label>
					</div>

					<div class="rewind-time">
						<span>{padNumber($rewindTimer.minutes)}</span>
						<label class="desktop-only">Minute{$rewindTimer.minutes > 1 ? 's' : ''}</label>
						<label class="mobile-only">M</label>
					</div>

					<div class="rewind-time">
						<span>{padNumber(parseInt($rewindTimer.seconds, 10))}</span>
						<label class="desktop-only">Second{$rewindTimer.seconds > 1 ? 's' : ''}!</label>
						<label class="mobile-only">S!</label>
					</div>
				</div>
			{:else if $rewindTimer.hours < -2}
				<span class="replayed-link-text desktop-only">2025 BeastSaber Awards Premiered!</span>
				<span class="replayed-link-text mobile-only">Mapping Awards OUT!</span>
			{:else}
				<span class="replayed-link-text desktop-only">2025 BeastSaber Awards NOW! 🔴</span>
				<span class="replayed-link-text mobile-only">Mapping Awards NOW! 🔴</span>
			{/if}
		</div>
		<img class="cc-cover-2" src="/assets/beastsabericonbig.webp" />

		<button
			class="close-banner"
			title="Hide banner"
			on:click|preventDefault|stopPropagation={() => {
				$configStore = produce($configStore, draft => {
					draft.preferences.beastiesbanner2026 = false;
				});
			}}><i class="fas fa-xmark" /></button>
	</div>
{/if} -->
<Router {url}>
	<Nav class={$configStore?.preferences?.theme} {openModal} {closeModal} />
	<Notifications zIndex={10000} item={NotificationComponent}>
		<SimpleModal
			closeButton={false}
			styleWindow={{width: '90vw', height: '65vh'}}
			styleContent={{padding: 0, 'margin-bottom': '-0.5em'}}
			bind:openFunction={openModal}
			bind:closeFunction={closeModal}>
			<main bind:this={mainEl} class={$configStore?.preferences?.theme}>
				<div class="ssr-page-container">
					<Route path="/">
						{#if $account?.player}
							{#await pageImports.DashboardPage() then module}
								<svelte:component this={module.default} />
							{/await}
						{:else if $account?.refreshError}
							{#await pageImports.LandingPage() then module}
								<svelte:component this={module.default} />
							{/await}
						{/if}
					</Route>
					<Route path="/u/:initialPlayerId/*initialParams" let:params let:location>
						{#await pageImports.PlayerPage() then module}
							<svelte:component
								this={module.default}
								initialPlayerId={params.initialPlayerId}
								initialParams={params.initialParams}
								{location} />
						{/await}
					</Route>

					<Route path="/admin/*type" let:params let:location>
						{#await pageImports.AdminPage() then module}
							<svelte:component this={module.default} initialType={params.type} {location} />
						{/await}
					</Route>

					<Route path="/staff" let:location>
						{#await pageImports.StaffDashboard() then module}
							<svelte:component this={module.default} {location} />
						{/await}
					</Route>
					<Route path="/sotwnominations" let:location>
						{#await pageImports.NominatedScores() then module}
							<svelte:component this={module.default} {location} />
						{/await}
					</Route>
					<Route path="/beasties/nominations">
						{#await pageImports.BeastiesNominations() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/privacy">
						{#await pageImports.PrivacyPage() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/about">
						{#await pageImports.AboutPage() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/socket">
						{#await pageImports.Socket() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/settings">
						{#await pageImports.Settings() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/followed">
						{#await pageImports.FollowedPage() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/census2023">
						{#await pageImports.CensusPage() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/week100">
						{#await pageImports.Week100Page() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/survey/achievement">
						{#await pageImports.SurveyAchievementPage() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/supporting-project/link">
						{#await pageImports.PatreonPage() then module}
							<svelte:component this={module.default} action="linkPatreon" />
						{/await}
					</Route>
					<Route path="/tibytes-presets">
						{#await pageImports.TibytesPresets() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/supporting-project">
						{#await pageImports.PatreonPage() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/ranking/*page" let:params let:location>
						{#await pageImports.RankingPage() then module}
							<svelte:component this={module.default} page={params.page} {location} />
						{/await}
					</Route>
					<Route path="/scores/*page" let:params let:location>
						{#await pageImports.ScoresPage() then module}
							<svelte:component this={module.default} page={params.page} {location} />
						{/await}
					</Route>
					<Route path="/score/:scoreId" let:params let:location>
						{#await pageImports.ScorePage() then module}
							<svelte:component this={module.default} scoreId={params.scoreId} {location} />
						{/await}
					</Route>
					<Route path="/leaderboard/:type/:leaderboardId/*page" let:params let:location>
						{#await pageImports.LeaderboardPage() then module}
							<svelte:component
								this={module.default}
								leaderboardId={params.leaderboardId}
								type={params.type}
								page={params.page}
								{location}
								showCurve={true}
								separatePage={true} />
						{/await}
					</Route>
					<Route path="/leaderboard/approval/:type/:leaderboardId/*page" let:params let:location>
						{#await pageImports.LeaderboardPage() then module}
							<svelte:component
								this={module.default}
								leaderboardId={params.leaderboardId}
								type={params.type}
								page={params.page}
								{location}
								showCurve={true}
								separatePage={true}
								showApproveRequest={true} />
						{/await}
					</Route>
					<Route path="/leaderboards/*page" let:params let:location>
						{#await pageImports.LeaderboardsPage() then module}
							<svelte:component this={module.default} page={params.page} {location} />
						{/await}
					</Route>
					<Route path="/leaderboards/loloppe/*page" let:params let:location>
						{#await pageImports.LeaderboardsLoloppe() then module}
							<svelte:component this={module.default} page={params.page} {location} />
						{/await}
					</Route>

					<Route path="/maps">
						{#await pageImports.MapsPortal() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/maps/trending" let:location>
						{#await pageImports.MapsTrending() then module}
							<svelte:component this={module.default} {location} />
						{/await}
					</Route>
					<Route path="/maps/suggestions/:leaderboardId" let:params let:location>
						{#await pageImports.SongSuggestMap() then module}
							<svelte:component this={module.default} leaderboardId={params.leaderboardId} {location} />
						{/await}
					</Route>
					<Route path="/maps/suggestions" let:location>
						{#await pageImports.SongSuggestMap() then module}
							<svelte:component this={module.default} {location} />
						{/await}
					</Route>
					<Route path="/maps/:type/*page" let:params let:location>
						{#await pageImports.MapsPage() then module}
							<svelte:component this={module.default} page={params.page} type={params.type} {location} />
						{/await}
					</Route>
					<Route path="/replayed">
						{#await pageImports.ReplayedLanding() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/replayed/player/*id" let:params>
						{#await pageImports.Replayed() then module}
							<svelte:component this={module.default} playerId={params.id ? params.id : null} />
						{/await}
					</Route>
					<Route path="/replayed/mapper/*id" let:params>
						{#await pageImports.Replayed() then module}
							<svelte:component this={module.default} replayedType="mapper" playerId={params.id ? params.id : null} />
						{/await}
					</Route>
					<Route path="/event/project-tree">
						{#await pageImports.ProjectTree() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/event/building-blocks-2024" let:location>
						{#await pageImports.BuildingBlocks2024() then module}
							<svelte:component this={module.default} {location} />
						{/await}
					</Route>
					<Route path="/event/lovelive">
						{#await pageImports.LoveLivePage() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/event/rocketleaguevol2">
						{#await pageImports.RocketLeaguePackPage() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/event/gamifiedvivify" let:location>
						{#await pageImports.GamifiedVivifyPack() then module}
							<svelte:component this={module.default} {location} />
						{/await}
					</Route>
					<Route path="/event/:eventId/*page" let:params let:location>
						{#await pageImports.EventPage() then module}
							<svelte:component this={module.default} eventId={params.eventId} page={params.page} {location} />
						{/await}
					</Route>
					<Route path="/events/*page" let:params let:location>
						{#await pageImports.EventsPage() then module}
							<svelte:component this={module.default} page={params.page} {location} />
						{/await}
					</Route>
					<Route path="/badges" let:location>
						{#await pageImports.BadgesPage() then module}
							<svelte:component this={module.default} {location} />
						{/await}
					</Route>
					<Route path="/clan/:clanId/*page" let:params let:location>
						{#await pageImports.ClanPage() then module}
							<svelte:component this={module.default} clanId={params.clanId} page={params.page} {location} />
						{/await}
					</Route>
					<Route path="/clan/maps/:clanId/*page" let:params let:location>
						{#await pageImports.ClanPage() then module}
							<svelte:component this={module.default} clanId={params.clanId} page={params.page} maps={true} {location} />
						{/await}
					</Route>
					<Route path="/clans/*page" let:params let:location>
						{#await pageImports.ClansPage() then module}
							<svelte:component this={module.default} page={params.page} {location} />
						{/await}
					</Route>
					<Route path="/clansmap/leaderboard/*leaderboardId" let:params let:location>
						{#await pageImports.ClansMap() then module}
							<svelte:component this={module.default} leaderboardId={params.leaderboardId} {location} />
						{/await}
					</Route>
					<Route path="/clansmap/history/*startTimeset" let:params let:location>
						<GlobalClansMapHistory
							startTimeset={params.startTimeset.includes('/') ? params.startTimeset.split('/')[0] : params.startTimeset}
							finishTimeset={params.startTimeset.includes('/') ? params.startTimeset.split('/')[1] : null}
							{location} />
					</Route>
					<Route path="/clansmap" let:location>
						{#await pageImports.ClansMap() then module}
							<svelte:component this={module.default} {location} />
						{/await}
					</Route>
					<Route path="/songsuggestmap/leaderboard/*leaderboardId" let:params let:location>
						{#await pageImports.SongSuggestMap() then module}
							<svelte:component this={module.default} leaderboardId={params.leaderboardId} {location} />
						{/await}
					</Route>
					<Route path="/songsuggestmap" let:location>
						{#await pageImports.SongSuggestMap() then module}
							<svelte:component this={module.default} {location} />
						{/await}
					</Route>
					<Route path="/datavis/gigamap50" let:location>
						{#await pageImports.GigaMap() then module}
							<svelte:component this={module.default} {location} topCount={50} />
						{/await}
					</Route>
					<Route path="/datavis/gigamap1000" let:location>
						{#await pageImports.GigaMap() then module}
							<svelte:component this={module.default} {location} topCount={1000} />
						{/await}
					</Route>
					<Route path="/datavis/gigamap5000" let:location>
						{#await pageImports.GigaMap() then module}
							<svelte:component this={module.default} {location} topCount={5000} />
						{/await}
					</Route>
					<Route path="/clansmap/clan/*clanTag" let:params let:location>
						{#await pageImports.ClansMap() then module}
							<svelte:component this={module.default} clanTag={params.clanTag} {location} />
						{/await}
					</Route>
					<Route path="/playlists/featured/*page" let:params let:location>
						{#await pageImports.FeaturedPlaylists() then module}
							<svelte:component this={module.default} page={params.page} {location} />
						{/await}
					</Route>
					<Route path="/playlists/*id" let:params>
						{#await pageImports.PlaylistsPage() then module}
							<svelte:component this={module.default} index={params.id} />
						{/await}
					</Route>
					<Route path="/playlist/:id" let:params>
						{#await pageImports.PlaylistPage() then module}
							<svelte:component this={module.default} id={params.id} />
						{/await}
					</Route>
					<Route path="/help">
						{#await pageImports.SupportPage() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/dashboard">
						{#await pageImports.DashboardPage() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
					<Route path="/signin/*action" let:params>
						{#await pageImports.SigninPage() then module}
							<svelte:component this={module.default} action={params.action} />
						{/await}
					</Route>
					<Route path="/signin/oauth2" let:location>
						{#await pageImports.OauthSignInPage() then module}
							<svelte:component this={module.default} {location} />
						{/await}
					</Route>
					<Route path="/developer" let:params let:location>
						{#await pageImports.DeveloperPortalPage() then module}
							<svelte:component this={module.default} {location} />
						{/await}
					</Route>
					<Route path="/*">
						{#await pageImports.NotFoundPage() then module}
							<svelte:component this={module.default} />
						{/await}
					</Route>
				</div>
			</main>
		</SimpleModal>
	</Notifications>
</Router>

<PlaylistCart />
<Svrollbar />

{#if $search}
	<Search />
{/if}

<link rel="stylesheet" href="/build/themes/{$configStore.preferences.theme}.css" />

<footer class="site-footer">
	<div class="footer-inner">
		<a class="footer-brand" href="/" on:click|preventDefault={() => navigate('/')}>
			<img src="/assets/snoresaber-logo-hd.png" alt="snoresaber" />
			<span>snoresaber</span>
		</a>
		<div class="footer-links" aria-label="Footer navigation">
			<a href="/privacy" on:click|preventDefault={() => navigate('/privacy')}>Privacy</a>
			<a href="/help" on:click|preventDefault={() => navigate('/help')}>Help</a>
			<a href="/socket" on:click|preventDefault={() => navigate('/socket')}>Scores feed</a>
			<a href="https://github.com/SaberRank" target="_blank" rel="noreferrer">Source</a>
		</div>
		<span class="footer-copy">snoresaber</span>
	</div>
</footer>

<style>
	.reebanner {
		background-color: rgb(48, 23, 23);
		color: white;
		font-size: large;
		height: 3em;
		width: 100%;
		display: flex;
		justify-content: center;
		justify-items: center;
		align-items: center;
		margin-bottom: -0.1em;

		overflow: visible;
		pointer-events: none;
	}

	.replayedbanner {
		background-color: rgb(99 0 178);
		color: white;
		font-size: large;
		height: 3em;
		width: 100%;
		display: flex;
		justify-content: space-between;
		justify-items: center;
		align-items: center;
		margin-bottom: -0.1em;

		overflow: visible;
		pointer-events: none;
	}

	.rewindbanner {
		background-color: #2e0d51;
		color: white;
		font-size: large;
		height: 3em;
		width: 100%;
		display: flex;
		justify-content: space-between;
		justify-items: center;
		align-items: center;
		margin-bottom: -0.1em;

		overflow: visible;
		pointer-events: none;
	}

	.timer {
		display: flex;
		gap: 0.3em;
	}

	.rewind-text-and-timer {
		display: flex;
		gap: 0.3em;
		margin-right: 0.8em;
		justify-content: center;
	}

	.rewind-time {
		display: flex;
		gap: 0.3em;
	}

	.reelink {
		position: absolute;
		width: 100%;
		height: 3em;
		z-index: 102;
		pointer-events: auto;
	}

	.link-text {
		z-index: 101;
		font-weight: 800;
		color: cornflowerblue;
	}

	.replayed-link-text {
		z-index: 101;
		font-weight: 800;
		color: #20a0ee;
	}

	.banner-spacer {
		width: 3em;
	}

	.close-banner {
		border: none;
		color: white;
		background-color: transparent;
		cursor: pointer;
		width: 3em;
		z-index: 104;
		pointer-events: auto;
	}

	.reesaber-red {
		height: 8em;
		position: absolute;
		right: 65%;
		top: -1.1em;
		z-index: 100;
	}
	.reesaber-blue {
		height: 8em;
		position: absolute;
		left: 65%;
		top: -1.1em;
		z-index: 100;
	}

	.cover-1 {
		height: 4em;
		position: absolute;
		left: 25%;
		top: -0.7em;
		transform: rotateZ(7deg);
		z-index: 100;
		border-radius: 8px;
		box-shadow: 2px 11px 7px #0000007a;
	}

	.cover-2 {
		height: 3em;
		position: absolute;
		left: 10%;
		top: 0.5em;
		transform: rotateZ(350deg);
		z-index: 100;
		border-radius: 8px;
		box-shadow: 2px 11px 7px #0000007a;
	}

	.cover-3 {
		height: 2em;
		position: absolute;
		left: 19%;
		top: -0.2em;
		transform: rotateZ(3deg);
		z-index: 100;
		border-radius: 6px;
		box-shadow: 1px 5px 7px #0000007a;
	}

	.cover-4 {
		height: 4em;
		position: absolute;
		right: 7%;
		top: -0.7em;
		transform: rotateZ(4deg);
		z-index: 100;
		border-radius: 8px;
		box-shadow: 2px 11px 7px #0000007a;
	}

	.cover-5 {
		height: 3em;
		position: absolute;
		right: 18%;
		top: 0.6em;
		transform: rotateZ(10deg);
		z-index: 100;
		border-radius: 8px;
		box-shadow: 2px 11px 7px #0000007a;
	}

	.cover-6 {
		height: 4em;
		position: absolute;
		right: 25%;
		top: -1.4em;
		transform: rotateZ(349deg);
		z-index: 100;
		border-radius: 8px;
		box-shadow: 2px 11px 7px #0000007a;
	}

	.cc-cover-1 {
		height: 3.5em;
		position: absolute;
		left: 14%;
		z-index: 100;
	}

	.cc-cover-2 {
		height: 3.5em;
		position: absolute;
		right: 14%;
		z-index: 100;
	}

	:global(.notifications) {
		position: fixed;
		z-index: 10000;
	}

	:global(.notifications .position-top-left, .notifications .position-top-center, .notifications .position-top-right) {
		top: 3.5rem !important;
	}

	:global(.notification) {
		padding: 0;
		width: 20rem;
	}

	:global(.notification .notification-content) {
		width: auto !important;
	}

	:global(.v-scrollbar) {
		z-index: 1000;
	}

	main {
		margin-top: 1em;
	}
	.mobile-only {
		display: none;
	}
	@media (max-width: 1000px) {
		.banner-center-text {
			flex-direction: column;
			align-items: center;
			gap: 0;
		}

		.timer {
			margin-top: -0.3em;
		}
	}
	.mobile-only {
		display: none;
	}
	@media (max-width: 1000px) {
		.banner-center-text {
			flex-direction: column;
			align-items: center;
			gap: 0;
		}

		.timer {
			margin-top: -0.3em;
		}
	}
	@media (max-width: 1000px) {
		.rewind-text-and-timer {
			flex-direction: column;
			align-items: center;
			gap: 0;
		}

		.timer {
			margin-top: -0.3em;
		}
	}
	@media (max-width: 600px) {
		main {
			margin-top: 0;
		}

		.reesaber-red {
			right: 5%;
			top: -1.9em;
		}

		.reesaber-blue {
			display: none;
		}
		.cover-1 {
			left: 65%;
		}
		.cover-3 {
			left: 45%;
		}
		.cover-4 {
			display: none;
		}
		.cover-5 {
			display: none;
		}
		.cover-6 {
			display: none;
		}
		.link-text {
			color: white;
			text-shadow: 3px 3px black;
			padding: 0.6em;
		}

		.replayed-link-text {
			color: white;
			text-shadow: 3px 3px black;
			margin-bottom: 0.2em;
		}

		.banner-center-text {
			max-width: 60%;
			text-align: center;
			flex-wrap: wrap;
		}

		.cc-cover-1 {
			left: 10%;
		}

		.cc-cover-2 {
			right: 10%;
		}

		.mobile-only {
			display: block;
		}
		.desktop-only {
			display: none;
		}

		.rewind-text-and-timer {
			max-width: 60%;
			text-align: center;
			flex-wrap: wrap;
		}

		.cc-cover-1 {
			left: 10%;
		}

		.cc-cover-2 {
			right: 10%;
		}

		.mobile-only {
			display: block;
		}
		.desktop-only {
			display: none;
		}

		.site-footer {
			padding: 1.25rem 1rem 1rem;
		}

		.footer-inner {
			grid-template-columns: 1fr;
			gap: 1rem;
		}

		.footer-links {
			justify-content: flex-start;
		}

		.footer-bottom {
			align-items: flex-start;
			flex-direction: column;
			gap: 0.25rem;
		}
	}

	.ssr-page-container {
		display: grid;
		grid-template-columns: 1fr;
		grid-template-rows: 1fr;
		min-height: calc(100vh - 7.7rem);
	}

	.ssr-page-container :global(> *) {
		grid-area: 1 / 1 / 1 / 1;
	}

	.site-footer {
		margin-top: 2.5rem;
		padding: 2rem 1.5rem 1rem;
		background: linear-gradient(180deg, rgba(8, 9, 16, 0.55), rgba(5, 6, 12, 0.96));
		border-top: 1px solid rgba(255, 255, 255, 0.08);
	}

	.footer-inner {
		max-width: 1200px;
		margin: 0 auto;
		display: grid;
		grid-template-columns: minmax(240px, 1fr) minmax(360px, 1.6fr);
		column-gap: 3rem;
		row-gap: 1.5rem;
	}

	.footer-brand {
		display: flex;
		align-items: center;
		gap: 0.85rem;
		min-width: 0;
	}

	.footer-brand-mark {
		width: 2.7rem;
		height: 2.7rem;
		flex: 0 0 2.7rem;
		display: grid;
		place-items: center;
		border-radius: 0.75rem;
		background: rgba(255, 255, 255, 0.04);
		border: 1px solid rgba(255, 255, 255, 0.1);
		box-shadow: 0 8px 24px rgba(0, 0, 0, 0.25);
	overflow: hidden;
	}

	.footer-brand-mark img {
		width: 25px;
		height: 26px;
		image-rendering: auto;
	}

	.footer-brand strong {
		display: block;
		font-size: 1rem;
		letter-spacing: 0.08em;
		text-transform: uppercase;
	}

	.footer-brand span {
		display: block;
		margin-top: 0.25rem;
		font-size: 0.78rem;
		color: rgba(255, 255, 255, 0.55);
	}

	.footer-links {
		display: flex;
		justify-content: flex-end;
		align-items: center;
		align-content: center;
		flex-wrap: wrap;
		gap: 0.55rem;
	}

	.footer-links a {
		display: inline-flex;
		align-items: center;
		min-height: 2.1rem;
		padding: 0.35rem 0.75rem;
		border-radius: 0.6rem;
		color: rgba(255, 255, 255, 0.72);
		background: rgba(255, 255, 255, 0.035);
		border: 1px solid rgba(255, 255, 255, 0.07);
		font-size: 0.78rem;
		text-decoration: none;
		transition: background 120ms ease, border-color 120ms ease, color 120ms ease, transform 120ms ease;
	}

	.footer-links a:hover {
		color: #fff;
		background: rgba(255, 255, 255, 0.09);
		border-color: rgba(255, 255, 255, 0.16);
		transform: translateY(-1px);
	}

	.footer-bottom {
		grid-column: 1 / -1;
		display: flex;
		justify-content: space-between;
		align-items: center;
		padding-top: 1rem;
		border-top: 1px solid rgba(255, 255, 255, 0.06);
		font-size: 0.7rem;
		color: rgba(255, 255, 255, 0.38);
	}

	.mobile-tooltip {
		position: fixed;
		z-index: 1000;
		top: 0;
		left: 0;
		min-width: 5rem;
		max-width: 10rem;
		overflow: hidden;
		display: none;
		background-color: lightyellow;
		color: gray;
		font-size: 0.75rem;
		padding: 0.125rem;
	}

	.site-footer {
		margin-top: 3rem;
		padding: 1rem 1.25rem 1.5rem;
		background: transparent;
	}
	.footer-inner {
		max-width: 1320px; margin:0 auto; min-height:4.2rem; padding:.7rem 1rem;
		display:grid; grid-template-columns:auto 1fr auto; align-items:center; gap:1.5rem;
		border-top:1px solid rgba(255,255,255,.08);
	}
	.footer-brand { display:flex; align-items:center; gap:.65rem; color:inherit !important; text-decoration:none; font-family:'Audiowide',sans-serif; letter-spacing:.12em; }
	.footer-brand img { width:2rem; height:2rem; object-fit:contain; }
	.footer-links { display:flex; justify-content:center; align-items:center; gap:.35rem; flex-wrap:wrap; }
	.footer-links a { color:inherit !important; text-decoration:none; opacity:.72; padding:.45rem .7rem; border-radius:.55rem; }
	.footer-links a:hover { opacity:1; background:rgba(255,255,255,.06); }
	.footer-copy { opacity:.38; font-size:.75rem; letter-spacing:.08em; }
	@media (max-width: 700px) { .footer-inner { grid-template-columns:1fr; gap:.6rem; text-align:center; } .footer-brand, .footer-links { justify-content:center; } }
</style>
