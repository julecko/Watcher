<script lang="ts">
	import type { Seeker } from '$lib/types';
	export let seeker: Seeker;
    export let connected: import('svelte/store').Writable<boolean>;
</script>

<div>
	<h2 class="text-3xl font-semibold text-white">{seeker.name}</h2>
	<div class="text-gray-300">
		<p><strong>ID:</strong> {seeker.id}</p>
		<p><strong>OS:</strong> {seeker.os}</p>
		<p><strong>IP:</strong> {seeker.ip}</p>
        <p class="text-sm font-medium">
			<strong>Status:</strong>
			<span class={$connected ? 'text-green-400' : 'text-red-400'}>
				{$connected ? 'Connected' : 'Disconnected'}
			</span>
		</p>
	</div>
	<h3 class="text-xl text-white">Metadata</h3>
	<pre
		class="bg-gray-900 text-sm text-gray-200 p-4 rounded overflow-auto max-h-64"
	>{JSON.stringify(seeker.metadata, null, 2)}</pre>
</div>