<script>
	import {fade} from 'svelte/transition';
	import Button from '../components/Common/Button.svelte';
	import ContentBox from '../components/Common/ContentBox.svelte';
	import {MetaTags} from 'svelte-meta-tags';
	import {CURRENT_URL} from '../network/queues/saberrank/api-queue';
	import ssrConfig from '../ssr-config';

	$: metaDescription =
		ssrConfig.name +
		' is a competitive Beat Saber leaderboard focused on scores, rankings, progression, and player performance.';
</script>

<svelte:head>
	<title>SaberRank — Competitive Beat Saber Rankings</title>
	<meta name="description" content={metaDescription} />
</svelte:head>

<article class="page-content" transition:fade|global>
	<div class="sr-home">
		<section class="sr-hero">
			<div class="hero-grid" aria-hidden="true"></div>
			<div class="hero-glow hero-glow-one" aria-hidden="true"></div>
			<div class="hero-glow hero-glow-two" aria-hidden="true"></div>

			<div class="hero-copy">
				<div class="eyebrow"><span></span> COMPETE • IMPROVE • CLIMB</div>
				<img class="hero-wordmark" src="/assets/SaberRank/wordmark.webp" alt="SaberRank" />
				<h1>Competitive Beat Saber rankings.</h1>
				<p>
					Track your scores, compare performances, and climb the SaberRank ladder.
					Everything you need to see how you stack up.
				</p>

				<div class="hero-actions">
					<a href="https://github.com/SaberRank/saberrank-mod/releases" target="_blank" rel="noreferrer">
						<Button iconFa="fas fa-download" type="green" label="Get the PC mod" />
					</a>
					<a href="https://github.com/SaberRank/saberrank-qmod/releases" target="_blank" rel="noreferrer">
						<Button iconFa="fas fa-download" type="green" label="Get the Quest mod" />
					</a>
				</div>

				<div class="hero-links">
					<a href="/ranking/1">View global rankings</a>
					<span>•</span>
					<a href="/maps">Browse maps</a>
				</div>
			</div>

			<div class="hero-preview">
				<div class="preview-header">
					<div>
						<span class="preview-kicker">LIVE STANDINGS</span>
						<strong>Global Rankings</strong>
					</div>
					<span class="live-dot"></span>
				</div>

				<div class="preview-player first">
					<span class="place">01</span>
					<div class="avatar-placeholder">N</div>
					<div class="player-copy"><strong>NeonBlade</strong><small>18,492 PP</small></div>
					<span class="score-accent">#1</span>
				</div>
				<div class="preview-player">
					<span class="place">02</span>
					<div class="avatar-placeholder cyan">P</div>
					<div class="player-copy"><strong>Pulse</strong><small>18,155 PP</small></div>
					<span class="score-accent">#2</span>
				</div>
				<div class="preview-player">
					<span class="place">03</span>
					<div class="avatar-placeholder violet">A</div>
					<div class="player-copy"><strong>ArcByte</strong><small>17,921 PP</small></div>
					<span class="score-accent">#3</span>
				</div>

				<div class="preview-footer">
					<span>Performance</span>
					<strong>98.42%</strong>
					<span class="footer-pp">+412 PP</span>
				</div>
			</div>
		</section>

		<section class="sr-intro">
			<div class="section-heading">
				<span class="section-number">01</span>
				<div>
					<span class="section-kicker">THE SABERRANK SYSTEM</span>
					<h2>Built around the scores that matter.</h2>
				</div>
			</div>

			<div class="feature-grid">
				<ContentBox cls="sr-feature">
					<div class="feature-icon rank-icon">01</div>
					<div>
						<span class="feature-kicker">RANKINGS</span>
						<h3>Know where you stand.</h3>
						<p>Compare players, follow your position, and see the scores pushing the leaderboard forward.</p>
					</div>
					<img src="/assets/SaberRank/leaderboard.webp" alt="SaberRank global rankings preview" />
				</ContentBox>

				<ContentBox cls="sr-feature">
					<div class="feature-icon score-icon">02</div>
					<div>
						<span class="feature-kicker">PERFORMANCE</span>
						<h3>Understand every score.</h3>
						<p>Review accuracy, performance points, and score history without digging through clutter.</p>
					</div>
					<img src="/assets/SaberRank/score.webp" alt="SaberRank score analysis preview" />
				</ContentBox>
			</div>
		</section>

		<section class="sr-pillars">
			<div class="pillar">
				<span>01</span>
				<strong>Player-first</strong>
				<p>Your profile is the center of your progression.</p>
			</div>
			<div class="pillar">
				<span>02</span>
				<strong>Score-driven</strong>
				<p>Clear numbers, meaningful rankings, useful detail.</p>
			</div>
			<div class="pillar">
				<span>03</span>
				<strong>Community-ready</strong>
				<p>Designed to grow with the SaberRank community.</p>
			</div>
		</section>

		<section class="sr-cta">
			<div>
				<span class="section-kicker">READY?</span>
				<h2>Put your scores on the board.</h2>
				<p>Install SaberRank, play your maps, and start climbing.</p>
			</div>
			<a href="/ranking/1" class="cta-link">Explore rankings <span>→</span></a>
		</section>
	</div>
</article>

<MetaTags
	title="SaberRank — Competitive Beat Saber Rankings"
	description={metaDescription}
	openGraph={{
		title: 'SaberRank — Competitive Beat Saber Rankings',
		description: metaDescription,
		images: [{url: CURRENT_URL + '/assets/SaberRank/icon.webp'}],
		siteName: 'SaberRank',
	}}
	twitter={{
		handle: '@saberrank_',
		site: '@saberrank_',
		cardType: 'summary',
		title: 'SaberRank — Competitive Beat Saber Rankings',
		description: metaDescription,
		image: CURRENT_URL + '/assets/SaberRank/icon.webp',
		imageAlt: 'SaberRank logo',
	}} />

<style>
	:global(.sr-home) {
		max-width: 1400px;
		margin: 0 auto;
		padding: 1.25rem 1.25rem 4rem;
		color: #f5f7ff;
	}

	.sr-hero {
		min-height: 620px;
		position: relative;
		overflow: hidden;
		border: 1px solid rgba(91, 216, 255, 0.18);
		border-radius: 26px;
		background:
			linear-gradient(115deg, rgba(8, 12, 25, 0.98) 0%, rgba(10, 12, 31, 0.92) 48%, rgba(25, 8, 35, 0.84) 100%),
			url('/assets/SaberRank/hero.webp') center / cover;
		box-shadow: 0 30px 90px rgba(0, 0, 0, 0.4);
		display: grid;
		grid-template-columns: 1.2fr 0.8fr;
		align-items: center;
		padding: 4.5rem;
		gap: 3rem;
	}

	.hero-grid {
		position: absolute;
		inset: 0;
		background-image: linear-gradient(rgba(255,255,255,.025) 1px, transparent 1px), linear-gradient(90deg, rgba(255,255,255,.025) 1px, transparent 1px);
		background-size: 48px 48px;
		mask-image: linear-gradient(to right, black, transparent 85%);
	}

	.hero-glow {
		position: absolute;
		border-radius: 50%;
		filter: blur(80px);
		pointer-events: none;
	}

	.hero-glow-one {
		width: 340px;
		height: 340px;
		right: 8%;
		top: -100px;
		background: rgba(255, 24, 181, .18);
	}

	.hero-glow-two {
		width: 280px;
		height: 280px;
		left: 35%;
		bottom: -120px;
		background: rgba(45, 207, 255, .14);
	}

	.hero-copy,
	.hero-preview {
		position: relative;
		z-index: 1;
	}

	.eyebrow,
	.section-kicker,
	.feature-kicker,
	.preview-kicker {
		font-size: .72rem;
		letter-spacing: .25em;
		font-weight: 800;
		color: #58d9ff;
	}

	.eyebrow {
		display: flex;
		align-items: center;
		gap: .6rem;
	}

	.eyebrow span,
	.live-dot {
		width: 8px;
		height: 8px;
		border-radius: 50%;
		background: #ff2abf;
		box-shadow: 0 0 16px #ff2abf;
	}

	.hero-wordmark {
		display: block;
		width: min(520px, 100%);
		height: auto;
		margin: 1.25rem 0 .6rem;
		object-fit: contain;
		object-position: left center;
	}

	.hero-copy h1 {
		font-size: clamp(2rem, 4vw, 3.8rem);
		line-height: 1.05;
		max-width: 680px;
		margin: 0;
	}

	.hero-copy p {
		max-width: 610px;
		color: #b8c2d8;
		font-size: 1.08rem;
		line-height: 1.7;
		margin: 1.25rem 0;
	}

	.hero-actions {
		display: flex;
		flex-wrap: wrap;
		gap: .75rem;
		margin-top: 1.6rem;
	}

	.hero-links {
		display: flex;
		gap: .7rem;
		margin-top: 1.25rem;
		font-size: .9rem;
		color: #8995ae;
	}

	.hero-links a:hover { color: #fff; }

	.hero-preview {
		padding: 1.2rem;
		border: 1px solid rgba(255,255,255,.1);
		border-radius: 20px;
		background: rgba(7, 11, 23, .76);
		backdrop-filter: blur(18px);
		box-shadow: 0 25px 60px rgba(0,0,0,.35);
	}

	.preview-header {
		display: flex;
		justify-content: space-between;
		align-items: center;
		padding: .5rem .5rem 1rem;
		border-bottom: 1px solid rgba(255,255,255,.08);
	}

	.preview-header div { display: flex; flex-direction: column; gap: .25rem; }
	.preview-header strong { font-size: 1.25rem; }
	.live-dot { display: block; }

	.preview-player {
		display: grid;
		grid-template-columns: 2.5rem 2.6rem 1fr auto;
		gap: .7rem;
		align-items: center;
		padding: .8rem .55rem;
		border-bottom: 1px solid rgba(255,255,255,.055);
	}

	.preview-player.first {
		background: linear-gradient(90deg, rgba(255,42,191,.09), transparent);
	}

	.place { color: #68748c; font-weight: 800; }
	.avatar-placeholder {
		width: 2.35rem;
		height: 2.35rem;
		display: grid;
		place-items: center;
		border-radius: 10px;
		background: linear-gradient(135deg,#ff2abf,#8d2cff);
		font-weight: 900;
	}
	.avatar-placeholder.cyan { background: linear-gradient(135deg,#2ed7ff,#376cff); }
	.avatar-placeholder.violet { background: linear-gradient(135deg,#9a52ff,#ff2abf); }
	.player-copy { display: flex; flex-direction: column; gap: .15rem; }
	.player-copy small { color: #7f8ba3; }
	.score-accent { color: #58d9ff; font-weight: 800; }

	.preview-footer {
		display: grid;
		grid-template-columns: 1fr auto auto;
		gap: 1rem;
		padding: 1rem .55rem .4rem;
		color: #7f8ba3;
	}
	.preview-footer strong { color: #fff; }
	.footer-pp { color: #ff48c5; font-weight: 800; }

	.sr-intro { padding: 5rem 0 2rem; }
	.section-heading { display: flex; gap: 1rem; align-items: flex-start; margin-bottom: 2rem; }
	.section-number { color: #ff2abf; font-weight: 900; font-size: .9rem; padding-top: .25rem; }
	.section-heading h2, .sr-cta h2 { font-size: clamp(2rem, 3vw, 3rem); margin: .3rem 0 0; line-height: 1.05; }

	.feature-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 1.25rem; }
	:global(.sr-feature) {
		position: relative;
		overflow: hidden;
		min-height: 450px;
		padding: 2rem !important;
		border: 1px solid rgba(255,255,255,.07);
		background: linear-gradient(145deg, rgba(16,22,40,.96), rgba(8,12,25,.96)) !important;
	}
	.feature-icon {
		width: 3rem;
		height: 3rem;
		display: grid;
		place-items: center;
		border-radius: 12px;
		font-weight: 900;
		margin-bottom: 2rem;
	}
	.rank-icon { background: rgba(55,210,255,.12); color: #58d9ff; }
	.score-icon { background: rgba(255,42,191,.12); color: #ff48c5; }
	.sr-feature h3 { font-size: 1.65rem; margin: .5rem 0; }
	.sr-feature p { max-width: 500px; color: #8f9ab1; line-height: 1.6; }
	.sr-feature img {
		width: 78%;
		display: block;
		margin: 1.5rem auto -1rem;
		border-radius: 14px;
		box-shadow: 0 18px 40px rgba(0,0,0,.35);
	}

	.sr-pillars {
		display: grid;
		grid-template-columns: repeat(3, 1fr);
		gap: 1px;
		margin: 3rem 0 4rem;
		border: 1px solid rgba(255,255,255,.07);
		border-radius: 18px;
		overflow: hidden;
		background: rgba(255,255,255,.07);
	}
	.pillar { background: #0b1020; padding: 2rem; }
	.pillar span { color: #ff2abf; font-size: .75rem; font-weight: 900; }
	.pillar strong { display: block; font-size: 1.25rem; margin: .7rem 0 .35rem; }
	.pillar p { color: #818ca4; line-height: 1.5; margin: 0; }

	.sr-cta {
		display: flex;
		justify-content: space-between;
		align-items: center;
		gap: 2rem;
		padding: 2.5rem 3rem;
		border-radius: 20px;
		background: linear-gradient(100deg, rgba(255,42,191,.12), rgba(48,205,255,.08));
		border: 1px solid rgba(255,255,255,.09);
	}
	.sr-cta p { color: #8995ae; margin: .75rem 0 0; }
	.cta-link {
		flex: 0 0 auto;
		padding: .9rem 1.25rem;
		border-radius: 10px;
		background: #ff2abf;
		color: #fff;
		font-weight: 800;
		box-shadow: 0 10px 30px rgba(255,42,191,.2);
	}
	.cta-link span { margin-left: .6rem; }

	@media (max-width: 950px) {
		.sr-hero { grid-template-columns: 1fr; padding: 3rem 2rem; }
		.hero-preview { max-width: 620px; width: 100%; }
	}
	@media (max-width: 700px) {
		:global(.sr-home) { padding: .75rem .75rem 3rem; }
		.sr-hero { min-height: 0; padding: 2.25rem 1.25rem; border-radius: 18px; }
		.hero-wordmark { width: 100%; }
		.hero-copy h1 { font-size: 2rem; }
		.hero-links { flex-wrap: wrap; }
		.feature-grid, .sr-pillars { grid-template-columns: 1fr; }
		:global(.sr-feature) { min-height: 0; }
		.sr-feature img { width: 100%; }
		.sr-cta { flex-direction: column; align-items: flex-start; padding: 2rem; }
	}
</style>
