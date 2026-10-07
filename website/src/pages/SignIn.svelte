<script>
 import {onMount} from 'svelte';
 import {navigate} from 'svelte-routing';
 import snoreAccount, {connectScoreSaber, loadSnoreAccount, logoutSnoreAccount} from '../stores/snore-account';

 let scoreId = '';
 let error = '';
 let linking = false;
 let loading = true;
 let queryError = '';

 onMount(async () => {
   const params = new URLSearchParams(window.location.search);
   queryError = params.get('error') || '';
   await loadSnoreAccount();
   loading = false;
 });

 function steamLogin() { window.location.href = '/api/auth/steam'; }

 async function link() {
   error = '';
   const value = scoreId.trim();
   if (!/^\d+$/.test(value)) { error = 'Enter your numeric ScoreSaber player ID.'; return; }
   linking = true;
   try { await connectScoreSaber(value); scoreId = ''; }
   catch (e) { error = e?.message || 'Unable to link ScoreSaber.'; }
   finally { linking = false; }
 }
</script>

<svelte:head><title>snore saber</title></svelte:head>

<div class="auth-page">
 <div class="auth-card">
  <img src="/assets/snoresaber-icon.png" alt="" class="mark" />
  <div class="eyebrow">SNORE SABER ACCOUNT</div>
  <h1>Sign in to <span>snore saber</span></h1>
  <p class="lead">Your SnoreSaber account uses Steam to prove who you are. We never ask for or store your Steam password.</p>

  {#if queryError}<div class="error">Steam sign-in failed: {queryError.replaceAll('_',' ')}</div>{/if}

  {#if loading}
    <div class="status">Checking your SnoreSaber session…</div>
  {:else if $snoreAccount}
    <div class="signed-in">
      {#if $snoreAccount.avatarUrl}<img src={$snoreAccount.avatarUrl} alt="" />{:else}<span class="avatar">{$snoreAccount.displayName?.[0] || '?'}</span>{/if}
      <div class="identity"><strong class="gradient-name">{$snoreAccount.displayName}</strong><small>Steam {$snoreAccount.steamId}</small>{#if $snoreAccount.scoresaberId}<small>ScoreSaber {$snoreAccount.scoresaberId}</small>{/if}</div>
    </div>

    {#if !$snoreAccount.scoresaberId}
      <div class="link-box">
       <h2>Connect your ScoreSaber profile</h2>
       <p>Your Steam account and ScoreSaber account use the same SteamID. Enter it below and SnoreSaber will verify the match.</p>
       <div class="input-row"><input bind:value={scoreId} placeholder="7656119…" on:keydown={(e)=>e.key==='Enter'&&link()} /><button on:click={link} disabled={linking}>{linking ? 'Checking…' : 'Connect ScoreSaber'}</button></div>
       {#if error}<div class="error">{error}</div>{/if}
      </div>
    {:else}
      <div class="success"><span>✓</span><div><b>ScoreSaber connected</b><small>Your SnoreSaber account is linked to your competitive profile.</small></div><button on:click={()=>navigate(`/u/${$snoreAccount.scoresaberId}`)}>Open profile →</button></div>
    {/if}

    <button class="logout" on:click={logoutSnoreAccount}>Sign out</button>
  {:else}
    <button class="steam" on:click={steamLogin}>
      <svg viewBox="0 0 24 24" aria-hidden="true"><path d="M8.4 17.2a3.1 3.1 0 1 0 0-6.2 3.1 3.1 0 0 0 0 6.2Zm8.7-3.1a3.4 3.4 0 1 0-6.8 0 3.4 3.4 0 0 0 6.8 0Z"/><path d="m10.8 14.7 3.1-1.5M5.4 15.8l3.1 1.1"/></svg>
      Sign in with Steam
    </button>
    <div class="divider"><span>SECURE LOGIN</span></div>
    <ul class="points"><li>Steam verifies your identity directly.</li><li>SnoreSaber creates its own account and session.</li><li>Your public ScoreSaber profile can be linked afterward.</li></ul>
  {/if}

  <p class="fine">SnoreSaber is an independent community project. Steam authentication is handled by Valve; ScoreSaber data remains public competitive data.</p>
 </div>
</div>

<style>
 .auth-page{min-height:calc(100vh - 82px);display:grid;place-items:center;padding:55px 20px;background:radial-gradient(circle at 50% 0%,rgba(239,114,187,.12),transparent 35rem)}
 .auth-card{width:min(690px,100%);padding:44px;border:1px solid rgba(255,255,255,.09);border-radius:26px;background:linear-gradient(145deg,rgba(19,20,32,.98),rgba(8,9,16,.98));box-shadow:0 35px 100px rgba(0,0,0,.45)}
 .mark{width:62px;height:62px;object-fit:contain;margin-bottom:20px}.eyebrow{font-size:11px;letter-spacing:.24em;color:#f083c6;font-weight:800}h1{font-size:42px;margin:11px 0;background:linear-gradient(90deg,#fff,#f58acb,#9c84ff);-webkit-background-clip:text;background-clip:text;color:transparent}h1 span{font-weight:500}.lead{color:#aaa9bb;line-height:1.65;margin:0 0 30px}.steam{width:100%;height:58px;border:0;border-radius:14px;background:linear-gradient(90deg,#171b25,#24344c);color:#fff;font-size:16px;font-weight:800;cursor:pointer;box-shadow:0 10px 30px rgba(0,0,0,.25)}.steam:hover{filter:brightness(1.15)}.steam svg{width:26px;height:26px;vertical-align:middle;margin-right:9px;stroke:#fff;fill:none;stroke-width:1.5;stroke-linecap:round}.divider{display:flex;align-items:center;gap:12px;margin:25px 0 18px;color:#777688;font-size:10px;letter-spacing:.18em}.divider:before,.divider:after{content:'';height:1px;background:rgba(255,255,255,.08);flex:1}.points{margin:0;padding:0;list-style:none;display:grid;gap:11px}.points li{padding:12px 14px;border:1px solid rgba(255,255,255,.06);border-radius:11px;color:#aaa9bb;background:rgba(255,255,255,.02)}.points li:before{content:'✓';color:#f17fc5;margin-right:10px}.signed-in{display:flex;align-items:center;gap:15px;padding:15px;border-radius:16px;border:1px solid rgba(255,255,255,.08);background:rgba(255,255,255,.025)}.signed-in img,.avatar{width:58px;height:58px;border-radius:50%;object-fit:cover}.avatar{display:grid;place-items:center;background:linear-gradient(135deg,#ef72bb,#8f78ff);font-size:22px;font-weight:900}.identity{display:grid;gap:4px}.identity strong{font-size:19px}.identity small{color:#818092}.gradient-name{background:linear-gradient(90deg,#fff,#f184c8,#a080ff);-webkit-background-clip:text;background-clip:text;color:transparent}.link-box{margin-top:20px;padding:20px;border:1px solid rgba(255,255,255,.08);border-radius:16px;background:rgba(255,255,255,.02)}.link-box h2{margin:0 0 7px;font-size:18px}.link-box p{margin:0 0 16px;color:#9695a6;font-size:13px;line-height:1.5}.input-row{display:flex;gap:9px}.input-row input{flex:1;min-width:0;height:50px;border:1px solid rgba(180,160,255,.2);border-radius:11px;background:#0d0e17;color:#fff;padding:0 14px;outline:none}.input-row button,.success button{border:0;border-radius:11px;background:linear-gradient(90deg,#ef72bb,#8f78ff);color:#160b15;font-weight:800;padding:0 17px;cursor:pointer}.input-row button{height:50px}.input-row button:disabled{opacity:.55}.success{margin-top:20px;display:flex;align-items:center;gap:12px;padding:14px;border:1px solid rgba(107,240,179,.16);background:rgba(107,240,179,.05);border-radius:14px}.success>span{font-size:23px;color:#70e6b1}.success div{flex:1}.success b,.success small{display:block}.success small{margin-top:4px;color:#858496;font-size:11px}.logout{margin-top:16px;border:1px solid rgba(255,255,255,.1);background:transparent;color:#aaa9bb;border-radius:11px;height:43px;padding:0 18px;cursor:pointer}.status,.error{margin-top:15px;padding:13px;border-radius:11px}.status{color:#aaa9bb;background:rgba(255,255,255,.03)}.error{color:#ff9fbd;border:1px solid rgba(255,70,120,.2);background:rgba(255,70,120,.07)}.fine{margin:26px 0 0;padding-top:20px;border-top:1px solid rgba(255,255,255,.07);color:#6f6e7e;font-size:11px;line-height:1.6}@media(max-width:600px){.auth-card{padding:28px 21px}h1{font-size:34px}.input-row{flex-direction:column}.input-row button{width:100%}.success{flex-wrap:wrap}.success button{width:100%;height:43px}}
</style>
