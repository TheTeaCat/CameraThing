# CameraThing

This repository should contain all the information required to create a CameraThing, and set up the cloud services it requires.



## Local Development

The cloud services used by the CameraThing can be run locally like so:

```bash
git clone git@github.com:TheTeaCat/CameraThing.git
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