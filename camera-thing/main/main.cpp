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
// #define MOCKDELAY 2500

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
// Setup

void setup() {
  //Make LED breathe during setup
  myLed.breathe(500);

  //Setup serial output
  Serial.begin(115200);
  Serial.println("arduino started");
  Serial.printf("\nwire pins: sda=%d scl=%d\n", SDA, SCL);

  //Setup pin for button
  pinMode(buttonPin, INPUT_PULLUP);

  //Setup GPS. Should be pretty fast...
  Serial.println("Setting up GPS...");
  bool gpsSuccess = setupGPS();
  if (!gpsSuccess) {
    Serial.println("Failed to setup GPS :(");
    myLed.blink(100); //blink(100) for hardware failure
    WAIT_MS(5000);
    ESP.restart();
  }
  Serial.println("Set up GPS!");

  //Setup camera. This may take a while if something has gone wrong...
  Serial.println("Setting up camera...");
  bool cameraSuccess = setupCamera();
  if (!cameraSuccess) {
    Serial.println("Failed to setup camera :(");
    myLed.blink(100); //blink(100) for hardware failure
    WAIT_MS(5000);
    ESP.restart();
  }
  Serial.println("Set up camera!");

  //Setup wifi. This may take a while normally...
  Serial.println("Setting up WiFi...");
  bool wifiSuccess = setupWifiManager(60, 10);
  if (!wifiSuccess) {
    Serial.println("Failed to WiFi connection :(");
    myLed.triangle(500); //triangle(500) for internet failure
    WAIT_MS(5000);
    ESP.restart();
  }
  Serial.println("Set up WiFi!");

  //Check tweeter service is available. This may also take a while normally...
  Serial.println("Checking tweeter service is accessible...");
  bool tweeterSuccess = checkTweeterAccessible(60000);
  if (!tweeterSuccess) {
    Serial.println("Failed to check tweeter service health :(");
    myLed.triangle(500); //triangle(500) for internet failure
    WAIT_MS(5000);
    ESP.restart();
  }
  Serial.println("Tweeter is accessible!");

  //If we're mocking delays, add MOCKDELAY ms to the startup time.
  #ifdef MOCKDELAY
    Serial.printf("[MockDelay] - Mocking a longer setup time by waiting %d ms...\n", MOCKDELAY);
    WAIT_MS(MOCKDELAY);
  #endif

  //Turn LED off again now setup
  myLed.off();
}

/////////////////////////////////////////////////////////////////////////////
// Loop

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
    //Turn on the LED while we get a JPEG from the camera
    myLed.on();

    //Get a JPEG from the camera
    uint8_t *jpgBuffer;
    size_t jpgLen;
    bool gotJPEG = getJPEG(&jpgBuffer, &jpgLen);

    //If we fail to get a frame buffer, signal an err and return.
    if (!gotJPEG || jpgLen == 0) {
      Serial.println("[Loop] - Failed to get JPEG :(");
      myLed.blink(100); //blink(100) for hardware failure
      WAIT_MS(2000);
      myLed.off();
      delete jpgBuffer;
      return;
    }

    //If we're mocking delays, delay to pretend it takes a while to get a
    //frame buffer from the camera module
    #ifdef MOCKDELAY
      Serial.printf("[MockDelay] - Mocking a slower camera by waiting %d ms...\n", MOCKDELAY);
      WAIT_MS(MOCKDELAY);
    #endif

    //Output success
    Serial.println("[Loop] - Got JPEG from camera");

    //Turn the LED off now the camera is done
    myLed.off();

    //////////////////////////////////////////////////////////////////////
    //Geolocation
    //This step is optional and only completed if the user doesn't press down 
    //the button after a second
    float lat; float lon;

    //Give the user 3.5 seconds to hold the button down to disable geolocation
    Serial.println("[Loop] - Getting geolocation preference from user...");
    myLed.step(1500,4); //Communicate with 3 loops of a 4-step 1.5s animation
    WAIT_MS(3500);
    //If the user has held down the button, no geolocation is used.
    bool geolocationEnabled = digitalRead(buttonPin) == HIGH;
    if(!geolocationEnabled) {
      Serial.println("[Loop] - Geolocation disabled.");
    } else {
      Serial.println("[Loop] - Geolocation enabled.");
    }

    if (geolocationEnabled) {
      //Communicate geolocating with a fast breathe
      myLed.breathe(250);

      //Get geolocation with geolocate() function
      bool gotGeolocation = geolocate(&lat, &lon, 1000);

      //If we're mocking delays, delay to pretend it takes a while to get a 
      //geolocation from the GPS featherwing
      #ifdef MOCKDELAY
        Serial.printf("[MockDelay] - Mocking a slower GPS by waiting %d ms...\n", MOCKDELAY);
        WAIT_MS(MOCKDELAY);
      #endif

      //If there is some err getting the geolocation data, signal an err and 
      //return.
      if(!gotGeolocation) {
        myLed.blink(100); //blink(100) for hardware failure
        WAIT_MS(2000);
        myLed.off();
        delete jpgBuffer;
        return;
      }

      //Output geolocation to serial
      Serial.printf("[Loop] - Got geolocation, lat: %f, long: %f\n", lat, lon);

      //Turn the LED off now the GPS is done
      myLed.off();
    }

    //////////////////////////////////////////////////////////////////////
    //Upload
    //Communicate uploading by throbbing with fast attack, slow decay
    myLed.throb(900,100);

    //Upload the information to the tweet service
    bool tweetSuccess = true;
    //bool tweetSuccess = makeTweetRequest(30000,geolocationEnabled,lat,lon,&jpgBuffer,&jpgLen);

    //If we're mocking delays, delay to pretend it takes a while to send the
    //data to the tweeter service
    #ifdef MOCKDELAY
      Serial.printf("[MockDelay] - Mocking a slower tweet response by waiting %d ms...\n", MOCKDELAY);
      WAIT_MS(MOCKDELAY);
    #endif

    //If there is some err getting the data to the tweeter, signal an err and 
    //return.
    if(!tweetSuccess) {
      myLed.triangle(500); //triangle(500) for internet failure
      WAIT_MS(2000);
      myLed.off();
      delete jpgBuffer;
      return;
    }

    //Turn the LED off now the upload is done
    myLed.off();

    //////////////////////////////////////////////////////////////////////
    //Cleanup
    //Delete the jpgBuffer now we're done with it so we don't have a memory 
    //leak!
    delete jpgBuffer;

    //////////////////////////////////////////////////////////////////////
    //Buttondown warning     
    //Start flashing quickly (will only stay on if the button is still pressed
    //down; otherwise turned off in the next loop)
    myLed.blink(50);
  }

  //Give background processes some time
  if(loopN++ % 100000  == 0) {
    WAIT_MS(10);
  }
}