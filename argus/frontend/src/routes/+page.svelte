<script lang="ts">
	import { onMount } from 'svelte';
	import type { Rat } from '$lib/types';

	let seekers: Record<string, Rat> = {};

	onMount(async () => {
		const ws = new WebSocket('ws://localhost:8080/ws/frontend');
		ws.onmessage = async (event) => {
			console.log('Message from server:', event.data);
			try {
				const message = JSON.parse(event.data);

				switch (message.type) {
					case 'Seekers':
						seekers = JSON.parse(message.data);
						console.log('Seekers data:', seekers);
						break;
					case 'SomeOtherType':
						console.log('Other message type:', message.data);
						break;

					default:
						console.warn('Unknown message type:', message.type);
				}
			} catch (err) {
				console.error('Failed to parse message:', err);
			}
		};
		return () => ws.close();
	});
</script>

<div class="grid grid-cols-1 gap-4">
	{#each Object.values(seekers) as seeker}
		<a href="/{seeker.id}" class="p-4 bg-gray-800 rounded hover:bg-gray-700">
			<h2 class="text-xl">{seeker.name}</h2>
			<p>OS: {seeker.os}</p>
			<p>IP: {seeker.ip}</p>
		</a>
	{:else}
		<p>No seeker connected.</p>
	{/each}
</div>
