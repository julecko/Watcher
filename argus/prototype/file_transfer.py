import base64
import os
import json


def save_file(file_data, shell_process):
    try:
        filename = file_data.get("filename", "")
        content = file_data.get("content", "")
        path = file_data.get("path", "")

        if not filename or not content:
            return "[Error] Missing filename or content"

        file_content = base64.b64decode(content)

        if path.endswith(os.sep) or os.path.isdir(path):
            save_path = os.path.join(path, filename)
        else:
            if not path.lower().endswith(filename.lower()):
                save_path = os.path.join(path, filename)
            else:
                save_path = path

        save_path = os.path.abspath(save_path)
        os.makedirs(os.path.dirname(save_path), exist_ok=True)

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