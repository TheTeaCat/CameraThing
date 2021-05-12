# CameraThing

This repository should contain all the information required to create a CameraThing, and set up the cloud services it requires.



## Local Development

The cloud services used by the CameraThing can be run locally like so:

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



## Deployment

The cloud services could easily be deployed to Heroku. Relevant docs for doing this can be found here: https://devcenter.heroku.com/categories/deploying-with-docker

I'm deploying to a digitalocean droplet by SSHing into it, then doing:

```bash
git clone --recursive git@github.com:TheTeaCat/CameraThing.git
cd CameraThing
docker-compose up -d
```

And updating it by doing:

```bash
cd CameraThing
git pull
docker-compose down
docker-compose up -d
```

And then I can view the logs by doing:

```bash
docker-compose logs -f -t
```

Here's an example of the logs:

```bash
imagerec_1  | 2021-05-12T14:33:30.786469533Z Listening on port 8080...
tweeter_1   | 2021-05-12T14:33:31.556102006Z go: downloading github.com/drswork/go-twitter v0.0.0-20190721142740-110a39637298
tweeter_1   | 2021-05-12T14:33:31.598762190Z go: downloading github.com/dghubble/oauth1 v0.7.0
tweeter_1   | 2021-05-12T14:33:31.751421772Z go: downloading github.com/cenkalti/backoff v2.1.1+incompatible
tweeter_1   | 2021-05-12T14:33:31.762678126Z go: downloading github.com/dghubble/sling v1.3.0
tweeter_1   | 2021-05-12T14:33:31.965136990Z go: downloading github.com/google/go-querystring v1.0.0
tweeter_1   | 2021-05-12T14:33:34.164685551Z 2021/05/12 14:33:34 Tweeter service started.
tweeter_1   | 2021-05-12T14:33:34.164822029Z 2021/05/12 14:33:34 Creating recogniser instance...
tweeter_1   | 2021-05-12T14:33:34.164928096Z 2021/05/12 14:33:34 Creating tweeter instance...
tweeter_1   | 2021-05-12T14:33:34.364083614Z 2021/05/12 14:33:34 Logged into twitter as: Camera Thing
tweeter_1   | 2021-05-12T14:33:34.364219450Z 2021/05/12 14:33:34 Creating endpoint instances...
tweeter_1   | 2021-05-12T14:33:34.364304811Z 2021/05/12 14:33:34 Request @ /tweet!
tweeter_1   | 2021-05-12T14:33:34.364377865Z 2021/05/12 14:33:34 Attaching endpoint handlers...
tweeter_1   | 2021-05-12T14:33:34.364466469Z 2021/05/12 14:33:34 Using port 8080...
tweeter_1   | 2021-05-12T14:33:34.364536259Z 2021/05/12 14:33:34 Listening and serving!
tweeter_1   | 2021-05-12T14:37:21.992260180Z 2021/05/12 14:37:21 Request @ /health!
```

