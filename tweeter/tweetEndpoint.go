package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"image/jpeg"
	"io"
	"log"
	"net/http"
	"os"
	"strconv"
	"strings"

	"github.com/nfnt/resize"
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
	} else if !authTokenSet && env == PROD {
		return nil, errors.New("TWEET_AUTH_TOKEN not set")
	}

	//Return tweetEndpoint
	return &tweetEndpoint{
		authToken: authToken,
	}, nil
}

//Endpoint handler
func (te *tweetEndpoint) handle(w http.ResponseWriter, r *http.Request) {
	//Log req occurred
	log.Println("      [/tweet] - Request @ /tweet!")

	//Check request for auth token
	authToken := r.FormValue("auth")
	if authToken != te.authToken {
		log.Println("[401] [/tweet] - No auth token supplied")
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusUnauthorized)
		json.NewEncoder(w).Encode("Invalid auth token")
		return
	}

	//////////////////////////////////////////////////////////////////////
	//Get GPS location from request
	//We only add GPS data to the tweet if both lat and long are provided
	longitudeProvided := true
	latitudeProvided := true

	latStr := r.FormValue("lat")
	lat, err := strconv.ParseFloat(latStr, 64)
	if latStr == "" || err != nil {
		latitudeProvided = false
	} else if lat < -180 || lat > 180 {
		log.Println("[400] [/tweet] - Latitude out of range")
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode("Latitude out of range (min -180, max 180)")
		return
	}

	longStr := r.FormValue("long")
	long, err := strconv.ParseFloat(longStr, 64)
	if longStr == "" || err != nil {
		longitudeProvided = false
	} else if long < -90 || long > 90 {
		log.Println("[400] [/tweet] - Longitude out of range")
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode("Longitude out of range (min -90, max 90)")
		return
	}

	if !longitudeProvided && !latitudeProvided {
		log.Println("      [/tweet] - No longitude or latitude provided, not using geolocation")
	} else if longitudeProvided && latitudeProvided {
		log.Println("      [/tweet] - Longitude and latitude provided, using geolocation")
	} else {
		if longitudeProvided {
			log.Println("[400] [/tweet] - Only longitude provided")
		} else {
			log.Println("[400] [/tweet] - Only latitude provided")
		}
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode("Longitude and latitude must be provided, or neither")
		return
	}

	//////////////////////////////////////////////////////////////////////
	//Read image from form
	imageFile, _, err := r.FormFile("image")
	if err != nil {
		log.Printf("[400] [/tweet] - Couldn't read image file, err: %[1]v", err.Error())
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode("Failed to read image file")
		return
	}
	defer imageFile.Close()

	//////////////////////////////////////////////////////////////////////
	//Get image data out into slice of bytes
	var imageBuffer bytes.Buffer
	io.Copy(&imageBuffer, imageFile)
	imageBytes := imageBuffer.Bytes()

	//////////////////////////////////////////////////////////////////////
	//Get image labels
	labels, err := myRecogniser.recognise(imageBytes)
	recogniserFailed := false //We still tweet if the recogniser fails.
	if err != nil {
		log.Printf("[XXX] [/tweet] - Image recognition failed, err: %[1]v", err.Error())
		recogniserFailed = true
	}

	//////////////////////////////////////////////////////////////////////
	//Construct a tweetBody of up to five labels followed by question marks
	tweetBody := ""
	if !recogniserFailed {
		for i := 0; i < 5 && i < len(labels); i++ {
			tweetBody += strings.Title(labels[i].Label) + "? "
		}
	}
	//If no labels were returned, or the tweeter otherwise failed, say we've got
	//no idea what it is
	if len(tweetBody) == 0 {
		tweetBody = "I have no idea. "
	}

	//Add the location to the tweet if we have both lat and long
	if longitudeProvided && latitudeProvided {
		tweetBody += fmt.Sprintf("(%.5f,%.5f)", lat, long)
	}

	//////////////////////////////////////////////////////////////////////
	//Decode image into jpeg
	img, err := jpeg.Decode(bytes.NewReader(imageBytes))
	if err != nil {
		log.Printf("[500] [/tweet] - Failed to decode image as jpeg, err: %[1]v", err.Error())
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode("Failed to decode image as jpeg")
		return
	}

	//////////////////////////////////////////////////////////////////////
	//Upscale image with no interpolation
	bigImg := resize.Resize(1280, 0, img, resize.NearestNeighbor)
	bigImgBytesBuf := new(bytes.Buffer)
	err = jpeg.Encode(bigImgBytesBuf, bigImg, nil)
	if err != nil {
		log.Printf("[500] [/tweet] - Failed to upscale image, err: %[1]v", err.Error())
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode("Failed to upscale image")
		return
	}

	//Make tweet
	tweetMade, err := myTweeter.tweetWithImage(tweetBody, bigImgBytesBuf.Bytes())
	if err != nil {
		log.Printf("[500] [/tweet] - Failed to tweet image, err: %[1]v", err.Error())
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode("Failed to tweet image")
		return
	}

	//Respond
	response := map[string]string{"Tweet": tweetBody}
	if tweetMade != nil {
		response["TweetURL"] = "https://twitter.com/" + tweetMade.User.ScreenName + "/status/" + tweetMade.IDStr

	}
	log.Printf("[201] [/tweet] - Successfully tweeted: %[1]v", strings.ReplaceAll(tweetBody, "\n", ""))
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(response)
}
