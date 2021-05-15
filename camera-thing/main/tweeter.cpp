// tweeter.cpp
// Utils for querying the tweeter service's endpoints

#include <Arduino.h>
#include "utils.h"
#include "secrets.h"
#include "esp_camera.h"


//Setup sets up the webClient that the queries to the tweeter service will be 
//made by. It will setup a WiFi or GSM connection depending upon what values are
//set in secrets.h
#ifdef WIFISSID
  #include "wifiClient.h"
  bool setupNetworkConn() {
    return setupWifiClient(60, 5);
  }
#endif
#ifdef APN
  #include "gprsClient.h"
  bool setupNetworkConn() {
    return setupGPRSClient();
  }
#endif

//checkTweeterAccessible queries the tweeter's /health endpoint and checks that
//the response code provided is 200 OK within a given timeout, in milliseconds.
//It returns false for fail, true for success.
bool checkTweeterAccessible(int timeout) {
  //Connect to tweeter. If fails to connect, log & return false for fail
  Serial.printf("[checkTweeterAccessible] - Connecting to %s:%d...\n", TWEETER_HOST, TWEETER_PORT);
  if (!webClient.connect(TWEETER_HOST, TWEETER_PORT)) {
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
  webClient.print(req);

  //Handle timeout
  Serial.printf("[checkTweeterAccessible] - Awaiting response (read timeout %d ms)...", timeout);
  int startTime = millis();
  while(webClient.available() == 0) {
    if(millis() - startTime > timeout) {
      Serial.println(" timed out :(");
      webClient.stop();
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
  while(webClient.available()) {
    //Get a line...
    String line = webClient.readStringUntil('\r');
    //Print it to serial...
    Serial.println(line);
    //Then check if it states 200 OK...
    if (line == "HTTP/1.1 200 OK") {
      success = true;
    }
  }
  Serial.println("[makeTweetRequest] -------------------------Response End");

  //Close wifi client
  webClient.stop();

  //Return the success flag!
  return success;
}

//makeTweetRequest makes a request to the tweeter service's /tweet endpoint, 
//with a provided latitude, longitude and JPEG data, within a given timeout and
//checks that the tweeter service returns a 201 Created response. If returns
//false for fail, true for success. Pointers to the JPEG data are passed into
//this function to save memory.
bool makeTweetRequest(int timeout, bool geolocationEnabled, float lat, float lon, uint8_t **jpgBuffer, size_t *jpgLen) {
  //Connect to tweeter
  Serial.printf("[makeTweetRequest] - Connecting to %s:%d...\n", TWEETER_HOST, TWEETER_PORT);
  if (!webClient.connect(TWEETER_HOST, TWEETER_PORT)) {
    Serial.println("[makeTweetRequest] - Failed to connect :(");
    return false;
  }

  //Write request body
  Serial.println("[makeTweetRequest] - Writing request...");
  webClient.print("POST /tweet?auth=" TWEETER_AUTH_TOKEN);
  if (geolocationEnabled) {
    //Only add lat&long to GET params if geolocationEnabled
    webClient.printf("&lat=%02.5f&long=%02.5f", lat, lon);
  }
  webClient.print(" HTTP/1.1\r\n");
  webClient.print("Host: " TWEETER_HOST "\r\n");
  webClient.print("Content-Type: multipart/form-data;boundary=\"boundary\"\r\n");
  webClient.printf("Content-Length: %d\r\n", (*jpgLen)+1000); //This is uh... good enough
  webClient.print("Connection: close\r\n\r\n");
  webClient.print("--boundary\r\n"
                   "Content-Disposition: form-data; name=\"image\"; filename=\"Untitled.jpg\"\r\n"
                   "\r\n");
  int written = webClient.write(*jpgBuffer, *jpgLen);
  webClient.print("\r\n--boundary--\r\n\r\n");

  //Display JPEG bytes written and success in serial
  Serial.printf("[makeTweetRequest] - %d bytes out of %d written from JPEG\n", written, *jpgLen);
  Serial.println("[makeTweetRequest] - Finished writing request");

  //Await response from server with timeout
  Serial.printf("[makeTweetRequest] - Awaiting response (read timeout %d ms)...", timeout);
  int startTime = millis();
  while(webClient.available() == 0) {
    if(millis() - startTime > timeout) {
      Serial.println(" timed out :(");
      webClient.stop();
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
  while(webClient.available()) {
    //Get a line...
    String line = webClient.readStringUntil('\r');
    //Print it to serial...
    Serial.println(line);
    //Then check if it states we succeeded...
    if (line == "HTTP/1.1 201 Created") {
      success = true;
    }
  }
  Serial.println("[makeTweetRequest] -------------------------Response End");

  //Close wifi client
  webClient.stop();

  //Return the success flag!
  return success;
}