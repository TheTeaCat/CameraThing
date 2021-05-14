#include <Arduino.h>
#include "secrets.h"
#include "utils.h"
#include "asyncLed.h"

#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb
#include <TinyGsmClient.h>

//Initialise modem
#define SerialAT Serial2
TinyGsm modem(SerialAT);
// #include <StreamDebugger.h>
// StreamDebugger debugger(SerialAT, Serial);
// TinyGsm modem(debugger);

//Initialise client
TinyGsmClient client(modem);

//TTGO T-Call pin definitions
#define SIM800L_RX     26
#define SIM800L_TX     27
#define SIM800L_PWRKEY 4
#define SIM800L_RST    5
#define SIM800L_POWER  23
#define LED_BLUE  13

//Create LED instance
AsyncLED blueLED = AsyncLED(LED_BLUE, 14);

//setupGSM sets up the GPRS connection for the TinyGSM client.
bool setupGSM() {
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