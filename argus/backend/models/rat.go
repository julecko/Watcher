package models

import "github.com/gorilla/websocket"

// Rat represents a connected client with extensible fields
type Rat struct {
	ID       string                 `json:"id"`       // Unique identifier
	Name     string                 `json:"name"`     // Rat name
	OS       string                 `json:"os"`       // Operating system
	IP       string                 `json:"ip"`       // IP address
	Conn     *websocket.Conn        `json:"-"`        // WebSocket connection (not serialized)
	Metadata map[string]interface{} `json:"metadata"` // Extensible metadata
}

// Message represents WebSocket messages
type Message struct {
	Type string      `json:"type"`
	Data interface{} `json:"data"`
}
