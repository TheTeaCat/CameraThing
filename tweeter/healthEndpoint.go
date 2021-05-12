package main

import (
	"encoding/json"
	"log"
	"net/http"
)

type healthEndpoint struct {
}

//Constructor
func newHealthEndpoint() (*healthEndpoint, error) {
	//Return healthEndpoint
	return &healthEndpoint{}, nil
}

//Endpoint handler
func (te *healthEndpoint) handle(w http.ResponseWriter, r *http.Request) {
	//Log req occurred
	log.Println("Request @ /health!")

	//Respond
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode("I'm healthy!")
}
