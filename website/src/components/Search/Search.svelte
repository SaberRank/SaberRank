<script>
 import {navigate} from 'svelte-routing';
 import {search, searchValue} from '../../stores/search';
 import {onMount} from 'svelte';
 let loading=false, error='', results={players:[],maps:[]}, timer;
 function close(){ $search=false; $searchValue=''; }
 async function run(value){
  const q=value.trim(); if(q.length<2){results={players:[],maps:[]};return;}
  loading=true; error='';
  try{const r=await fetch(`/api/search?q=${encodeURIComponent(q)}`);const j=await r.json();if(!r.ok)throw Error(j.error||'Search failed');results=j;}catch(e){error=e.message||'Search failed';}finally{loading=false;}
 }
 $: if($searchValue){ clearTimeout(timer); timer=setTimeout(()=>run($searchValue),220); }
 onMount(()=>()=>clearTimeout(timer));
</script>
<main><button class="backdrop" aria-label="Close search" on:click={close}></button><section class="box"><header><div><span>SNORE SABER SEARCH</span><h2>Search everything</h2></div><button class="close" on:click={close}>×</button></header><input autofocus bind:value={$searchValue} placeholder="Player, map, or mapper…" /><div class="body">
{#if loading}<div class="state">Searching live data…</div>{:else if error}<div class="state error">{error}</div>{:else if !$searchValue}<div class="state">Start typing to search players and maps.</div>{:else if !results.players.length&&!results.maps.length}<div class="state">No matching public results.</div>{:else}
 {#if results.players.length}<h3>Players</h3>{#each results.players as p}<button class="result" on:click={()=>{close();navigate(`/u/${p.id}`)}}><img src={p.avatar||'/assets/snoresaber-icon.png'} alt=""/><span><b class="gradient">{p.name}</b><small>#{p.rank||'—'} · {Number(p.pp||0).toFixed(2)} PP · {p.country||'—'}</small></span></button>{/each}{/if}
 {#if results.maps.length}<h3>Maps</h3>{#each results.maps as m}<button class="result" on:click={()=>{close();navigate('/maps')}}><img src={m.cover||'/assets/snoresaber-icon.png'} alt=""/><span><b class="gradient">{m.name}</b><small>{m.mapper||'Unknown mapper'} · {Number(m.stars||0).toFixed(2)}★</small></span></button>{/each}{/if}
{/if}</div></section></main>
<style>
main{position:fixed;inset:0;z-index:2000}.backdrop{position:absolute;inset:0;width:100%;height:100%;border:0;background:rgba(3,4,9,.82);backdrop-filter:blur(14px);cursor:pointer}.box{position:relative;width:min(760px,calc(100% - 28px));max-height:min(760px,calc(100vh - 70px));margin:65px auto 0;border:1px solid rgba(255,255,255,.1);border-radius:22px;background:#0d0e17;box-shadow:0 35px 100px rgba(0,0,0,.65);overflow:hidden}.box header{display:flex;justify-content:space-between;padding:24px 24px 16px}.box header span{font-size:10px;letter-spacing:.22em;color:#ef7dc5;font-weight:800}.box h2{margin:6px 0 0;font-size:25px;background:linear-gradient(90deg,#fff,#f286c8,#9e84ff);-webkit-background-clip:text;color:transparent}.close{border:0;background:rgba(255,255,255,.05);color:#fff;width:38px;height:38px;border-radius:10px;font-size:25px;cursor:pointer}.box>input{display:block;width:calc(100% - 48px);height:52px;margin:0 24px 14px;box-sizing:border-box;border:1px solid rgba(180,160,255,.2);border-radius:12px;background:#080912;color:#fff;padding:0 15px;outline:none}.body{padding:0 24px 25px;overflow:auto;max-height:560px}.state{padding:30px 5px;text-align:center;color:#888697}.error{color:#ff9fbd}h3{font-size:11px;text-transform:uppercase;letter-spacing:.16em;color:#777688;margin:18px 0 8px}.result{width:100%;display:flex;align-items:center;gap:12px;border:0;border-top:1px solid rgba(255,255,255,.05);background:transparent;color:#fff;padding:10px 4px;text-align:left;cursor:pointer}.result:hover{background:rgba(255,255,255,.035)}.result img{width:43px;height:43px;border-radius:9px;object-fit:cover}.result span{min-width:0}.result b,.result small{display:block;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}.result small{margin-top:4px;color:#7e7d8d;font-size:10px}.gradient{background:linear-gradient(90deg,#fff,#f28acb,#a989ff);-webkit-background-clip:text;color:transparent}
</style>
