# CameraThing

Behold!

![A photograph of the CameraThing; a small white cardboard box with a small camera lens poking out of a hole that has been crudely cut out of it, with an LED above and to the right and a button from a mechanical keyboard handing out above and to the left. The words "Camera Thing" have been written on the front of the box in sharpie.](./docs/imgs/CameraThing.jpg)



Details on how to construct a CameraThing can be found in `/camera-thing`.

Details on the cloud services on which the CameraThing depends can be found below, with more details in each service's subdirectory.



## Cloud Services

The CameraThing is dependent upon a set of cloud services:

1. An instance of the "tweeter" service, in found in `/tweeter` of this repo.
2. An instance of `go-tensorflow-image-recognition`, which has been added as a git submodule to this repo and was **not written by me**.



### Local Development

The cloud services used by the CameraThing can be run locally with docker-compose like so:

```bash
git clone --recursive git@github.com:TheTeaCat/CameraThing.git
cd CameraThing
docker-compose up
```

Easy peasy!

The CameraThing simply queries the `/tweet` endpoint of the Tweeter service, which you can mock locally with the following curl command:

```bash
$ curl "localhost:8080/tweet?auth=dev&lat=53.36097329965131&long=-1.6899902029658576" -F 'image=@./test.jpg'
{"Tweet":"#Breakwater #Promontory #Seashore #GreyWhale #Wing (53.36097,-1.68999)"}
```

You do not need to have twitter credentials to test the tweeter service. For more details, see the docs in `/tweeter`.



### Deployment

Details on how to deploy the cloud services.



#### Heroku

The cloud services are dockerised and could easily be deployed to Heroku. Relevant docs for doing this can be found here: https://devcenter.heroku.com/categories/deploying-with-docker



#### Digitalocean/VPS

You can deploy to a digitalocean droplet or other VPS by SSHing into it, [installing docker](https://docs.docker.com/get-docker/), then doing:

```bash
git clone --recursive git@github.com:TheTeaCat/CameraThing.git
cd CameraThing
docker-compose up -d
```

You can then update by doing:

```bash
cd CameraThing
git pull
docker-compose down
docker-compose up -d
```

To view the logs:

```bash
docker-compose logs -f -t
```

An example of healthy log output:

```bash
Attaching to camerathing_tweeter_1, camerathing_imagerec_1
imagerec_1  | 2021-05-17T11:24:37.145636320Z Listening on port 8080...
tweeter_1   | 2021-05-17T11:24:38.711815035Z go: downloading github.com/drswork/go-twitter v0.0.0-20190721142740-110a39637298
tweeter_1   | 2021-05-17T11:24:38.759193581Z go: downloading github.com/dghubble/oauth1 v0.7.0
tweeter_1   | 2021-05-17T11:24:38.822071433Z go: downloading github.com/nfnt/resize v0.0.0-20180221191011-83c6a9932646
tweeter_1   | 2021-05-17T11:24:39.038935342Z go: downloading github.com/cenkalti/backoff v2.1.1+incompatible
tweeter_1   | 2021-05-17T11:24:39.070894538Z go: downloading github.com/dghubble/sling v1.3.0
tweeter_1   | 2021-05-17T11:24:39.366430315Z go: downloading github.com/google/go-querystring v1.0.0
tweeter_1   | 2021-05-17T11:24:42.286375568Z 2021/05/17 11:24:42 Tweeter service started.
tweeter_1   | 2021-05-17T11:24:42.286493822Z 2021/05/17 11:24:42 Creating recogniser instance...
tweeter_1   | 2021-05-17T11:24:42.286568766Z 2021/05/17 11:24:42 Creating tweeter instance...
tweeter_1   | 2021-05-17T11:24:42.514809485Z 2021/05/17 11:24:42 Logged into twitter as: Camera Thing
tweeter_1   | 2021-05-17T11:24:42.514976972Z 2021/05/17 11:24:42 Creating endpoint instances...
tweeter_1   | 2021-05-17T11:24:42.515112458Z 2021/05/17 11:24:42 Attaching endpoint handlers...
tweeter_1   | 2021-05-17T11:24:42.515290175Z 2021/05/17 11:24:42 Using port 8080...
tweeter_1   | 2021-05-17T11:24:42.515419248Z 2021/05/17 11:24:42 Listening and serving!
tweeter_1   | 2021-05-17T11:45:43.037054362Z 2021/05/17 11:45:43       [/tweet] - Request @ /tweet!
tweeter_1   | 2021-05-17T11:45:43.501037730Z 2021/05/17 11:45:43       [/tweet] - Longitude and latitude provided, using geolocation
tweeter_1   | 2021-05-17T11:45:45.266833446Z 2021/05/17 11:45:45       [TWEETER] - Successfully tweeted: Mongoose? Wombat? Wallaby? Weasel? Chesapeake Bay Retriever? (53.36097,-1.68999)
tweeter_1   | 2021-05-17T11:45:45.266990972Z 2021/05/17 11:45:45 [201] [/tweet] - Successfully tweeted: Mongoose? Wombat? Wallaby? Weasel? Chesapeake Bay Retriever? (53.36097,-1.68999)
tweeter_1   | 2021-05-17T11:46:40.255168229Z 2021/05/17 11:46:40       [/tweet] - Request @ /tweet!
tweeter_1   | 2021-05-17T11:46:40.458978395Z 2021/05/17 11:46:40       [/tweet] - No longitude or latitude provided, not using geolocation
tweeter_1   | 2021-05-17T11:46:41.901819054Z 2021/05/17 11:46:41       [TWEETER] - Successfully tweeted: Mongoose? Wombat? Wallaby? Weasel? Chesapeake Bay Retriever? 
tweeter_1   | 2021-05-17T11:46:41.901948053Z 2021/05/17 11:46:41 [201] [/tweet] - Successfully tweeted: Mongoose? Wombat? Wallaby? Weasel? Chesapeake Bay Retriever? 
```

