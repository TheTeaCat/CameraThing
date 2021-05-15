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
  #define LED_BLUE  13

  //Create LED instance
  AsyncLED blueLED = AsyncLED(LED_BLUE, 14);

  //setupGPRSClient sets up the GPRS connection for the TinyGSM client.
  bool setupGPRSClient() {
    //Make blue LED breathe
    blueLED.breathe(1000);

    //Setup power
    pinMode(SIM800L_PWRKEY, OUTPUT);
    pinMode(SIM800L_RST, OUTPUT);
    pinMode(SIM800L_POWER, OUTPUT);
    digitalWrite(SIM800L_PWRKEY, LOW);
    digitalWrite(SIM800L_RST, HIGH);
    digitalWrite(SIM800L_POWER, HIGH);

    //Give SIM800L 10 seconds to startup
    Serial.println("Giving 10s startup time to SIM800L...");
    WAIT_MS(10000);

    //Initialise SerialAT...
    Serial.print("Starting SerialAT...");
    SerialAT.begin(4800, SERIAL_8N1, SIM800L_RX, SIM800L_TX);
    WAIT_MS(3000);
    Serial.println(" done :)");

    //Restart SIM800L...
    Serial.print("Initializing modem...");
    if(!modem.restart()){
      Serial.println(" fail :(");
      blueLED.blink(100);
      WAIT_MS(2000);
      return false;
    }
    Serial.println(" success!");

    //Setup GPRS...
    Serial.printf("Connecting to APN '%s'...", APN);
    if (!modem.gprsConnect(APN, GPRS_USER, GPRS_PASS)) {
      Serial.println(" fail :(");
      blueLED.blink(100);
      WAIT_MS(2000);
      return false;
    }
    Serial.println(" success!");

    //Turn LED off and return true for success!
    blueLED.off();
    return true;
  }
#endif