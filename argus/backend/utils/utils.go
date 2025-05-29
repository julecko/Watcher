package utils

import (
	"argus/backend/models"
	"log"
	"strings"
	"sync"

	"github.com/gorilla/websocket"
)

func RemoveFromMap(m interface{}, key interface{}, lock *sync.RWMutex) {
	lock.Lock()
	defer lock.Unlock()

	switch typedMap := m.(type) {
	case map[string]*models.Seeker:
		if k, ok := key.(string); ok {
			delete(typedMap, k)
		}
	case map[*websocket.Conn]bool:
		if k, ok := key.(*websocket.Conn); ok {
			delete(typedMap, k)
			k.Close()
		}
	default:
		log.Println("Unsupported map type passed to removeFromMap")
	}
}

func GetSeekerID(path string) (string, bool) {
	if !strings.HasPrefix(path, "/ws/frontend/") {
		return "", false
	}
	parts := strings.Split(path, "/")
	if len(parts) != 4 {
		return "", false
	}
	return parts[3], true
}
