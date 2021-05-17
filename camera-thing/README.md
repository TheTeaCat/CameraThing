# CameraThing

The IoT device!

This document provides an overview of what the device does, and how to use it.

For how to build the device, you'll want to see [HARDWARE.md](./HARDWARE.md); and to write the firmware to it, you'll want to see [FIRMWARE.md](./FIRMWARE.md).

You can see a serial transcript in [SERIAL_TRANSCRIPT.md](.SERIAL_TRANSCRIPT.md).



## What does it do?

The CameraThing takes a picture from the OV7670 module, JPEG compresses it, (optionally) gets a geolocation from a GPS featherwing if this has been set up (see the footnotes of [FIRMWARE.md](./FIRMWARE.md)), then uploads the image and geolocation to the tweeter service either over a 2G mobile internet connection or a WiFi connection (whichever has been setup when the firmware was flashed to the device).



## User Interface

The UI for the CameraThing is simple. It consists of: a button, *and* an LED!



### Startup

During startup, the LED breathes slowly.

Startup may fail for four reasons:

1. Setting up the GPS module fails (probably bad wiring). This will trigger a [hardware failure animation](#hardware-failure-animation) before restarting the CameraThing.
2. Setting up the camera fails (also probably bad wiring, or unsupported camera config - too high resolution/unsupported colour type). This will trigger a [hardware failure animation](#hardware-failure-animation) before restarting the CameraThing.
3. Connecting to WiFi fails (this will take a long time as up to 5 attempts are made, each lasting 1 minute) or connecting to 2G fails - whichever is setup. This should only happen if the network credentials are invalid or the network specified couldn't be found. This will trigger a [network failure animation](#network-failure-animation) before restarting the CameraThing.
4. The CameraThing couldn't contact the tweeter service's `/health` endpoint, or the tweeter service's `/health` endpoint returned a response code other than `200 OK`. This could happen if the tweeter service is down, or having difficulty at the moment. This will trigger a [network failure animation](#network-failure-animation) before restarting the CameraThing.

If everything goes well, the LED should just happily breathe for a few seconds while all the above runs through. Once it's finished, the LED blinks for 50 milliseconds every 3 seconds to indicate that it is on.



### Taking a picture

Typical user flow for taking an image is as follows:

1. Press the button to take a picture.

Incredible



The state of the device is then communicated through a single LED, using a number of animations defined in `main/asyncLed.cpp`.

1. The camera is getting and processing an image from the camera. During this period, the LED is simply **on**. If there is a failure at this point (camera or jpeg encoding failed), the LED does a [hardware failure animation](#hardware-failure-animation).
2. The camera is awaiting user preference for whether a geolocation should be supplied - the device waits for 2.4 seconds while performing a **fast breathe animation**. If the button is not down at the end of this period, then geolocation data is not used - handy if you're taking a picture at home and don't want to put your kitchen's GPS coordinates on the internet.
3. If the user chose to add geolocation data to the tweet, the camera is gets a longitude and latitude from the GPS module. This is communicated by a **"throb" animation with fast attack and slow decay** - to me, this signifies "pulling" or "down" which feels apt as we're pulling information from GPS satellites. If we fail to get a geolocation, the LED does a **[hardware failure animation](#hardware-failure-animation)**.
4. The camera is making a request to the tweeter service's `/tweet` endpoint. This is communicated by a **"throb" animation with slow attack and fast decay**, which signifies "pushing" or "up" to me, which makes sense as we're uploading the image to a cloud service. If this fails for any reason (including if the tweeter service returns a response code other than `201 Created`) then the LED does a [network failure animation](#network-failure-animation) and then restarts the device.



### Hardware failure animation

See the [startup](#startup) and [taking a picture](#taking-a-picture) sections for details on what causes these, and the [troubleshooting](#troubleshooting) section for what you can do about them.

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

See the [startup](#startup) and [taking a picture](#taking-a-picture) sections for details on what causes these, and the [troubleshooting](#troubleshooting) section for what you can do about them.

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



## Troubleshooting

What can I do if it's not working?

Step 1: Turn it off and on again.



### Hardware failure

These are indicated by a [hardware failure animation](#hardware-failure-animation).

In the case of a hardware failure, it's probably best to just find a computer and have a look at what the device is saying in the serial output. If you're out and about with device, and this isn't an option, then it could be worth checking the wiring is OK; something could have just slipped from the GPS or camera module.



### Network failure

These are indicated by a [network failure animation](#network-failure-animation).

If the device is saying there's a network failure, then if you're using a WiFi connection the first thing to check is that your WiFi network is working - if you're using a mobile hotspot from your phone, has your phone just ran out of battery or turned its hotspot off?

The next possibility is that your tweeter service has died or moved, which you can check by going to its `/health` endpoint in your browser to check it still says `"I'm Healthy!"`.

If neither of these things work, you'll probably have to find a laptop to plug it into and check the serial output.