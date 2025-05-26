# Argus

A monitoring and control system with a Go backend and Svelte (TypeScript) frontend, designed for Windows.

## Setup

1. **Install Dependencies**:
   ```cmd
   go mod tidy
   cd frontend
   npm install
   npm install -D typescript
   cd ..\rat_client
   pip install websocket-client
   ```

2. **Build**:
   ```cmd
   build.bat
   ```

3. **Run Server**:
   ```cmd
   bin\argus.exe
   ```

4. **Run Rat Client**:
   ```cmd
   cd rat_client
   python rat.py
   ```

## Routes
- `/`: List of connected rats.
- `/:id`: Details of a specific rat.
- `/:id/shell`: Reverse shell for a rat (uses `cmd.exe`).

## Notes
- Ensure Go, Node.js, and Python are installed and added to your PATH.
- The frontend uses TypeScript for type safety; types are defined in `frontend/src/lib/types.ts`.
- WebSocket server runs on `ws://localhost:8080/ws`.
- Use Windows Command Prompt for running commands.
- Example shell commands: `dir`, `type file.txt`, `echo %PATH%`.