package main

import (
	"errors"
	"fmt"
	"log"
	"os"
	"strings"

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
	config := oauth1.NewConfig(conKey, conSecret)
	token := oauth1.NewToken(accToken, accTokenSecret)
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

/*tweetWithImage makes a tweet with a single image and a tweet body. It fails if
it cannot upload the image, or create the tweet. If the client of the recieving
tweeter instance is nil, then it simply outputs the tweet that would be made to
the logs as follows:

[DEV] [TWITTER] - Tweeted: Test Tweet
*/
func (t *tweeter) tweetWithImage(tweetBody string, image []byte) error {
	//If client is nil, we just log it
	if t.client == nil {
		log.Printf(
			"[DEV] [TWITTER] - Tweeted: %[1]v",
			strings.ReplaceAll(tweetBody, "\n", "\\n"),
		)
		return nil
	}

	//Upload our picture
	media, resp, err := t.client.Media.Upload(image, "tweet_image")
	if err != nil {
		log.Println(len(image))
		if resp != nil {
			log.Println(resp)
		}
		return errors.New(fmt.Sprintf(
			"Failed to upload image, err: %[1]v",
			err.Error(),
		))
	}

	//@todo: Find place ID of nearby places to add as a placeID in the tweet.

	//Make the tweet using the media ID
	_, _, err = t.client.Statuses.Update(
		tweetBody,
		&twitter.StatusUpdateParams{
			MediaIds: []int64{media.MediaID},
		},
	)
	if err != nil {
		return errors.New(fmt.Sprintf(
			"Failed to tweet image, err: %[1]v",
			err.Error(),
		))
	}

	return nil
}
