#include <Arduino.h>
#include "utils.h"
#include "asyncLed.h"

#include "Adafruit_FONA.h"

// const char apn[] = "payandgo.o2.co.uk";
// const char user[] = "payandgo";
// const char pass[] = "password";

//TTGO T-Call pin definitions
#define SIM800L_RX     27
#define SIM800L_TX     26
#define SIM800L_PWRKEY 4
#define SIM800L_RST    5
#define SIM800L_POWER  23
#define LED_BLUE  13

//Create LED instance
AsyncLED blueLED = AsyncLED(LED_BLUE, 14);

//Initialize modem
HardwareSerial *sim800lSerial = &Serial1;
Adafruit_FONA sim800l = Adafruit_FONA(SIM800L_PWRKEY);

bool setupGSM() {
  //Make blue LED breathe
  blueLED.breathe(1000);

  //Setup power
  pinMode(SIM800L_POWER, OUTPUT);
  digitalWrite(SIM800L_POWER, HIGH);

  //Give SIM800L 10 seconds to startup
  Serial.println("Giving 10s startup time to SIM800L...");
  WAIT_MS(10000);

  //Initialise SIM800L
  //Using 4800 baud because apparently it makes it easier to read?
  sim800lSerial->begin(4800, SERIAL_8N1, SIM800L_TX, SIM800L_RX);
  if (!sim800l.begin(*sim800lSerial)) {
    Serial.println("Couldn't find GSM SIM800L");
    blueLED.blink(50);
    WAIT_MS(5000);
    return false;
  }
  Serial.println("GSM SIM800L is OK");  
  blueLED.blink(500);
  WAIT_MS(5000);

  //Get IMEI from SIM
  char imei[16] = {0};
  uint8_t imeiLen = sim800l.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("SIM card IMEI: "); Serial.println(imei);
  } else {
    Serial.println("Failed to read IMEI from SIM.");
    return false;
  }

  //Return true for success
  return true;
}