# CameraThing

The IoT device!


## Setup

Wifi credentials must be set in `secrets.h`. An example can be found in `secrets.example.h`.

The details of your tweeter service also need to be set in `secrets.h`.



## User Interface

The UI for the CameraThing is simple. It consists of: a button, *and* an LED!

Typical user flow for taking an image is as follows:

1. Press the button to take a picture.

Incredible

The state of the device is then communicated through a single LED, using a number of animations defined in `main/asyncLed.cpp`.

1. The camera is getting and processing an image from the camera. During this period, the LED is simply **on**. If there is a failure at this point (camera or jpeg encoding failed), the LED does a **hardware failure animation**.
2. The camera is awaiting user preference for whether a geolocation should be supplied - the device waits for 2 seconds while performing a **fast breathe animation**. If the button is not down at the end of this period, then geolocation data is not used - handy if you're taking a picture at home and don't want to put your kitchen's GPS coordinates on the internet.
3. If the user chose to add geolocation data to the tweet, the camera is gets a longitude and latitude from the GPS module. This is communicated by a **"throb" animation with fast attack and slow decay** - to me, this signifies "pulling" or "down" which feels apt as we're pulling information from GPS sattelites. If we fail to get a geolocation, the LED does a **hardware failure animation**.
4. The camera is making a request to the tweeter service's `/tweet` endpoint. This is communicated by a **"throb" animation with slow attack and fast decay**, which signifies "pushing" or "up" to me, which makes sense as we're uploading the image to a cloud service. If this fails for any reason (including if the tweeter service returns a response code other than `201 Created`) then the LED does a **network failure animation** and then restarts the device.



### Hardware failure animation

The animation used for a hardware failure during the image-taking flow is `blink(100)` for 2 seconds.

```
  255_^   __    __    __    __    __    __    __    __    __    __ 
      |  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|
Duty  |  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|
Cycle |  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|
      |  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|  |##|
    0_|__|##|__|##|__|##|__|##|__|##|__|##|__|##|__|##|__|##|__|##|__>
       200ms^                 1000ms^                       2000ms^
                                   Time
Each on or off period lasts 100ms.
```



### Network failure animation

The animation used for a network failure during the image-taking flow is `step(1000,4)` for 3 seconds.

```
    255_^                ____                ____                ____ 
Duty    |           ____|####|          ____|####|          ____|####|
Cycle   |      ____|#########|     ____|#########|     ____|#########|
      0_|_____|##############|____|##############|____|##############|__>
                       1000ms^             2000ms^             3000ms^
                                   Time
Each step lasts 250ms.
```



## Debugging

The image from the camera can be outputted over serial by using `frameBufferToSerial` within the `takePicture` method. This is useful to make sure everything's wired up correctly.
