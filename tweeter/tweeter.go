package main

import (
	"log"

	"github.com/dghubble/go-twitter/twitter"
)

//A tweeter struct manages our twitter client instance
type tweeter struct {
	client *twitter.Client
}

//Constructor
func newTweeter() *tweeter {
	//@todo create twitter client
	return &tweeter{
		client: nil,
	}
}

/*tweet makes a tweet if credentials are set, otherwise outputs to logs.*/
func (t *tweeter) tweet(tweet string) error {
	//If client is nil, we just log it
	if t.client == nil {
		log.Printf("[DEV] [TWITTER] - Tweeted: %[1]v", tweet)
		return nil
	}

	//@todo use twitter client

	return nil
}
