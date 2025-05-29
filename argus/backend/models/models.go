package models

import (
	"time"

	"github.com/gorilla/websocket"
)

// Seeker represents a connected client with extensible fields
type Seeker struct {
	ID           string                 `json:"id"`   // Unique identifier
	Name         string                 `json:"name"` // Rat name
	OS           string                 `json:"os"`   // Operating system
	IP           string                 `json:"ip"`   // IP address
	Conn         *websocket.Conn        `json:"-"`    // WebSocket connection (not serialized)
	LastActive   time.Time              `json:"last_active"`
	Disconnected bool                   `json:"disconnected"`
	Metadata     map[string]interface{} `json:"metadata"` // Extensible metadata
}

type Frontend struct {
	SeekerId string
	Conn     *websocket.Conn
}

type Message struct {
	Type string      `json:"type"`
	Data interface{} `json:"data"`
}
