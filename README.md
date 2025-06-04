# Watcher

**Disclaimer**: This project was built as a proof of concept for educational and security research purposes. It should only be used in controlled environments with explicit permission.

## Overview
In the past, I explored communication in small proof-of-concept programs using Discord, which I initially thought was a clever approach. At the time, my understanding of networks was limited. Since then, I’ve gained significant knowledge about networking and how professional command-and-control (C2) systems function. I created Watcher as a proof of concept to upgrade from simple Discord bot communication to proper WebSocket-based communication using a C2 server.

## Functionalities
- **Remote Shell**: Enables a remote user to execute command-line commands on the target system.
- **Keylogger**: Captures keystrokes in real time and sends them immediately to the server.
- **Screenshare**: Takes screenshots of the target system and transmits them to the remote user.
- **File Upload/Download**: Simplifies file transfers compared to shell commands. Integrates with the shell to track the current working directory (CWD) and offers it as a default path for uploads and downloads.
- **Windows Notifications**: Partially implemented on the client side but functional on the server and prototype. Further refinement is needed for full client-side support.

## Argus
Argus is a C2 server written in Go, chosen for its simple concurrency model and rapid development capabilities. It supports WebSocket connections from both frontend clients and RAT (Remote Access Trojan) clients. The server stores these connections and processes or forwards messages between the appropriate frontend and RAT clients as needed. Argus also includes a simple Python prototype of a RAT, which can be adapted for use in scenarios where traditional .exe RAT delivery is impractical due to slow internet connections.

## Seeker
Seeker is a Remote Access Tool (RAT) written in C++ using Visual Studio 2022. It is designed similarly to the Python prototype in Argus but optimized for speed, leveraging C++’s performance advantages.
## Build

In the `Argus` directory, run the following commands:

```bash
cd frontend
npm install
cd ..
./build.sh
```

To build the `Seeker`, compile it using Visual Studio.
