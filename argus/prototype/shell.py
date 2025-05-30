import json
import threading
import subprocess

shell_process = None
shell_thread = None
shell_running = False
shell_lock = threading.Lock()

def shell_reader(ws, process):
    while True:
        output = process.stdout.readline()
        if output == '' and process.poll() is not None:
            break
        if output:
            try:
                msg = json.dumps({"type": "shell_output", "data": output.rstrip()})
                ws.send(msg)
            except Exception as e:
                print(f"[Error] Failed to send shell output: {e}")
                break

def start_shell(ws):
    global shell_process, shell_thread, shell_running

    if shell_running:
        ws.send(json.dumps({"type": "shell_output", "data": "[Info] Shell session already running."}))
        return

    try:
        shell_process = subprocess.Popen(
            ["cmd.exe"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            bufsize=1,
            universal_newlines=True,
        )
    except Exception as e:
        ws.send(json.dumps({"type": "shell_output", "data": f"[Error] Failed to start shell: {e}"}))
        return

    shell_running = True
    shell_thread = threading.Thread(target=shell_reader, args=(ws, shell_process), daemon=True)
    shell_thread.start()
    ws.send(json.dumps({"type": "shell_output", "data": "[Info] Shell started."}))

def stop_shell(ws):
    global shell_process, shell_running
    if shell_running and shell_process:
        try:
            shell_process.terminate()
            shell_process.wait(timeout=2)
        except Exception:
            pass
        shell_running = False
        ws.send(json.dumps({"type": "shell_output", "data": "[Info] Shell stopped."}))