<script lang="ts">
	import { writable } from 'svelte/store';
	export let sendMessage: (type: string, data: string) => void;
	export let screenShareFrame: import('svelte/store').Writable<string>;
	let screenSharing = writable(false);

	function startScreenShare() {
		sendMessage('screenshare_command', 'start');
		screenSharing.set(true);
	}

	function stopScreenShare() {
		sendMessage('screenshare_command', 'stop');
		screenSharing.set(false);
	}
</script>

<div class="mt-6 p-4 bg-gray-700 rounded space-y-4">
	<div class="flex justify-between items-center">
		<h3 class="text-2xl font-semibold text-white">Screen Share</h3>
		<div class="space-x-2">
			{#if !$screenSharing}
				<button
					on:click={startScreenShare}
					class="bg-green-600 hover:bg-green-500 text-white px-4 py-2 rounded transition"
				>
					Start Screen Share
				</button>
			{:else}
				<button
					on:click={stopScreenShare}
					class="bg-red-600 hover:bg-red-500 text-white px-4 py-2 rounded transition"
				>
					Stop Screen Share
				</button>
			{/if}
		</div>
	</div>

	{#if $screenSharing}
		<div class="bg-gray-900 p-4 rounded">
			{#if $screenShareFrame}
				<img
					src="data:image/jpeg;base64,{$screenShareFrame}"
					alt="Screen Share"
					class="w-full h-auto rounded object-contain"
				/>
			{:else}
				<p class="text-gray-400 text-sm">Waiting for screen share to start...</p>
			{/if}
		</div>
	{:else}
		<div class="bg-gray-900 p-4 rounded">
			<p class="text-gray-400 text-sm">Screen share not active. Click "Start Screen Share" to begin.</p>
		</div>
	{/if}
</div>