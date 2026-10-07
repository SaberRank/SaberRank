<script>
	import {navigate} from 'svelte-routing';
	import account, {setSnoreProfile} from '../stores/snore-account';

	let query = '';
	let loading = false;
	let error = '';
	let found = null;

	async function connect() {
		error = '';
		found = null;
		const value = query.trim();
		if (!value) return;
		loading = true;
		try {
			const endpoint = /^\d+$/.test(value)
				? `/api/scoresaber/v2/players/${encodeURIComponent(value)}`
				: `/api/scoresaber/v2/players/vanity/${encodeURIComponent(value)}`;
			const response = await fetch(endpoint);
			if (!response.ok) throw new Error(response.status === 404 ? 'Player not found.' : `ScoreSaber returned ${response.status}.`);
			found = await response.json();
		} catch (e) {
			error = e?.message || 'Could not find that player.';
		} finally { loading = false; }
	}

	function save() {
		if (!found) return;
		const profile = {
			id: found.id || found.playerId,
			name: found.name || found.playerNameInGame || 'Player',
			country: found.country || '',
			avatar: found.avatar || found.avatarUrl || '',
			rank: found.rank || found.stats?.rank || 0,
			pp: found.pp || found.stats?.pp || 0,
		};
		setSnoreProfile(profile);
		navigate(`/u/${profile.id}`);
	}
</script>

<svelte:head><title>snore saber</title></svelte:head>

<div class="auth-page">
	<div class="auth-card">
		<img src="/assets/snoresaber-icon.png" alt="" class="mark" />
		<div class="eyebrow">SNORE SABER PROFILE</div>
		<h1>Connect your player</h1>
		<p class="lead">Use your ScoreSaber player ID or vanity name. SnoreSaber stores only the public profile you choose to connect.</p>

		<div class="field">
			<label for="player">ScoreSaber player</label>
			<div class="input-row">
				<input id="player" bind:value={query} on:keydown={(e) => e.key === 'Enter' && connect()} placeholder="Player ID or vanity name" autocomplete="off" />
				<button on:click={connect} disabled={loading}>{loading ? 'Finding…' : 'Find player'}</button>
			</div>
		</div>

		{#if error}<div class="error">{error}</div>{/if}

		{#if found}
			<div class="preview">
				{#if found.avatar || found.avatarUrl}<img src={found.avatar || found.avatarUrl} alt="" />{:else}<span>{(found.name || '?')[0]}</span>{/if}
			<div class="preview-copy"><b>{found.name || found.playerNameInGame}</b><small>#{found.rank || found.stats?.rank || '—'} · {Number(found.pp || found.stats?.pp || 0).toFixed(2)} PP</small></div>
			<button class="connect" on:click={save}>Use this profile</button>
			</div>
		{/if}

		<div class="note"><strong>No password is requested here.</strong> This is a SnoreSaber profile connection, not a copy of ScoreSaber authentication.</div>
	</div>
</div>

<style>
	.auth-page{min-height:calc(100vh - 82px);display:grid;place-items:center;padding:50px 20px;background:radial-gradient(circle at 50% 0%,rgba(239,114,187,.12),transparent 34rem)}
	.auth-card{width:min(700px,100%);padding:42px;border:1px solid rgba(255,255,255,.09);border-radius:24px;background:linear-gradient(145deg,rgba(19,20,32,.98),rgba(9,10,17,.98));box-shadow:0 30px 90px rgba(0,0,0,.4)}
	.mark{width:62px;height:62px;object-fit:contain;margin-bottom:20px}.eyebrow{font-size:11px;letter-spacing:.24em;color:#f083c6;font-weight:800}h1{font-size:42px;margin:10px 0 10px;background:linear-gradient(90deg,#fff,#f58acb,#9c84ff);-webkit-background-clip:text;background-clip:text;color:transparent}.lead{color:#aaa9bb;line-height:1.6;margin-bottom:30px}.field label{display:block;font-size:13px;margin-bottom:9px;color:#d9d6e2}.input-row{display:flex;gap:10px}.input-row input{flex:1;min-width:0;height:50px;border:1px solid rgba(180,160,255,.2);border-radius:12px;background:#0d0e17;color:#fff;padding:0 15px;outline:none}.input-row input:focus{border-color:#ef72bb;box-shadow:0 0 0 3px rgba(239,114,187,.1)}.input-row button,.connect{border:0;border-radius:12px;background:linear-gradient(90deg,#ef72bb,#8f78ff);color:#130b15;font-weight:800;padding:0 20px;cursor:pointer}.input-row button{height:50px}.input-row button:disabled{opacity:.55;cursor:wait}.error{margin-top:16px;padding:12px;border-radius:10px;background:rgba(255,70,120,.09);border:1px solid rgba(255,70,120,.2);color:#ff9ebd}.preview{display:flex;align-items:center;gap:13px;margin-top:22px;padding:13px;border:1px solid rgba(255,255,255,.08);border-radius:15px;background:rgba(255,255,255,.025)}.preview img,.preview>span{width:48px;height:48px;border-radius:50%;object-fit:cover}.preview>span{display:grid;place-items:center;background:linear-gradient(135deg,#ef72bb,#8f78ff);font-weight:900}.preview-copy{flex:1;min-width:0}.preview-copy b,.preview-copy small{display:block}.preview-copy small{margin-top:4px;color:#9695a6;font-size:11px}.connect{height:42px}.note{margin-top:25px;padding-top:20px;border-top:1px solid rgba(255,255,255,.07);font-size:12px;line-height:1.55;color:#888797}.note strong{color:#c6c3d0}@media(max-width:600px){.auth-card{padding:28px 22px}h1{font-size:33px}.input-row{flex-direction:column}.input-row button{width:100%}.preview{flex-wrap:wrap}.connect{width:100%}}
</style>
