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

	async function setSeeker(id: string) {
		const res = await fetch(`/api/seekers`);
		if (!res.ok) {
			console.error('Fetch error:', res.status, res.statusText);
			return;
		}
		const seekers: Record<string, Seeker> = await res.json();
		seeker = seekers[id] ?? null;
        connected.set(!seeker.disconnected);
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
	<div class="p-6 bg-gray-800 rounded shadow-lg space-y-4">
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
