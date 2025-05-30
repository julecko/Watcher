package handlers

import (
	"argus/backend/models"
	"argus/backend/utils"
	"encoding/json"
	"log"
	"net/http"
	"path/filepath"
)

func ServeFrontend(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, filepath.Join("backend", "static", "index.html"))
}

func GetSeekers(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

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

func RemoveSeeker(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var request struct {
		UUID string `json:"uuid"`
	}
	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		http.Error(w, "Invalid request body", http.StatusBadRequest)
		return
	}

	if request.UUID == "" {
		http.Error(w, "UUID is required", http.StatusBadRequest)
		return
	}

	seekersLock.RLock()
	seeker, exists := seekers[request.UUID]
	seekersLock.RUnlock()
	if !exists {
		http.Error(w, "Seeker not found", http.StatusNotFound)
		return
	}

	if seeker.Conn != nil {
		err := sendToSeeker(request.UUID, models.Message{
			Type: "Disconnect",
			Data: "Server-initiated disconnect",
		})
		if err != nil {
			log.Printf("[RemoveSeeker] Failed to send disconnect message to seeker %s: %v", request.UUID, err)
		}
	}

	utils.RemoveFromMap(seekers, request.UUID, &seekersLock)
	log.Printf("[RemoveSeeker] Seeker removed: %s", request.UUID)

	w.WriteHeader(http.StatusOK)
	w.Write([]byte(`{"message": "Seeker removed successfully"}`))
}
