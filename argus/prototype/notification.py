from win11toast import toast

def show_notification(message, title = "Notification", duration = "short"):
    toast(title, message, duration=duration)