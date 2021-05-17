# Firmware

This document details how to setup the CameraThing's firmware and perform development on it.



## Secrets.h

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



## Debugging

The image from the camera can be outputted over serial by using `frameBufferToSerial` within the `takePicture` method. This is useful to make sure everything's wired up correctly.