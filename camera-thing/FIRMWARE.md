# Firmware

This document details how to setup the CameraThing's firmware.



## Building

I used platform.io, an appropriate `platformio.ini` file is included in this repo. You should just be able to do:

```bash
cd camera-thing
pio run -t upload -t monitor
```

I had some permissions issues with `/dev/ttyUSB0` so I made a simple bash script, `go.sh` which fixes that and runs `pio run -t upload -t monitor` for me.

You can have a sample of healthy serial output in [SERIAL_TRANSCRIPT.md](./SERIAL_TRANSCRIPT.md).



## Preprocessor Instructions

All the `#defines` are detailed here.



### Hardware Config

There are a number of identifiers that need to be defined to describe the pinout used by your CameraThing. Configuring these incorrectly can lead to the LED signalling hardware or network failures, detailed in [README.md](./README.md).



#### `camera.cpp`

The GPIO pins used by the camera module are defined in `camera.cpp`.

| Identifier    | Value                                                        |
| ------------- | ------------------------------------------------------------ |
| CAM_PIN_PWDN  | The PWDN pin of the OV7670. This is optional and can be set to -1. |
| CAM_PIN_RESET | The RESET pin of the OV7670. This is optional and can be set to -1. |
| CAM_PIN_XCLK  | The XCLK pin of the OV7670.                                  |
| CAM_PIN_SIOD  | The SIOD pin of the OV7670.                                  |
| CAM_PIN_SIOC  | The SIOC pin of the OV7670.                                  |
| CAM_PIN_D7    | The D7 pin of the OV7670.                                    |
| CAM_PIN_D6    | The D6 pin of the OV7670.                                    |
| CAM_PIN_D5    | The D5 pin of the OV7670.                                    |
| CAM_PIN_D4    | The D4 pin of the OV7670.                                    |
| CAM_PIN_D3    | The D3 pin of the OV7670.                                    |
| CAM_PIN_D2    | The D2 pin of the OV7670.                                    |
| CAM_PIN_D1    | The D1 pin of the OV7670.                                    |
| CAM_PIN_D0    | The D0 pin of the OV7670.                                    |
| CAM_PIN_VSYNC | The VSYNC pin of the OV7670.                                 |
| CAM_PIN_HREF  | The HREF pin of the OV7670.                                  |
| CAM_PIN_PCLK  | The PCLK pin of the OV7670.                                  |



#### `gprsClient.cpp` and `gprsClient.h`

The GPIO pins used for the SIM800L module are defined in `gprsClient.cpp` as follows:

| Identifier     | Value                         |
| -------------- | ----------------------------- |
| SIM800L_RX     | The RX pin of the SIM800L     |
| SIM800L_TX     | The TX pin of the SIM800L     |
| SIM800L_PWRKEY | The PWRKEY pin of the SIM800L |
| SIM800L_RST    | The RST pin of the SIM800L    |
| SIM800L_POWER  | The POWER pin of the SIM800L  |

The config for [TinyGsm](https://github.com/vshymanskyy/TinyGSM) are defined in `gprsClient.h` as follows:

| Identifier            | Value                                                        |
| --------------------- | ------------------------------------------------------------ |
| TINY_GSM_MODEM_SIM800 | None; this is a flag to state which GPRS chip we're using. In this case, it's a SIM800L. |
| TINY_GSM_RX_BUFFER    | How big the buffer for receiving bytes from the SIM800L module should be |
| SerialAT              | The Serial connection that should be used for the TinyGsm modem |



#### `geolocate.cpp`

The Serial output used by the GPS module is defined in `geolocate.cpp` with the identifier `GPSSerial`. You may want to use a `HardwareSerial` to use custom GPIO pins.



### Secrets.h

To build the firmware, you need to create a file `main/secrets.h` containing a number of definitions described below. You can find a template for this file as `main/secrets.example.h`.

```c++
///////////////////////////////////////////////////////////////////////////
// Network credentials
// Note: Either WiFi OR 2G must be used; not both.

//Credentials for using the webClient in wifiClient.h
//If WIFI_SSID is defined then the CameraThing will attempt to use WiFi.
#define WIFI_SSID "Put your SSID in here! :)"
#define WIFI_PASS "Put your password in here! :)"

//GPRS credentials for the webClient in gprsClient.h
//If APN is defined then the CameraThing will attempt to use 2G
#define APN "Put the APN of your provider here :)"
#define GPRS_USER "Put the username for your APN here :)"
#define GPRS_PASS "Put the password for your APN here :)"

///////////////////////////////////////////////////////////////////////////
// Cloud services

//Details for your tweeter service
#define TWEETER_HOST "Put the hostname of your tweeter service here! :)"
#define TWEETER_PORT Put the port of your tweeter here! :)
#define TWEETER_AUTH_TOKEN "Put the auth token for your tweeter here! :)"
```

You can define `WIFI_SSID` and `WIFI_PASS` to make the CameraThing use a WiFi connection (for example, a mobile hotspot), **OR** you can define `APN`, `GPRS_USER` and `GPRS_PASS` to use a 2G connection, **but not both!**

If your device doesn't have a SIM800L, you can happily just use a WiFi connection.



### DEBUG_IMG_TO_SERIAL

In `camera.cpp` you can define an identifier `DEBUG_IMG_TO_SERIAL` which will cause the CameraThing to output the framebuffer it gets from the camera to serial every time a photograph is taken - this is useful for initially making sure your wiring is setup correctly, although it is in greyscale so any colour issues will not show up.



### FAST_STARTUP

In `main.cpp` you can define an identifier `FAST_STARTUP` which will disable the CameraThing querying the tweeter service's `/tweet` endpoint on startup to check it has a network connection before allowing any photos to be taken - some people may prefer to know the camera will work for the first photo, others may prefer a shorter startup time with the risk that they will learn their tweeter service is unavailable by the failure of their first photo to upload.



### RESPONSE_TO_SERIAL

In `tweeter.cpp` you can define an identifier `RESPONSE_TO_SERIAL` which will disable the CameraThing outputting the response from the tweeter service's `/tweet` endpoint to serial. Currently, the CameraThing doesn't actually use the tweeter's response so if `RESPONSE_TO_SERIAL` is defined then the CameraThing doesn't wait for the tweeter service to respond at all before allowing the user to take another photo.



## Footnotes



### `geolocate.cpp` and `geolocate.h`

These files are not used in the current CameraThing. When I was developing on the Feather ESP32, I was using a GPS Featherwing to provide a latitude and longitude to the tweeter service so the tweets could have geolocations in them. This functionality still exists in the tweeter service so it can be added back later, and the appropriate code for implementing this is commented out in `main.cpp`. 

When I was developing on the Feather ESP32 I had three GPIO pins left between the camera (OV7670), GPS featherwing, The Comically Large Pink LED and button. When I moved to the TTGO T-Call v1.4, I discovered that its SIM800L module uses five GPIO pins. Consequently, I had to remove the GPS Featherwing to make room.

If you wanted to make a CameraThing with a Feather ESP32 that uses WiFi instead, then you could bring this functionality back by uncommenting the areas marked out in `main.cpp` and configuring the hardware as described in the "Hardware Config" section of this document.