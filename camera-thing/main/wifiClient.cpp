// wifiManager.cpp
// Config and utils for connecting to a wifi connection to test sending data to
// the tweeter service

#include <WiFi.h>
#include "utils.h"
#include "secrets.h"
#include "esp_camera.h"

WiFiClient wifiClient; // the TLS web client

String ip2str(IPAddress address) { // utility for printing IP addresses
  return
    String(address[0]) + "." + String(address[1]) + "." +
    String(address[2]) + "." + String(address[3]);
}

void setupWifiManager() {
  //Return immediately if the apropriate config vars aren't set
  #ifndef WIFISSID
    Serial.println("[setupWifi] - Couldn't connect to WiFi; WIFISSID not set.");
    return;
  #endif
  #ifndef WIFIPASSWORD
    Serial.println("[setupWifi] - Couldn't connect to WiFi; WIFIPASSWORD not set.");
    return;
  #endif

  #if defined(WIFISSID) && defined(WIFIPASSWORD)
    //We try 5 times to connect to WiFi before giving up
    for(int attempt = 0; attempt < 10; attempt++) {
      Serial.printf("[setupWifi] - Trying to connect to '%s'\n", WIFISSID);
      
      //Attempt to begin wifi conn
      WiFi.begin(WIFISSID, WIFIPASSWORD);

      //We allow each attempt to run for 60 seconds
      bool success = false;
      Serial.print("[setupWifi] - Connecting...");
      for(int trial = 0; trial < 60; trial++) {
        if (WiFi.status() != WL_CONNECTED) {
          Serial.print(".");
          WAIT_MS(1000);
        } else {
          Serial.print(" connected!\n");
          success = true;
          break;
        }
        if(trial == 59) {
          Serial.print(" failed! :(\n");
        }
      }

      if(success) {
        Serial.printf("[setupWifi] - WiFi successfully connected to SSID: '%s'\n", WIFISSID);
        Serial.printf("[setupWifi] - Device IP: %s\n", ip2str(WiFi.localIP()));
        return;
      }
    }

    Serial.printf("[setupWifi] - WiFi failed to connect to SSID: '%s'\n", WIFISSID);
  #endif
}

void checkTweeterAccessible(int timeout) {
  //Connect to tweeter
  Serial.printf("[checkTweeterAccessible] - Connecting to %s:%d...\n", TWEETER_HOST, TWEETER_PORT);
  if (!wifiClient.connect(TWEETER_HOST, TWEETER_PORT)) {
    Serial.println("[checkTweeterAccessible] - Failed to connect :(");
    return;
  }

  //Construct request
  char *req = "GET /health HTTP/1.1\r\n"
              "Host: " TWEETER_HOST "\r\n\r\n";
  Serial.println("[checkTweeterAccessible] -------------------------Request Start");
  Serial.print(req);
  Serial.println("[checkTweeterAccessible] -------------------------Request End");

  //Make request to the tweeter's /health endpoint
  Serial.println("[checkTweeterAccessible] - Making request...");
  wifiClient.print(req);

  //Handle timeout
  Serial.printf("[checkTweeterAccessible] - Awaiting response (read timeout %d ms)...", timeout);
  int startTime = millis();
  while(wifiClient.available() == 0) {
    if(millis() - startTime > timeout) {
      Serial.println(" timed out :(");
      wifiClient.stop();
      return;
    }
    WAIT_MS(1000);
    Serial.print(".");
  }
  Serial.println(" success!");

  //Output response
  Serial.println("[checkTweeterAccessible] -------------------------Response Start");
  while(wifiClient.available()) {
    Serial.println(wifiClient.readStringUntil('\r'));
  }
  Serial.println("[checkTweeterAccessible] -------------------------Response End");

  //Close wifi client
  wifiClient.stop();
}

void makeTweetRequest(int timeout, float lat, float lon, uint8_t **jpgBuffer, size_t *jpgLen) {
  //////////////////////////////////////////////////////////////////////
  //Connect to tweeter
  Serial.printf("[makeTweetRequest] - Connecting to %s:%d...\n", TWEETER_HOST, TWEETER_PORT);
  if (!wifiClient.connect(TWEETER_HOST, TWEETER_PORT)) {
    Serial.println("[makeTweetRequest] - Failed to connect :(");
    return;
  }

  //////////////////////////////////////////////////////////////////////
  //Write request body
  Serial.println("[makeTweetRequest] - Writing request...");

  wifiClient.print("POST /tweet?auth=" TWEETER_AUTH_TOKEN "&lat=");
  wifiClient.printf("%02.5f", lat);
  wifiClient.print("&long=");
  wifiClient.printf("%02.5f", lon);
  wifiClient.print(" HTTP/1.1\r\n");
  wifiClient.print("Host: " TWEETER_HOST "\r\n");
  wifiClient.print("Content-Type: multipart/form-data;boundary=\"boundary\"\r\n");
  wifiClient.printf("Content-Length: %d\r\n", (*jpgLen)+1000); //This is uh... good enough
  wifiClient.print("Connection: close\r\n");
  wifiClient.print("\r\n"
                   "--boundary\r\n"
                   "Content-Disposition: form-data; name=\"image\"; filename=\"Untitled.jpg\"\r\n"
                   "\r\n");
  int written = wifiClient.write(*jpgBuffer, *jpgLen);
  wifiClient.print("\r\n--boundary--\r\n\r\n");

  Serial.printf("[makeTweetRequest] - %d bytes out of %d written from JPEG\n", written, *jpgLen);
  Serial.println("[makeTweetRequest] - Successfully written request!");

  //////////////////////////////////////////////////////////////////////
  //Await response with timeout
  Serial.printf("[makeTweetRequest] - Awaiting response (read timeout %d ms)...", timeout);
  int startTime = millis();
  while(wifiClient.available() == 0) {
    if(millis() - startTime > timeout) {
      Serial.println(" timed out :(");
      wifiClient.stop();
      return;
    }
    WAIT_MS(1000);
    Serial.print(".");
  }
  Serial.println(" success!");

  //////////////////////////////////////////////////////////////////////
  //Output response
  Serial.println("[makeTweetRequest] -------------------------Response Start");
  while(wifiClient.available()) {
    Serial.println(wifiClient.readStringUntil('\r'));
  }
  Serial.println("[makeTweetRequest] -------------------------Response End");

  //Close wifi client
  wifiClient.stop();
}