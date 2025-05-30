<script lang="ts">  
	export let sendMessage: (type: string, data: string) => void;

	let messageType = '';
	let messageData = '';
</script>

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
			on:click={() => sendMessage(messageType, messageData)}
			class="bg-green-600 hover:bg-green-500 text-white px-4 py-2 rounded transition"
		>
			Send Message
		</button>
	</div>
</div>