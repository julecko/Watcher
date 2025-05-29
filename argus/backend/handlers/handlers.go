package handlers

import (
	"argus/backend/models"
	"argus/backend/utils"
	"context"
	"encoding/json"
	"fmt"
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

// Global variables
var (
	seekers               = make(map[string]*models.Seeker)
	seekersLock           sync.RWMutex
	frontends             = make(map[*websocket.Conn]bool)
	frontendsLock         sync.RWMutex
	frontendsSpecific     = make(map[string]*models.Frontend)
	frontendsSpecificLock sync.RWMutex
)

func ServeFrontend(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, filepath.Join("backend", "static", "index.html"))
}

func GetSeekers(w http.ResponseWriter, r *http.Request) {
	seekersLock.RLock()
	defer seekersLock.RUnlock()

	data, err := json.Marshal(seekers)
	if err != nil {
		http.Error(w, "Failed to serialize seekers", http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.Write(data)
}

func SeekerWebSocketHandler(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("Seeker WebSocket upgrade failed:", err)
		return
	}
	defer conn.Close()

	var initData models.Seeker
	if err := conn.ReadJSON(&initData); err != nil {
		log.Println("Failed to read initial seeker data:", err)
		return
	}

	initData.Conn = conn

	seekersLock.Lock()
	seekers[initData.ID] = &initData
	seekersLock.Unlock()

	if err := conn.WriteJSON(map[string]string{"id": initData.ID}); err != nil {
		log.Println("Failed to send ID to seeker:", err)
		utils.RemoveFromMap(seekers, initData.ID, &seekersLock)
		return
	}

	log.Printf("Seeker connected: %s (%s)", initData.Name, initData.ID)

	broadcastSeekerList()

	for {
		var msg models.Message
		if err := conn.ReadJSON(&msg); err != nil {
			log.Println("Seeker WebSocket read error:", err)
			utils.RemoveFromMap(seekers, initData.ID, &seekersLock)
			broadcastSeekerList()
			return
		}

		log.Printf("Received from seeker %s: %v", initData.ID, msg)

		broadcastToFrontends(msg)
	}
}

func FrontendWebSocketHandler(w http.ResponseWriter, r *http.Request) {
	path := strings.TrimPrefix(r.URL.Path, "/ws/frontend")
	path = strings.Trim(path, "/")

	if path == "" {
		frontendWebSocketHandlerNoId(w, r)
		return
	}

	r = r.WithContext(context.WithValue(r.Context(), "seekerId", path))
	frontendWebSocketHandlerWithId(w, r)
}

func frontendWebSocketHandlerNoId(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("Frontend WebSocket upgrade failed:", err)
		return
	}

	frontendsLock.Lock()
	frontends[conn] = true
	frontendsLock.Unlock()

	seekersLock.RLock()
	data, _ := json.Marshal(seekers)
	seekersLock.RUnlock()
	conn.WriteJSON(models.Message{
		Type: "Seekers",
		Data: string(data),
	})

	log.Println("Frontend connected")

	defer func() {
		utils.RemoveFromMap(frontends, conn, &frontendsLock)
		log.Println("Frontend disconnected")
	}()

	for {
		var msg models.Message
		if err := conn.ReadJSON(&msg); err != nil {
			log.Println("Frontend WebSocket read error:", err)
			return
		}

		log.Printf("Received from frontend: %v", msg)

		switch msg.Type {
		}
	}
}

func frontendWebSocketHandlerWithId(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("Frontend WebSocket upgrade failed:", err)
		return
	}

	seekerId, ok := r.Context().Value("seekerId").(string)
	if !ok || seekerId == "" {
		log.Println("Seeker ID missing or invalid")
		conn.Close()
		return
	}

	var frontendData models.Frontend
	frontendData.SeekerId = seekerId
	frontendData.Conn = conn

	frontendsSpecificLock.Lock()
	frontendsSpecific[seekerId] = &frontendData
	frontendsSpecificLock.Unlock()

	log.Printf("Frontend connected for seeker %s", seekerId)

	defer func() {
		utils.RemoveFromMap(frontendsSpecific, conn, &frontendsSpecificLock)
		log.Printf("Frontend for seeker %s disconnected", seekerId)
	}()

	for {
		var msg models.Message
		if err := conn.ReadJSON(&msg); err != nil {
			log.Println("Frontend WebSocket read error:", err)
			return
		}

		log.Printf("Received from frontend (%s): %v", seekerId, msg)

		sendToSeeker(seekerId, msg)
	}
}

func sendToSeeker(seekerID string, msg models.Message) error {
	seekersLock.RLock()
	seeker, exists := seekers[seekerID]
	seekersLock.RUnlock()

	if !exists {
		return fmt.Errorf("seeker %s not found", seekerID)
	}

	return seeker.Conn.WriteJSON(msg)
}

func broadcastToFrontends(msg models.Message) {
	frontendsLock.RLock()
	defer frontendsLock.RUnlock()

	for conn := range frontends {
		if err := conn.WriteJSON(msg); err != nil {
			log.Println("Error broadcasting to frontend:", err)
		}
	}
}

func broadcastSeekerList() {
	seekersLock.RLock()
	data, _ := json.Marshal(seekers)
	seekersLock.RUnlock()

	broadcastToFrontends(models.Message{
		Type: "Seekers",
		Data: string(data),
	})
}
