package main

import (
	"bytes"
	"encoding/json"
	"io"
	"log"
	"net/http"
)

type tweetEndpoint struct {
	authToken string //The auth token that must be provided to use this endpoint
}

//Constructor
func newTweetEndpoint() *tweetEndpoint {
	//@todo: get authToken from env var
	return &tweetEndpoint{
		authToken: "DEV",
	}
}

func (te *tweetEndpoint) handle(w http.ResponseWriter, r *http.Request) {
	//@todo: check request for auth token

	//Read image from form
	imageFile, _, err := r.FormFile("image")
	if err != nil {
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode("Failed to read image file")
		return
	}
	defer imageFile.Close()

	//Copy image data into buffer
	var imageBuffer bytes.Buffer
	io.Copy(&imageBuffer, imageFile)

	//Get image labels
	labels, err := myRecogniser.recognise(imageBuffer.Bytes())
	if err != nil {
		log.Printf("Failed to recognise image, err: %[1]v", err.Error())
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode("Failed to recognise image")
		return
	}

	//Construct a title for the image from up to the first three labels
	imageTitle := ""
	for i := 0; i < 3 && i < len(labels); i++ {
		if i > 0 {
			imageTitle += " "
		}
		imageTitle += labels[i].Label
	}
	//If no labels were returned, use a default name
	if len(labels) == 0 {
		imageTitle = "Untitled"
	}

	log.Printf("Image named: %[1]v", imageTitle)

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(map[string]string{
		"Title": imageTitle,
	})
}
