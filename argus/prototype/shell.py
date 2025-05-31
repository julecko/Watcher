import subprocess
import threading
import json

class Shell:
    def __init__(self):
        self.shell_running = False
        self.shell_process = None
        self.shell_lock = threading.Lock()

    def read_shell_output(self, ws, pipe):
        for line in iter(pipe.readline, ''):
            if not self.shell_running:
                break
            if line:
                ws.send(json.dumps({"type": "shell_output", "data": line.rstrip()}))

    def start_shell(self, ws):
        if not self.shell_running:
            self.shell_running = True
            try:
                self.shell_process = subprocess.Popen(
                    ['cmd.exe'],
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True,
                    encoding='utf-8',
                    errors='replace'
                )
                threading.Thread(
                    target=self.read_shell_output,
                    args=(ws, self.shell_process.stdout),
                    daemon=True
                ).start()
                threading.Thread(
                    target=self.read_shell_output,
                    args=(ws, self.shell_process.stderr),
                    daemon=True
                ).start()
            except Exception as e:
                ws.send(json.dumps({"type": "shell_output", "data": f"[Error] Failed to start shell: {e}"}))
                self.shell_running = False
                self.shell_process = None

    def stop_shell(self, ws):
        if self.shell_running:
            self.shell_running = False
            if self.shell_process:
                try:
                    self.shell_process.terminate()
                    self.shell_process.wait(timeout=1)
                except Exception as e:
                    ws.send(json.dumps({"type": "shell_output", "data": f"[Error] Failed to stop shell: {e}"}))
                self.shell_process = None

shell = Shell()