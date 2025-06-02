package main

import (
	"argus/backend/handlers"
	"log"
	"net/http"
	"os"
	"os/signal"
	"path/filepath"
	"syscall"
)

func main() {
	handlers.LoadSeekersFromFile()

	staticPath := filepath.Join("backend", "static")
	http.Handle("/_app/", http.StripPrefix("/_app/", http.FileServer(http.Dir(filepath.Join(staticPath, "_app")))))
	http.HandleFunc("/", handlers.ServeFrontend)

	http.HandleFunc("/ws/seeker", handlers.SeekerWebSocketHandler)
	http.HandleFunc("/ws/frontend", handlers.FrontendWebSocketHandler)
	http.HandleFunc("/ws/frontend/", handlers.FrontendWebSocketHandler)

	http.HandleFunc("/api/seekers", handlers.GetSeekers)
	http.HandleFunc("/api/remove-seeker", handlers.RemoveSeeker)

	go func() {
		c := make(chan os.Signal, 1)
		signal.Notify(c, os.Interrupt, syscall.SIGTERM)
		<-c
		log.Println("Shutting down... Saving seekers...")
		handlers.SaveSeekersToFile()
		os.Exit(0)
	}()

	log.Println("Server starting on http://localhost:8010")
	if err := http.ListenAndServe(":8010", nil); err != nil {
		log.Fatal(err)
	}
}
