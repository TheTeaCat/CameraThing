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

bool setupWifiManager() {
  //Return immediately if the apropriate config vars aren't set
  #ifndef WIFISSID
    Serial.println("[setupWifi] - Couldn't connect to WiFi; WIFISSID not set.");
    return false;
  #endif
  #ifndef WIFIPASSWORD
    Serial.println("[setupWifi] - Couldn't connect to WiFi; WIFIPASSWORD not set.");
    return false;
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

      //If success, log it and return true
      if(success) {
        Serial.printf("[setupWifi] - WiFi successfully connected to SSID: '%s'\n", WIFISSID);
        Serial.printf("[setupWifi] - Device IP: %s\n", ip2str(WiFi.localIP()));
        return true;
      }
    }

    //If we ran out of attempts, log and return false for fail
    Serial.printf("[setupWifi] - WiFi failed to connect to SSID: '%s'\n", WIFISSID);
    return false;
  #endif
}

bool checkTweeterAccessible(int timeout) {
  //Connect to tweeter. If fails to connect, log & return false for fail
  Serial.printf("[checkTweeterAccessible] - Connecting to %s:%d...\n", TWEETER_HOST, TWEETER_PORT);
  if (!wifiClient.connect(TWEETER_HOST, TWEETER_PORT)) {
    Serial.println("[checkTweeterAccessible] - Failed to connect :(");
    return false;
  }

  //Construct request
  char *req = "GET /health HTTP/1.1\r\n"
              "Host: " TWEETER_HOST "\r\n"
              "Connection: close\r\n\r\n";

  //Display request in serial
  Serial.println("[checkTweeterAccessible] -------------------------Request Start");
  Serial.print(req);
  Serial.println("[checkTweeterAccessible] -------------------------Request End");

  //Make request
  Serial.println("[checkTweeterAccessible] - Making request...");
  wifiClient.print(req);

  //Handle timeout
  Serial.printf("[checkTweeterAccessible] - Awaiting response (read timeout %d ms)...", timeout);
  int startTime = millis();
  while(wifiClient.available() == 0) {
    if(millis() - startTime > timeout) {
      Serial.println(" timed out :(");
      wifiClient.stop();
      return false;
    }
    WAIT_MS(1000);
    Serial.print(".");
  }
  Serial.println(" success!");

  //Get response
  Serial.println("[makeTweetRequest] -------------------------Response Start");
  //Will be set to true if the response states 200 OK
  bool success = false;
  //While there are bytes left to print...
  while(wifiClient.available()) {
    //Get a line...
    String line = wifiClient.readStringUntil('\r');
    //Print it to serial...
    Serial.println(line);
    //Then check if it states 200 OK...
    if (line == "HTTP/1.1 200 OK") {
      success = true;
  }
  }
  Serial.println("[makeTweetRequest] -------------------------Response End");

  //Close wifi client
  wifiClient.stop();

  //Return the success flag!
  return success;
}

bool makeTweetRequest(int timeout, float lat, float lon, uint8_t **jpgBuffer, size_t *jpgLen) {
  //Connect to tweeter
  Serial.printf("[makeTweetRequest] - Connecting to %s:%d...\n", TWEETER_HOST, TWEETER_PORT);
  if (!wifiClient.connect(TWEETER_HOST, TWEETER_PORT)) {
    Serial.println("[makeTweetRequest] - Failed to connect :(");
    return false;
  }

  //Write request body
  Serial.println("[makeTweetRequest] - Writing request...");
  wifiClient.printf("POST /tweet?auth=" TWEETER_AUTH_TOKEN 
                    "&lat=%02.5f"
                    "&long=%02.5f", lat, lon);
  wifiClient.print(" HTTP/1.1\r\n");
  wifiClient.print("Host: " TWEETER_HOST "\r\n");
  wifiClient.print("Content-Type: multipart/form-data;boundary=\"boundary\"\r\n");
  wifiClient.printf("Content-Length: %d\r\n", (*jpgLen)+1000); //This is uh... good enough
  wifiClient.print("Connection: close\r\n\r\n");
  wifiClient.print("--boundary\r\n"
                   "Content-Disposition: form-data; name=\"image\"; filename=\"Untitled.jpg\"\r\n"
                   "\r\n");
  int written = wifiClient.write(*jpgBuffer, *jpgLen);
  wifiClient.print("\r\n--boundary--\r\n\r\n");

  //Display JPEG bytes written and success in serial
  Serial.printf("[makeTweetRequest] - %d bytes out of %d written from JPEG\n", written, *jpgLen);
  Serial.println("[makeTweetRequest] - Finished writing request");

  //Await response from server with timeout
  Serial.printf("[makeTweetRequest] - Awaiting response (read timeout %d ms)...", timeout);
  int startTime = millis();
  while(wifiClient.available() == 0) {
    if(millis() - startTime > timeout) {
      Serial.println(" timed out :(");
      wifiClient.stop();
      return false;
    }
    WAIT_MS(1000);
    Serial.print(".");
  }
  Serial.println(" success!");

  //Get response
  Serial.println("[makeTweetRequest] -------------------------Response Start");
  //Will be set to true if the response states a tweet was created
  bool success = false;
  //While there are bytes left to print...
  while(wifiClient.available()) {
    //Get a line...
    String line = wifiClient.readStringUntil('\r');
    //Print it to serial...
    Serial.println(line);
    //Then check if it states we succeeded...
    if (line == "HTTP/1.1 201 Created") {
      success = true;
    }
  }
  Serial.println("[makeTweetRequest] -------------------------Response End");

  //Close wifi client
  wifiClient.stop();

  //Return the success flag!
  return success;
}