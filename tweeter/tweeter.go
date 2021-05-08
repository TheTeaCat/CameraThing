package main

import (
	"errors"
	"log"
	"os"

	"github.com/dghubble/go-twitter/twitter"
	"github.com/dghubble/oauth1"
)

//A tweeter struct manages our twitter client instance
type tweeter struct {
	client   *twitter.Client
	username string
}

//Constructor
func newTweeter() (*tweeter, error) {
	//Check twitter credentials are set in env vars
	accToken, accTokenSet := os.LookupEnv(
		"TWITTER_ACCESS_TOKEN",
	)
	accTokenSecret, accTokenSecretSet := os.LookupEnv(
		"TWITTER_ACCESS_TOKEN_SECRET",
	)
	conKey, conKeySet := os.LookupEnv(
		"TWITTER_CONSUMER_KEY",
	)
	conSecret, conSecretSet := os.LookupEnv(
		"TWITTER_CONSUMER_SECRET",
	)
	gotAllCreds := accTokenSet && accTokenSecretSet && conKeySet && conSecretSet

	//If creds are incomplete and we're not in dev env, we err
	if !gotAllCreds && env != DEV {
		return nil, errors.New("TWITTER_ environment variables incomplete")
	}

	//If the creds aren't set, & we are in dev, return a tweeter with nil client
	if !gotAllCreds && env == DEV {
		return &tweeter{
			client:   nil,
			username: "Dev",
		}, nil
	}

	//Otherwise if creds are set, we create a client
	token := oauth1.NewToken(accToken, accTokenSecret)
	config := oauth1.NewConfig(conKey, conSecret)
	httpClient := config.Client(oauth1.NoContext, token)
	client := twitter.NewClient(httpClient)

	//Verify creds
	verifyParams := &twitter.AccountVerifyParams{
		SkipStatus:   twitter.Bool(true),
		IncludeEmail: twitter.Bool(true),
	}
	user, _, err := client.Accounts.VerifyCredentials(verifyParams)
	//If creds fail, err
	if err != nil {
		return nil, errors.New("Failed to verify twitter credentials")
	}

	//Otherwise return tweeter with client & username filled in
	return &tweeter{
		client:   client,
		username: user.Name,
	}, nil
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
