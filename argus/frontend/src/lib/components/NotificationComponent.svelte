<script lang="ts">
	export let sendMessage: (type: string, data: string) => void;

	let messageInput: string = '';
	let titleInput: string = '';
	let isSending: boolean = false;

	function sendNotification() {
		if (!messageInput.trim() || !titleInput.trim()) {
			return;
		}

		isSending = true;
		const notificationData = JSON.stringify({
			message: messageInput.trim(),
			title: titleInput.trim()
		});

		try {
			sendMessage('notification', notificationData);
			messageInput = '';
			titleInput = '';
		} catch (err) {
			alert(`Error happened ${err}`);
		} finally {
			isSending = false;
		}
	}
</script>

<div class="mt-6 p-4 bg-gray-700 rounded-lg space-y-4">
	<div class="space-y-2">
		<h3 class="text-2xl font-semibold text-white">Notification Details</h3>
		<div class="flex flex-col space-y-2">
			<input
				bind:value={titleInput}
				type="text"
				placeholder="Notification title"
				class="flex-1 px-4 py-2 rounded bg-gray-800 text-white border border-gray-600 focus:outline-none focus:ring-2 focus:ring-blue-400"
				disabled={isSending}
				on:keydown={(e) => e.key === 'Enter' && sendNotification()}
			/>
			<div class="flex space-x-2">
				<input
					bind:value={messageInput}
					type="text"
					placeholder="Notification message"
					class="flex-1 px-4 py-2 rounded bg-gray-800 text-white border border-gray-600 focus:outline-none focus:ring-2 focus:ring-blue-400"
					disabled={isSending}
					on:keydown={(e) => e.key === 'Enter' && sendNotification()}
				/>
				<button
					on:click={sendNotification}
					class="bg-blue-600 hover:bg-blue-500 text-white px-4 py-2 rounded transition w-[110px]"
					disabled={isSending}
				>
					{#if isSending}
						Sending…
					{:else}
						Send
					{/if}
				</button>
			</div>
		</div>
	</div>
</div>