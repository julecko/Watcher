package handlers

import (
	"argus/backend/models"
	"argus/backend/utils"
	"context"
	"encoding/json"
	"log"
	"net/http"
	"strings"
)

type seekerIDKey string

func FrontendWebSocketHandler(w http.ResponseWriter, r *http.Request) {
	path := strings.TrimPrefix(r.URL.Path, "/ws/frontend")
	path = strings.Trim(path, "/")

	if path == "" {
		frontendWebSocketHandlerNoId(w, r)
		return
	}

	r = r.WithContext(context.WithValue(r.Context(), seekerIDKey("seekerId"), path))
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
	}
}

func frontendWebSocketHandlerWithId(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("Frontend WebSocket upgrade failed:", err)
		return
	}

	seekerId, ok := r.Context().Value(seekerIDKey("seekerId")).(string)
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
