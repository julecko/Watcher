<script lang="ts">
	import { onMount, afterUpdate } from 'svelte';
	import { writable } from 'svelte/store';
	export let sendMessage: (type: string, data: string) => void;
	export let shellOutput: import('svelte/store').Writable<string[]>;
    const shellRunning = writable(false);

	let shellInput = '';
	let outputDiv: HTMLDivElement;
	const default_height = 256;
	let height = default_height;

	function startShell() {
		sendMessage('shell_command', 'start');
		shellRunning.set(true);
		shellOutput.set([]);
	}

	function stopShell() {
		sendMessage('shell_command', 'stop');
		shellRunning.set(false);
		shellOutput.set([]);
		height = default_height;
	}

	function sendShellInput() {
		if ($shellRunning) {
			sendMessage('shell_input', shellInput);
			shellInput = '';
		}
	}

	afterUpdate(() => {
		if (outputDiv) {
			outputDiv.scrollTo({ top: outputDiv.scrollHeight, behavior: 'smooth' });
		}
	});

	let isResizing = false;
	let startY: number;
	let startHeight: number;

	function startResize(event: MouseEvent) {
		isResizing = true;
		startY = event.clientY;
		startHeight = height;
	}

	function onMouseMove(event: MouseEvent) {
		if (isResizing) {
			const delta = event.clientY - startY;
			const newHeight = startHeight + delta;
			height = Math.max(128, Math.min(512, newHeight));
		}
	}

	function stopResize() {
		isResizing = false;
	}

	onMount(() => {
		window.addEventListener('mousemove', onMouseMove);
		window.addEventListener('mouseup', stopResize);
		return () => {
			window.removeEventListener('mousemove', onMouseMove);
			window.removeEventListener('mouseup', stopResize);
		};
	});
</script>

<div class="mt-6 p-4 bg-gray-700 rounded space-y-4">
	<div class="flex justify-between items-center">
		<h3 class="text-2xl font-semibold text-white">Interactive Shell</h3>
		<div class="space-x-2">
			{#if !$shellRunning}
				<button
					on:click={startShell}
					class="bg-green-600 hover:bg-green-500 text-white px-4 py-2 rounded transition"
				>
					Start Shell
				</button>
			{:else}
				<button
					on:click={stopShell}
					class="bg-red-600 hover:bg-red-500 text-white px-4 py-2 rounded transition"
				>
					Stop Shell
				</button>
			{/if}
		</div>
	</div>

	{#if $shellRunning}
		<div class="flex space-x-2">
			<input
				bind:value={shellInput}
				type="text"
				placeholder="Type shell command…"
				class="flex-1 px-3 py-2 rounded bg-gray-800 text-white border border-gray-600 focus:outline-none focus:ring focus:border-blue-400"
				on:keydown={(e) => {
					if (e.key === 'Enter') {
						sendShellInput();
					}
				}}
			/>
			<button
				on:click={sendShellInput}
				class="bg-blue-600 hover:bg-blue-500 text-white px-4 py-2 rounded transition"
			>
				Send
			</button>
		</div>
	{/if}

	<div
		bind:this={outputDiv}
		class="mt-2 bg-gray-900 p-3 rounded overflow-y-auto font-mono text-sm text-green-200"
		style="height: {height}px; min-height: 128px; max-height: 512px;"
	>
		{#if $shellOutput.length > 0}
			{#each $shellOutput as line (line + Math.random())}
				<p>{line}</p>
			{/each}
		{:else}
			<p class="text-gray-500 italic">No shell output yet.</p>
		{/if}
	</div>
	<div
		class="h-2 bg-gray-600 cursor-ns-resize hover:bg-gray-500 transition-colors"
		role="button"
		tabindex="0"
		on:mousedown={startResize}
	></div>
</div>
