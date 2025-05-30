package handlers

import (
	"argus/backend/models"
	"net/http"
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

var (
	seekers               = make(map[string]*models.Seeker)
	seekersLock           sync.RWMutex
	frontends             = make(map[*websocket.Conn]bool)
	frontendsLock         sync.RWMutex
	frontendsSpecific     = make(map[string]*models.Frontend)
	frontendsSpecificLock sync.RWMutex
)