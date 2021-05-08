# Tweeter

The tweeter is a service which is intended to run alongside an instance of [go-tensorflow-image-recognition](https://github.com/tinrab/go-tensorflow-image-recognition/). Its purpose is to take images and geolocations from the CameraThing and create tweets from them on the CameraThing's twitter account, [@CameraThing](https://twitter.com/CameraThing).



## API

### /tweet

A `POST` request should be made to this endpoint with a multipart request body consisting of an image encoded as bytes with the key `image`, the auth token specified by the `TWEET_AUTH_TOKEN` environment variable as a `GET` parameter with the key `auth`, and a longitude and latitude as `GET` parameters with the keys `long` and `lat`.

For example, the following curl request:

```bash
$ curl "localhost:8080/tweet?auth=dev&lat=53.36097329965131&long=-1.6899902029658576" -F 'image=@./test.jpg'
{"Tweet":"#Breakwater #Promontory #Seashore #GreyWhale #Wing (53.36097,-1.68999)"}
```

The response contains the text used in the tweet created for that image. The request may take a while as the image is processed by [go-tensorflow-image-recognition](https://github.com/tinrab/go-tensorflow-image-recognition/) to create the tweet body before the client can be responded to.



## Environment Variables

### `ENV`

Should be set to either `DEV` or `PROD`. `DEV` allows for the rest of the environment variables to be left unset, default values will be used instead.



### `RECOGNISER_ENDPOINT`

Should be set to the address and endpoint where the tweeter can access the `/recognize` endpoint of an instance of [go-tensorflow-image-recognition](https://github.com/tinrab/go-tensorflow-image-recognition/).

If this is not set, and `ENV` is set to `DEV`, it defaults to `http://imagerec:8080/recognize`.



### `TWEET_AUTH_TOKEN`

Should be set to an alphanumeric randomly generated token which must be provided as a GET parameter to the tweeter's `/tweet` endpoint in order to authenticate - it is a shared secret between the CameraThing and the Tweeter service.

If this is not set, and `ENV` is set to `DEV`, it defaults to `dev`.



### `TWITTER_`

These environment variables all contain credentials to access the twitter API. They can be found in the Developer Portal under Projects and Apps -> Your Project -> Your App -> Keys and tokens:

![twitter-developer-portal](./docs/imgs/twitter-developer-portal.jpg)

#### `TWITTER_ACCESS_TOKEN`

Your Twitter Access Token.

#### `TWITTER_ACCESS_TOKEN_SECRET`

Your Twitter Access Secret.

#### `TWITTER_CONSUMER_KEY`

Your Twitter Consumer Key.

#### `TWITTER_CONSUMER_SECRET`

Your Twitter Consumer Secret.



#### 