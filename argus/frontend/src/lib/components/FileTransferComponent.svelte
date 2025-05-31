<script lang="ts">
	import type { Writable } from 'svelte/store';

	export let sendMessage: (type: string, data: string) => void;
	export let fileTransferOutput: Writable<string[]>;

	let fileInput: HTMLInputElement | null = null;
	let uploadPathInput = '';
	let selectedFile: File | null = null;
	let downloadPathInput = '';
	let outputDiv: HTMLDivElement | null = null;
	let isUploading = false;
	let isDownloading = false;

	function onFileSelected(event: Event) {
		const input = event.target as HTMLInputElement;
		if (input.files && input.files.length > 0) {
			selectedFile = input.files[0];
			fileTransferOutput.update((arr) => [
				...arr,
				`[Info] "${selectedFile?.name}" ready to upload`,
			]);
		} else {
			selectedFile = null;
		}
	}

	async function uploadFile() {
		if (!selectedFile) {
			fileTransferOutput.update((arr) => [
				...arr,
				'[Error] No file selected to upload',
			]);
			return;
		}

		isUploading = true;
		fileTransferOutput.update((arr) => [
			...arr,
			`[Info] Reading "${selectedFile?.name}"…`,
		]);

		const reader = new FileReader();
		reader.onload = () => {
			const raw = reader.result?.toString() || '';
			const commaIndex = raw.indexOf(',');
			const base64 = commaIndex >= 0 ? raw.slice(commaIndex + 1) : '';
			if (!base64) {
				fileTransferOutput.update((arr) => [
					...arr,
					`[Error] Could not convert "${selectedFile?.name}" to Base64.`,
				]);
				isUploading = false;
				return;
			}

			const fileData = {
				filename: selectedFile!.name,
				content: base64,
				path: uploadPathInput.trim() || '',
			};

			try {
				sendMessage('file_upload', JSON.stringify(fileData));
				fileTransferOutput.update((arr) => [
					...arr,
					`[Success] Upload request sent for "${selectedFile!.name}".`,
				]);
			} catch (err) {
				fileTransferOutput.update((arr) => [
					...arr,
					`[Error] Failed to send upload message: ${err}`,
				]);
			}

			if (fileInput) fileInput.value = '';
			selectedFile = null;
			uploadPathInput = '';
			isUploading = false;
		};

		reader.onerror = () => {
			fileTransferOutput.update((arr) => [
				...arr,
				`[Error] FileReader error on "${selectedFile!.name}".`,
			]);
			isUploading = false;
		};

		reader.readAsDataURL(selectedFile);
	}

	function downloadFile() {
		if (downloadPathInput.trim()) {
			isDownloading = true;
			sendMessage('file_download', downloadPathInput.trim());
			fileTransferOutput.update((arr) => [
				...arr,
				`[Info] Download request sent for "${downloadPathInput.trim()}".`,
			]);
			downloadPathInput = '';
			isDownloading = false;
		} else {
			fileTransferOutput.update((arr) => [
				...arr,
				'[Error] Please specify a file path to download',
			]);
		}
	}

	function clearOutput() {
		fileTransferOutput.set([]);
	}

	$: {
		if (outputDiv) {
			outputDiv.scrollTo({
				top: outputDiv.scrollHeight,
				behavior: 'smooth',
			});
		}
	}
</script>

<div class="mt-6 p-4 bg-gray-700 rounded-lg space-y-4">
	<div class="flex justify-between items-center">
		<h3 class="text-2xl font-semibold text-white">File Transfer</h3>
		<button
			on:click={clearOutput}
			class="bg-red-600 hover:bg-red-500 text-white px-4 py-2 rounded transition"
		>
			Clear Output
		</button>
	</div>

	<div class="space-y-2">
		<h4 class="text-lg font-medium text-white">Upload File to Seeker</h4>
		<div class="flex flex-col space-y-2">
			<input
				bind:this={fileInput}
				type="file"
				accept="*"
				on:change={onFileSelected}
				class="px-3 py-2 rounded bg-gray-800 text-white border border-gray-600 focus:outline-none focus:ring-2 focus:ring-blue-400"
				disabled={isUploading}
			/>
            <div class="flex space-x-2">
                <input
                    bind:value={uploadPathInput}
                    type="text"
                    placeholder="Optional save path (e.g., C:\Users\Public\file.txt)"
                    class="flex-1 px-4 py-2 rounded bg-gray-800 text-white border border-gray-600 focus:outline-none focus:ring-2 focus:ring-blue-400"
                    disabled={isUploading}
                />

                <button
                    on:click={uploadFile}
                    class="bg-blue-600 hover:bg-blue-500 text-white px-4 py-2 rounded transition w-[110px]"
                    disabled={isUploading || !selectedFile}
                > 
                    {#if isUploading}
                        Uploading…
                    {:else}
                        Upload
                    {/if}
                </button>
            </div>
		</div>
	</div>

	<div class="space-y-2">
		<h4 class="text-lg font-medium text-white">Download File from Seeker</h4>
		<div class="flex space-x-2">
			<input
				bind:value={downloadPathInput}
				type="text"
				placeholder="File path on Seeker (e.g., C:\Users\Public\file.txt)"
				class="flex-1 px-4 py-2 rounded bg-gray-800 text-white border border-gray-600 focus:outline-none focus:ring-2 focus:ring-blue-400"
				disabled={isDownloading}
				on:keydown={(e) => e.key === 'Enter' && downloadFile()}
			/>
			<button
				on:click={downloadFile}
				class="bg-blue-600 hover:bg-blue-500 text-white px-4 py-2 rounded transition w-[110px]"
				disabled={isDownloading}
			>
				Download
			</button>
		</div>
	</div>

	<div
		bind:this={outputDiv}
		class="mt-2 bg-gray-900 p-3 rounded max-h-64 overflow-y-auto font-mono text-sm text-green-200"
	>
		{#if $fileTransferOutput.length > 0}
			{#each $fileTransferOutput as line (line + Math.random())}
				<p>{line}</p>
			{/each}
		{:else}
			<p class="text-gray-500 italic">No transfer output yet.</p>
		{/if}
	</div>
</div>
