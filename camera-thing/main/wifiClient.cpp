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

      //We allow each attempt to run for 10 seconds
      bool success = false;
      Serial.print("[setupWifi] - Connecting...");
      for(int trial = 0; trial < 10; trial++) {
        if (WiFi.status() != WL_CONNECTED) {
          Serial.print(".");
          WAIT_MS(1000);
        } else {
          Serial.print(" connected!\n");
          success = true;
          break;
        }
        if(trial == 9) {
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
  String req = String("GET /health HTTP/1.1\r\n") + \
    "Host: " + TWEETER_HOST + "\r\n\r\n";
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

void makeTweetRequest(int timeout, float lat, float lon, camera_fb_t* frameBuffer) {
  //////////////////////////////////////////////////////////////////////
  //Connect to tweeter
  Serial.printf("[makeTweetRequest] - Connecting to %s:%d...\n", TWEETER_HOST, TWEETER_PORT);
  if (!wifiClient.connect(TWEETER_HOST, TWEETER_PORT)) {
    Serial.println("[makeTweetRequest] - Failed to connect :(");
    return;
  }

  //////////////////////////////////////////////////////////////////////
  //Construct request head and tail (before and after frameBuffer data)
  String reqHead = String("POST /tweet") + \
    "?auth=" + TWEETER_AUTH_TOKEN + \
    "&lat="+String(lat,6) + \
    "&long="+String(lon,6) + \
    " HTTP/1.1\r\n" + \
    "Host: " + TWEETER_HOST + "\r\n" + \
    "Content-Type: multipart/form-data;boundary=\"boundary\"\r\n" + \
    "Content-Length: " + String(frameBuffer->len) + "\r\n" + \
    + "\r\n" +
    + "--boundary\r\n" + \
    + "Content-Disposition: form-data; name=\"image\"; filename=\"frameBuffer.png\"\r\n" + \
    + "\r\n";
  String reqTail = String("\r\n--boundary--\r\n\r\n");
  Serial.println("[makeTweetRequest] -------------------------Request Start");
  Serial.print(reqHead + "[FRAMEBUFFER DATA]" + reqTail);
  Serial.println("[makeTweetRequest] -------------------------Request End");

  //////////////////////////////////////////////////////////////////////
  //Write request head
  Serial.println("[makeTweetRequest] - Making request...");
  wifiClient.print(reqHead);
  Serial.println("[makeTweetRequest] -------------------------Request Start (actual)");
  Serial.print(reqHead);

  //////////////////////////////////////////////////////////////////////
  //Write image data
  //Funky data time (I think I need to manually encode the framebuffer to a PNG 
  //here, yikes...)
  int written = wifiClient.write(frameBuffer->buf, frameBuffer->len);
  Serial.printf("[%d bytes out of %d written from frame buffer]", written, frameBuffer->len);

  //////////////////////////////////////////////////////////////////////
  //Write request tail
  wifiClient.print(reqTail);
  Serial.print(reqTail);
  Serial.println("[makeTweetRequest] -------------------------Request End (actual)");
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