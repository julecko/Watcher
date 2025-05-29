<script lang="ts">
	import { onMount } from 'svelte';
	import type { Seeker } from '$lib/types';

	let seekers: Record<string, Seeker> = {};

	function formatLastActive(dateStr: string) {
		const d = new Date(dateStr);
		return d.toLocaleString(undefined, {
			year: 'numeric',
			month: 'short',
			day: 'numeric',
			hour: '2-digit',
			minute: '2-digit',
			second: '2-digit',
		});
	}

	onMount(() => {
		const ws = new WebSocket('ws://localhost:8080/ws/frontend');

		ws.onmessage = (event) => {
			try {
				const message = JSON.parse(event.data);

				if (message.type === 'Seekers') {
					seekers = JSON.parse(message.data);
				} else {
					console.warn(`Unknown message type: ${message.type}`);
				}
			} catch (err) {
				console.error('Failed to parse message:', err);
			}
		};

		return () => ws.close();
	});
</script>

{#if Object.keys(seekers).length > 0}
	<div class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-6 p-4">
		{#each Object.values(seekers) as seeker}
			<a href="/{seeker.id}" class="block p-5 bg-gray-800 rounded-lg shadow-md hover:bg-gray-700 transition-colors duration-300">
				<h2 class="text-xl font-semibold mb-2 text-white">{seeker.name}</h2>
				<p class="text-gray-300 mb-1"><span class="font-medium text-gray-400">OS:</span> {seeker.os}</p>
				<p class="text-gray-300 mb-1"><span class="font-medium text-gray-400">IP:</span> {seeker.ip}</p>
				<p class="text-gray-300 mb-2"><span class="font-medium text-gray-400">Last Active:</span> {formatLastActive(seeker.last_active)}</p>
				<p>
					<span class="font-medium text-gray-400">Status:</span> 
					<span class={`inline-block px-2 py-1 rounded text-sm font-semibold ${
						seeker.disconnected ? 'bg-red-600 text-red-100' : 'bg-green-600 text-green-100'
					}`}>
						{seeker.disconnected ? 'Disconnected' : 'Connected'}
					</span>
				</p>
			</a>
		{/each}
	</div>
{:else}
	<p class="text-center text-gray-500 mt-8 text-lg">No seeker connected.</p>
{/if}
