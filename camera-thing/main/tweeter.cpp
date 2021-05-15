// tweeter.cpp
// Utils for querying the tweeter service's endpoints

#include <Arduino.h>
#include "utils.h"
#include "secrets.h"
#include "esp_camera.h"


//Setup sets up the webClient that the queries to the tweeter service will be 
//made by. It will setup a WiFi or GSM connection depending upon what values are
//set in secrets.h
#ifdef WIFI_SSID
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
  //If we're using GPRS we need to restart the SIM800L every time
  #ifdef APN
    bool setupGPRS = setupNetworkConn();
    if (!setupGPRS) {
      Serial.println("[checkTweeterAccessible] - Failed to setup GPRS connection :(");
      return false;
    }
  #endif

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
  //If we're using GPRS we need to restart the SIM800L every time
  #ifdef APN
    bool setupGPRS = setupNetworkConn();
    if (!setupGPRS) {
      Serial.println("[checkTweeterAccessible] - Failed to setup GPRS connection :(");
      return false;
    }
  #endif

  //Connect to tweeter
  Serial.printf("[makeTweetRequest] - Connecting to %s:%d...\n", TWEETER_HOST, TWEETER_PORT);
  if (!webClient.connect(TWEETER_HOST, TWEETER_PORT)) {
    Serial.println("[makeTweetRequest] - Failed to connect :(");
    return false;
  }

  //Construct request body
  char *reqHead = 
    "POST /tweet?auth=" TWEETER_AUTH_TOKEN " HTTP/1.1\r\n"
    "Host: " TWEETER_HOST "\r\n"
    "Content-Type: multipart/form-data;boundary=\"boundary\"\r\n"
    "Content-Length: 5000\r\n" //This is uh... good enough
    "Connection: close\r\n"
    "\r\n"
    "--boundary\r\n"
    "Content-Disposition: form-data; name=\"image\"; filename=\"Untitled.jpg\"\r\n"
    "\r\n";
  char *reqTail = 
    "\r\n"
    "--boundary--\r\n"
    "\r\n";

  //Write request body
  Serial.println("[makeTweetRequest] - Writing request...");
  int headWritten = webClient.write((uint8_t*)reqHead, strlen(reqHead));

  //Keep track of how many bytes we've written
  int jpgWritten = 0;

  //Max size of each chunk
  const int maxChunkSize = 512;

  //@todo: make this not get stuck together
  while(true) {
    //Figure out how many bytes remain to be written
    int remainingBytes = *jpgLen - jpgWritten;

    //Set the chunkSize to the min of maxChunkSize or the remaining bytes
    int chunkSize = maxChunkSize;
    if (remainingBytes < maxChunkSize) {
      chunkSize = remainingBytes;
    }

    //Write up to chunkSize bytes
    int written = webClient.write((*jpgBuffer)+jpgWritten, chunkSize);
    Serial.printf("[makeTweetRequest] - Written %d bytes of JPEG; %d out of %d so far\n", written, *jpgLen, jpgWritten);

    //Add the number of bytes written to the accumulator for logging later
    jpgWritten += written;

    //Break once we've written all the bytes!
    if(jpgWritten >= *jpgLen) {
      break;
    }
  }

  int tailWritten = webClient.write((uint8_t*)reqTail, strlen(reqTail));

  // //Display JPEG bytes written and success in serial
  Serial.printf("[makeTweetRequest] - %d bytes out of %d written from request head\n", headWritten, strlen(reqHead));
  Serial.printf("[makeTweetRequest] - %d bytes out of %d written from JPEG\n", jpgWritten, *jpgLen);
  Serial.printf("[makeTweetRequest] - %d bytes out of %d written from request tail\n", tailWritten, strlen(reqTail));
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