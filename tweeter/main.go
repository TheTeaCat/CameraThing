package main

import (
	"log"
	"net/http"
	"os"
)

type Environment string

const (
	DEV  Environment = "DEV"
	PROD Environment = "PROD"
)

var env Environment
var myRecogniser *recogniser
var myTweeter *tweeter

func main() {
	var err error

	log.Println("Tweeter service started.")

	//Create recogniser instance
	log.Println("Creating recogniser instance...")
	myRecogniser, err = newRecogniser()
	if err != nil {
		log.Fatalf(
			"Couldn't create recogniser instance, err: %[1]v",
			err.Error(),
		)
	}

	//Create tweeter instance
	log.Println("Creating tweeter instance...")
	myTweeter, err = newTweeter()
	if err != nil {
		log.Fatalf(
			"Couldn't create tweeter instance, err: %[1]v",
			err.Error(),
		)
	}

	//Create endpoints
	log.Println("Creating endpoint instances...")
	myTweetEndpoint, err := newTweetEndpoint()
	if err != nil {
		log.Fatalf(
			"Couldn't create tweetEndpoint instance, err: %[1]v",
			err.Error(),
		)
	}

	//Attach endpoint handlers
	log.Println("Attaching endpoint handlers...")
	http.HandleFunc("/tweet", myTweetEndpoint.handle)

	//Determine port to use from env vars (default to 8080)
	port, portSet := os.LookupEnv("PORT")
	if !portSet {
		port = "8080"
	}
	log.Printf("Using port %[1]v...", port)

	//Now everything is set up, listen & serve...
	log.Printf("Listening and serving!")
	log.Fatal(http.ListenAndServe(":"+port, nil))
}
