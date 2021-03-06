// main.cpp
// Main entry points

#include <Arduino.h>
#include "main.h"
#include "utils.h"
#include "secrets.h"
#include "camera.h"
#include "tweeter.h"
#include "asyncLed.h"

#ifdef APN
  #include "gprsClient.h"
#endif

//I would like to use the GPS featherwing but I have actually just ran out of 
//GPIO pins...
//#include "geolocate.h"

/////////////////////////////////////////////////////////////////////////////
// Global variables

//The pin of the LED
int ledPin = 12;
//The button pin we're using
int buttonPin = 13;

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
  myLed.breathe(1000);

  //Setup serial output
  Serial.begin(115200);
  Serial.println("[setup] - arduino started");
  Serial.printf("\n[setup] - wire pins: sda=%d scl=%d\n", SDA, SCL);

  //Setup pin for button
  pinMode(buttonPin, INPUT_PULLUP);

  //We only setupNetworkConn on startup if we're using WiFI; 2G has to be done 
  //before every request.
  #ifdef WIFI_SSID
    //Setup wifi. This may take a while normally...
    Serial.println("[setup] - Setting up network connection...");
    bool networkSuccess = setupNetworkConn();
    if (!networkSuccess) {
      Serial.println("[setup] - Failed to connect to network :(");
      //Signal network failure
      myLed.step(1000,4);
      WAIT_MS(3000);
      ESP.restart();
    }
    Serial.println("[setup] - Set up network connection!");
  #endif

  //Check tweeter service is available. This may also take a while normally...
  //It's not absolutely neccessary to do this, but a good idea if you want to be
  //certain that your images should upload when you take them. It can be 
  //disabled by defining FAST_STARTUP.
  #define FAST_STARTUP
  #ifndef FAST_STARTUP
    Serial.println("[setup] - Checking tweeter service is accessible...");
    bool tweeterSuccess = checkTweeterAccessible(60000);
    if (!tweeterSuccess) {
      Serial.println("[setup] - Failed to check tweeter service health :(");
      //Signal network failure
      myLed.step(1000,4);
      WAIT_MS(3000);
      ESP.restart();
    }
    Serial.println("[setup] - Tweeter is accessible!");
  #endif

  // !!! Currently not used, see FIRMWARE.md !!!
  //Setup GPS. Should be pretty fast...
  // Serial.println("[setup] - Setting up GPS...");
  // bool gpsSuccess = setupGPS();
  // if (!gpsSuccess) {
  //   Serial.println("[setup] - Failed to setup GPS :(");
  //   //Signal hardware failure
  //   myLed.flash(100);
  //   WAIT_MS(2000);
  //   ESP.restart();
  // }
  // Serial.println("[setup] - Set up GPS!");

  //Setup camera. This may take a while if something has gone wrong...
  Serial.println("[setup] - Setting up camera...");
  bool cameraSuccess = setupCamera();
  if (!cameraSuccess) {
    Serial.println("[setup] - Failed to setup camera :(");
    //Signal hardware failure
    myLed.flash(100);
    WAIT_MS(2000);
    ESP.restart();
  }
  Serial.println("[setup] - Set up camera!");

  //Blink the LED now to signal the CameraThing is on
  myLed.blink(2950,50);
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
      Serial.println("[loop] - Button is pressed! Taking a picture...");
    } else {
      Serial.println("[loop] - Button is no longer pressed!");
      myLed.blink(2950,50);
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

    //If we fail to get a frame buffer, signal an err and restart
    if (!gotJPEG || jpgLen == 0) {
      Serial.println("[loop] - Failed to get JPEG :(");
      myLed.flash(100); //flash(100) for hardware failure
      WAIT_MS(2000);
      myLed.off();
      delete jpgBuffer;
      ESP.restart();
    }

    //Output success
    Serial.printf("[loop] - Got JPEG from camera of %d bytes\n", jpgLen);

    //Turn the LED off now the camera is done
    myLed.off();

    //////////////////////////////////////////////////////////////////////
    //Geolocation
    //This step is optional and only completed if the user doesn't press down 
    //the button after a second
    float lat; float lon;
    bool geolocationEnabled = false;

    // !!! Currently not used, see FIRMWARE.md !!!
    // //Give the user 2.4 seconds to hold the button down to disable geolocation
    // Serial.println("[loop] - Getting geolocation preference from user...");

    // //Communicate input request with fast breathe
    // myLed.breathe(300);
    // WAIT_MS(2400);

    // //If the user has released the button after 2 seconds, geodata isn't used.
    // bool geolocationEnabled = digitalRead(buttonPin) == LOW;
    // if(!geolocationEnabled) {
    //   Serial.println("[loop] - Geolocation disabled.");
    // } else {
    //   Serial.println("[loop] - Geolocation enabled.");
    // }

    // if (geolocationEnabled) {
    //   //Communicate geolocating with a fast breathe
    //   myLed.throb(900,100);

    //   //Get geolocation with geolocate() function
    //   bool gotGeolocation = geolocate(&lat, &lon, 1000);

    //   //If there is some err getting the geolocation data, signal an err and 
    //   //return.
    //   if(!gotGeolocation) {
    //     myLed.flash(100); //flash(100) for hardware failure
    //     WAIT_MS(2000);
    //     myLed.off();
    //     delete jpgBuffer;
    //     ESP.restart();
    //   }

    //   //Output geolocation to serial
    //   Serial.printf("[loop] - Got geolocation, lat: %f, long: %f\n", lat, lon);

    //   //Turn the LED off now the GPS is done
    //   myLed.off();
    // }

    //////////////////////////////////////////////////////////////////////
    //Upload
    //Communicate uploading by throbbing with fast attack, slow decay
    myLed.throb(900,100);

    //Will contain the URL of the tweet
    String tweetURL;

    //Upload the information to the tweet service
    bool tweetSuccess = makeTweetRequest(
      30000,
      &tweetURL,
      geolocationEnabled,
      lat,
      lon,
      &jpgBuffer,
      &jpgLen
    );

    //If there is some err getting the data to the tweeter, signal an err and 
    //return.
    if(!tweetSuccess) {
      //Signal network failure
      myLed.step(1000,4);
      WAIT_MS(3000);
      myLed.off();
      //Restart
      ESP.restart();
    } else {
      Serial.print("Tweet URL: ");
      Serial.println(tweetURL);
    }

    //Turn the LED off now the upload is done
    myLed.off();

    //////////////////////////////////////////////////////////////////////
    //Success SMS 
    //If we're using GPRS, we can send an SMS containing the tweet URL
    #ifdef APN
      if (tweetSuccess) {
        myLed.throb(100,900);
        bool SMSSuccess = sendTweetText(tweetURL);
        if (!SMSSuccess) {
          Serial.println("Failed to send SMS :(");
          myLed.flash(100); //flash(100) for hardware failure
          WAIT_MS(3000);
          ESP.restart();
        } else {
          Serial.println("Successfully sent SMS!");
        }
        myLed.off();
      }
    #endif

    //////////////////////////////////////////////////////////////////////
    //Cleanup
    //Delete the jpgBuffer now we're done with it so we don't have a memory 
    //leak!
    delete jpgBuffer;

    //////////////////////////////////////////////////////////////////////
    //Buttondown warning     
    //Start flashing quickly (will only stay on if the button is still pressed
    //down; otherwise turned back to blinking in the next loop)
    myLed.flash(50);
  }

  //Give background processes some time
  if(loopN++ % 100000  == 0) {
    WAIT_MS(10);
  }
}