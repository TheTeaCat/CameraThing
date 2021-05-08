package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"io"
	"log"
	"net/http"
	"os"
	"strings"
)

type tweetEndpoint struct {
	authToken string //The auth token that must be provided to use this endpoint
}

//Constructor
func newTweetEndpoint() (*tweetEndpoint, error) {
	//Load authToken from env var
	authToken, authTokenSet := os.LookupEnv("TWEET_AUTH_TOKEN")
	if !authTokenSet && env == DEV {
		//Default value if not set for dev env
		authToken = "dev"
	} else {
		return nil, errors.New("TWEET_AUTH_TOKEN not set")
	}

	//Return tweetEndpoint
	return &tweetEndpoint{
		authToken: authToken,
	}, nil
}

//Endpoint handler
func (te *tweetEndpoint) handle(w http.ResponseWriter, r *http.Request) {
	//@Check request for auth token
	authToken := r.FormValue("auth")
	if authToken != te.authToken {
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusUnauthorized)
		json.NewEncoder(w).Encode("Invalid auth token")
		return
	}

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
	for i := 0; i < 5 && i < len(labels); i++ {
		if i > 0 {
			imageTitle += " "
		}
		imageTitle += "#" + strings.ReplaceAll(strings.Title(labels[i].Label), " ", "")
	}
	//If no labels were returned, use a default name
	if len(labels) == 0 {
		imageTitle = "Untitled"
	}

	//@todo: make tweet
	log.Printf("Image named: %[1]v", imageTitle)

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(map[string]string{
		"Title": imageTitle,
	})
}
