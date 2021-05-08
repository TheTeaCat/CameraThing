package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"strconv"
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
	//Check request for auth token
	authToken := r.FormValue("auth")
	if authToken != te.authToken {
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusUnauthorized)
		json.NewEncoder(w).Encode("Invalid auth token")
		return
	}

	//Get GPS location from request
	latStr := r.FormValue("lat")
	lat, err := strconv.ParseFloat(latStr, 64)
	if latStr == "" || err != nil {
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusBadRequest)
		if latStr == "" {
			json.NewEncoder(w).Encode("No latitude supplied.")
		} else {
			json.NewEncoder(w).Encode("Latitude not float64")
		}
		return
	} else if lat < -180 || lat > 180 {
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode("Latitude out of range (min -180, max 180)")
		return
	}
	longStr := r.FormValue("long")
	long, err := strconv.ParseFloat(longStr, 64)
	if longStr == "" || err != nil {
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusBadRequest)
		if latStr == "" {
			json.NewEncoder(w).Encode("No longitude supplied.")
		} else {
			json.NewEncoder(w).Encode("Longitude not float64")
		}
		return
	} else if long < -90 || long > 90 {
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode("Longitude out of range (min -90, max 90)")
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

	//Get image data out into slice of bytes
	var imageBuffer bytes.Buffer
	io.Copy(&imageBuffer, imageFile)
	imageBytes := imageBuffer.Bytes()

	//Get image labels
	labels, err := myRecogniser.recognise(imageBytes)
	if err != nil {
		log.Printf("Failed to recognise image, err: %[1]v", err.Error())
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode("Failed to recognise image")
		return
	}

	/*Construct a tweetBody consisting of an image title from up to the first
	three labels*/
	tweetBody := ""
	for i := 0; i < 5 && i < len(labels); i++ {
		tweetBody += "#" + strings.ReplaceAll(strings.Title(labels[i].Label), " ", "") + " "
	}
	//If no labels were returned, use a default name
	if len(labels) == 0 {
		tweetBody = "Untitled"
	}
	//Add the location to the tweet
	tweetBody += fmt.Sprintf("(%.5f,%.5f)", lat, long)

	//Make tweet
	err = myTweeter.tweetWithImage(tweetBody, imageBytes)
	if err != nil {
		log.Printf("Failed to tweet image, err: %[1]v", err.Error())
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode("Failed to tweet image")
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(map[string]string{
		"Tweet": tweetBody,
	})
}
