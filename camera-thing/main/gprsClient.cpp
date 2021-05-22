// gprsClient.cpp
// Config and utils for connecting to a 2G network

//Include secrets.h so we can tell if APN is defined
#include "secrets.h"

#ifdef APN
  #include <Arduino.h>
  #include "gprsClient.h"
  #include "utils.h"
  #include "asyncLed.h"

  //TTGO T-Call pin definitions
  #define SIM800L_RX     26
  #define SIM800L_TX     27
  #define SIM800L_PWRKEY 4
  #define SIM800L_RST    5
  #define SIM800L_POWER  23

  //Remember if we're already turned the SIM800L on so we don't have to wait as
  //long when we make another request
  bool SIM800LOn = false;

  //powerSIM800L sets up power to the SIM800L
  void powerSIM800L() {
    //We don't need to do anything if the SIM800L is already on
    if(SIM800LOn) {
      return;
    }

    //Setup power
    pinMode(SIM800L_PWRKEY, OUTPUT);
    pinMode(SIM800L_RST, OUTPUT);
    pinMode(SIM800L_POWER, OUTPUT);
    digitalWrite(SIM800L_PWRKEY, LOW);
    digitalWrite(SIM800L_RST, HIGH);
    digitalWrite(SIM800L_POWER, HIGH);

    //Give SIM800L 10 seconds to startup
    Serial.println("[setupGPRSClient] - Giving 10s startup time to SIM800L...");
    WAIT_MS(10000);

    //Initialise SerialAT...
    Serial.print("[setupGPRSClient] - Starting SerialAT...");
    SerialAT.begin(4800, SERIAL_8N1, SIM800L_RX, SIM800L_TX);
    WAIT_MS(3000);
    Serial.println(" done :)");

    //Set SIM800LOn to true because we've turned it on, now.
    SIM800LOn = true;
  }

  //setupGPRSClient sets up the GPRS connection for the TinyGSM client. It needs
  //to be called before every use of the webClient if using GPRS.
  bool setupGPRSClient() {
    //Make sure the SIM800L is powered up
    powerSIM800L();

    //Restart SIM800L...
    Serial.print("[setupGPRSClient] - Initializing modem...");
    if(!modem.restart()){
      Serial.println(" fail :(");
      return false;
    }
    Serial.println(" success!");

    //Setup GPRS...
    Serial.printf("[setupGPRSClient] - Connecting to APN '%s'...", APN);
    if (!modem.gprsConnect(APN, GPRS_USER, GPRS_PASS)) {
      Serial.println(" fail :(");
      return false;
    }
    Serial.println(" success!");

    //Return true for success!
    return true;
  }

  //sentTweetText sends a text to SMS_TARGET
  bool sendTweetText(String tweetURL) {
    #ifdef SMS_TARGET
      //Make sure the SIM800L is powered up
      powerSIM800L();

      //Restart SIM800L...
      Serial.print("[setupGPRSClient] - Initializing modem...");
      if(!modem.restart()){
        Serial.println(" fail :(");
        return false;
      }
      Serial.println(" success!");

      //Send text
      bool res = modem.sendSMS(
        SMS_TARGET,
        "Hey, I made a new tweet! \n" + tweetURL
      );

      //Return success status
      return res;
    #else
      return true;
    #endif
  }
#endif