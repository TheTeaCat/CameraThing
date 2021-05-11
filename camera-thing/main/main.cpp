// main.cpp
// Main entry points

#include <Arduino.h>
#include "main.h"
#include "utils.h"
#include "camera.h"
#include "geolocate.h"
#include "wifiClient.h"
#include "asyncLed.h"

/////////////////////////////////////////////////////////////////////////////
// Debugging flags

//In order to model the behaviour under less than optimal circumstances 
//(particularly as this is using a cellular connection), MOCKDELAY can be set
//to add a number of WAIT_MS() calls in places where there are operations that
//_could_ take a considerable amount of time. The delays applied are the value
//of MOCKDELAY in ms, for example MOCKDELAY=1000 would apply delays of 1 second.
//All the instances of MOCKDELAY being used should occur in this file, main.cpp.
#define MOCKDELAY 2500

/////////////////////////////////////////////////////////////////////////////
// utilities

// delay/yield macros
#define WAIT_A_SEC   vTaskDelay(    1000/portTICK_PERIOD_MS); // 1 second
#define WAIT_SECS(n) vTaskDelay((n*1000)/portTICK_PERIOD_MS); // n seconds
#define WAIT_MS(n)   vTaskDelay(       n/portTICK_PERIOD_MS); // n millis

/////////////////////////////////////////////////////////////////////////////
// Global variables

//The pin of the LED
int ledPin = 26;
//The button pin we're using
int buttonPin = 25;

//Whether the button is currently down
bool buttonDown = false;

//The current loop number
int loopN = 0;

//Our LED instance (we'll use PWM channel 15)
AsyncLED myLed = AsyncLED(ledPin, 15);

/////////////////////////////////////////////////////////////////////////////
// arduino-land entry points

void setup() {
  Serial.begin(115200);
  Serial.println("arduino started");
  Serial.printf("\nwire pins: sda=%d scl=%d\n", SDA, SCL);

  //Make LED breathe during setup
  myLed.breathe(500);

  //Setup pin for button
  pinMode(buttonPin, INPUT_PULLUP);

  //Setup GPS
  setupGPS();

  //Setup camera (this may take a while if something has gone wrong)
  Serial.println("Setting up camera...");
  setupCamera();
  Serial.println("Set up camera!");

  //Setup wifi (this may also take a while)
  Serial.println("Setting up WiFi...");
  setupWifiManager();
  Serial.println("Set up WiFi!");

  //If we're mocking delays, add 5s to the startup time.
  #ifdef MOCKDELAY
    Serial.printf("[MockDelay] - Mocking a longer setup time by waiting %d ms...\n", MOCKDELAY);
    WAIT_MS(MOCKDELAY);
  #endif

  //Turn LED off again now setup
  myLed.off();
}

void loop() {
  //Determine if the button is down
  bool prevButtonDown = buttonDown;
  buttonDown = digitalRead(buttonPin) == LOW;

  //Log that the button state has changed
  if(prevButtonDown != buttonDown){
    if(buttonDown) {
      Serial.println("[Loop] - Button is pressed! Taking a picture...");
    } else {
      Serial.println("[Loop] - Button is no longer pressed!");
      myLed.off();
    }
  }

  //Take a picture if the button has just been pressed
  if(!prevButtonDown && buttonDown){
      //////////////////////////////////////////////////////////////////////
      //Taking photograph
      //Turn on the LED while the camera is reading
      myLed.on();

      //@todo: refactor to return JPEG data
      //takePicture();

      //If we're mocking delays, delay to pretend it takes a while to get an
      //image from the camera module
      #ifdef MOCKDELAY
        Serial.printf("[MockDelay] - Mocking a slower camera by waiting %d ms...\n", MOCKDELAY);
        WAIT_MS(MOCKDELAY);
      #endif

      //Turn the LED off now the camera is done.
      myLed.off();

      //////////////////////////////////////////////////////////////////////
      //Geolocation
      //Communicate geolocating by throbbing with slow attack, fast decay
      myLed.throb(900,100);

      //Get geolocation with geolocate() function
      float lat; float lon;
      geolocate(&lat, &lon, 1000);

      //If we're mocking delays, delay to pretend it takes a while to get a 
      //geolocation from the GPS featherwing
      #ifdef MOCKDELAY
        Serial.printf("[MockDelay] - Mocking a slower GPS by waiting %d ms...\n", MOCKDELAY);
        WAIT_MS(MOCKDELAY);
      #endif

      //Output geolocation to serial
      Serial.printf("[Loop] - Got geolocation, lat: %f, long: %f\n", lat, lon);

      //Turn the LED off now the GPS is done.
      myLed.off();

      //////////////////////////////////////////////////////////////////////
      //Upload

      //@todo

      //////////////////////////////////////////////////////////////////////
      //Buttondown warning     
      //Start flashing quickly (will only stay on if the button is still pressed
      //down; otherwise turned off in the next loop)
      myLed.blink(100);
  }

  //Give background processes some time
  if(loopN++ % 100000  == 0) {
    WAIT_MS(10);
  }
}