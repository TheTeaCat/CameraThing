package main

import (
	"log"
	"net/http"
	"os"
)

var myRecogniser *recogniser
var myTweeter *tweeter

func main() {
	log.Println("Tweeter service started.")

	//Create recogniser instance
	log.Println("Creating recogniser instance...")
	myRecogniser = newRecogniser()

	//Create tweeter instance
	log.Println("Creating tweeter instance...")
	myTweeter = newTweeter()

	//Create endpoints
	log.Println("Creating endpoint instances...")
	myTweetEndpoint := newTweetEndpoint()

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
