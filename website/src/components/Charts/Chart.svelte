<script>
	import { onMount } from 'svelte';
	import { Chart as ChartJS } from 'chart.js';

	export let type = 'bar';
	export let data;
	export let options = {};

	let canvas;
	let chart;

	function renderChart() {
		if (!canvas) return;
		if (chart) chart.destroy();
		chart = new ChartJS(canvas, { type, data, options });
	}

	onMount(() => {
		renderChart();
		return () => chart && chart.destroy();
	});

	$: if (canvas && data && options) renderChart();
</script>

<canvas bind:this={canvas}></canvas>
