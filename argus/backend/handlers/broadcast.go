package handlers

import (
	"argus/backend/models"
	"encoding/json"
	"fmt"
	"log"
)

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

func notifySeekerConnected(id string) {
	frontendsSpecificLock.RLock()
	frontend, exists := frontendsSpecific[id]
	frontendsSpecificLock.RUnlock()

	if exists {
		msg := models.Message{
			Type: "SeekerConnected",
			Data: id,
		}
		if err := frontend.Conn.WriteJSON(msg); err != nil {
			log.Printf("Failed to notify frontend of seeker %s connection: %v", id, err)
		}
	}
}

func notifySeekerDisconnected(id string) {
	frontendsSpecificLock.RLock()
	frontend, exists := frontendsSpecific[id]
	frontendsSpecificLock.RUnlock()

	if exists {
		msg := models.Message{
			Type: "SeekerDisconnected",
			Data: id,
		}
		if err := frontend.Conn.WriteJSON(msg); err != nil {
			log.Printf("Failed to notify frontend of seeker %s disconnection: %v", id, err)
		}
	}
}
