// main.cpp
// Main entry points

#include <Arduino.h>
#include "main.h"
#include "utils.h"
#include "camera.h"
#include "geolocate.h"
#include "asyncLed.h"

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
      Serial.println("[LOOP] - Button is pressed! Taking a picture...");
    } else {
      Serial.println("[LOOP] - Button is no longer pressed!");
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
      myLed.off();

      //////////////////////////////////////////////////////////////////////
      //Geolocation
      //Communicate geolocating by throbbing with slow attack, fast decay
      myLed.throb(900,100);
      //Get geolocation with geolocate() function
      float lat; float lon;
      geolocate(&lat, &lon, 1000);
      //Output geolocation to serial
      Serial.printf("[LOOP] - Got geolocation, lat: %f, long: %f\n", lat, lon);
      //Stop throbbing
      myLed.off();
  }

  //Give background processes some time
  if(loopN++ % 100000  == 0) {
    WAIT_MS(10);
  }
}