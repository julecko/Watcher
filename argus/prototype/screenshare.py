import io
import base64
import json
import threading
import time
from PIL import ImageGrab

class ScreenShare:
    def __init__(self):
        self.running = False
        self.thread = None

    def capture_frame(self):
        try:
            screenshot = ImageGrab.grab()
            buffered = io.BytesIO()
            screenshot.save(buffered, format="JPEG", quality=50)
            encoded = base64.b64encode(buffered.getvalue()).decode("utf-8")
            return encoded
        except Exception as e:
            return f"[Error] Failed to capture frame: {str(e)}"

    def screenshare_loop(self, ws_holder):
        while self.running:
            if ws_holder.ws and ws_holder.ws.sock and ws_holder.ws.sock.connected:
                frame_data = self.capture_frame()
                ws_holder.ws.send(json.dumps({
                    "type": "screenshare_frame",
                    "data": frame_data
                }))
            time.sleep(0.1)  # 10 FPS

    def start_screenshare(self, ws_holder):
        if not self.running:
            self.running = True
            self.thread = threading.Thread(target=self.screenshare_loop, args=(ws_holder,), daemon=True)
            self.thread.start()

    def stop_screenshare(self):
        self.running = False
        if self.thread:
            self.thread = None

screen_share = ScreenShare()

def start_screenshare(ws_holder):
    screen_share.start_screenshare(ws_holder)

def stop_screenshare():
    screen_share.stop_screenshare()