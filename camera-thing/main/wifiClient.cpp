// wifiManager.cpp
// Config and utils for connecting to a wifi connection to test sending data to
// the tweeter service

#include <WiFi.h>
#include "utils.h"
#include "secrets.h"
#include "esp_camera.h"

//Utility for printing IP addresses
String ip2str(IPAddress address) {
  return
    String(address[0]) + "." + String(address[1]) + "." +
    String(address[2]) + "." + String(address[3]);
}

//setupWifiClient attempts to setup a wifi connection `maxAttempts` times, each 
//attempt consisting of up to `maxTrials` tests of WiFi.status() to check if 
//it's WL_CONNECTED yet. Each trial is spaced 1 second apart, so this will take
//maxTrials * maxAttempts seconds to terminate at worst.
bool setupWifiClient(int maxTrials, int maxAttempts) {
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
    //We try maxAttempts times to connect to WiFi before giving up
    for(int attempt = 0; attempt < maxAttempts; attempt++) {
      Serial.printf("[setupWifi] - Trying to connect to '%s'\n", WIFISSID);
      
      //Attempt to begin wifi conn
      WiFi.begin(WIFISSID, WIFIPASSWORD);

      //We allow each attempt to run for maxTrials seconds
      bool success = false;
      Serial.print("[setupWifi] - Connecting...");
      for(int trial = 0; trial < maxTrials; trial++) {
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
