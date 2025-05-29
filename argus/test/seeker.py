import websocket
import json
import uuid
import threading
import time

seconds_disconnected = 0

def on_message(ws, message):
    print(f"Received from server: {message}")
    data = json.loads(message)
    if data.get("type") == "shell_command":
        output = f"Executed command: {data['data']}"
        ws.send(json.dumps({
            "type": "shell_output",
            "data": output
        }))

def on_error(ws, error):
    print(f"Error: {error}")

def on_close(ws, close_status_code, close_msg):
    print("WebSocket closed")

def on_open(ws):
    global seconds_disconnected
    seconds_disconnected = 0
    print("Connected to server")
    rat_data = {
        "id": str(uuid.uuid4()),
        "name": "TestRat",
        "os": "Windows",
        "ip": "192.168.1.100",
        "metadata": {"version": "1.0"}
    }
    ws.send(json.dumps(rat_data))

def run_websocket():
    global seconds_disconnected
    while True:
        ws = websocket.WebSocketApp(
            "ws://localhost:8080/ws/seeker",
            on_open=on_open,
            on_message=on_message,
            on_error=on_error,
            on_close=on_close
        )
        ws.run_forever()

        print(f"Connection lost for {seconds_disconnected} seconds. Retrying in 5 seconds...")
        time.sleep(5)
        seconds_disconnected += 5

if __name__ == "__main__":
    ws_thread = threading.Thread(target=run_websocket)
    ws_thread.daemon = True
    ws_thread.start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Shutting down")