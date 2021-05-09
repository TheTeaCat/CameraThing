// main.cpp
// main entry points

#include <Arduino.h>
#include <main.h>

/////////////////////////////////////////////////////////////////////////////
// utilities

// delay/yield macros
#define WAIT_A_SEC   vTaskDelay(    1000/portTICK_PERIOD_MS); // 1 second
#define WAIT_SECS(n) vTaskDelay((n*1000)/portTICK_PERIOD_MS); // n seconds
#define WAIT_MS(n)   vTaskDelay(       n/portTICK_PERIOD_MS); // n millis

/////////////////////////////////////////////////////////////////////////////
// Global variables

//The pin of the LED
int ledPin = 12;
//The button pin we're using
int buttonPin = 14;

//Whether the button is currently down
bool buttonDown = false;

//The current loop number
int loopN = 0;

/////////////////////////////////////////////////////////////////////////////
// arduino-land entry points

void setup() {
  Serial.begin(115200);
  Serial.println("arduino started");
  Serial.printf("\nwire pins: sda=%d scl=%d\n", SDA, SCL);

  //Setup pin for button
  pinMode(buttonPin, INPUT_PULLUP);

  //Setup pins for LED & button
  pinMode(ledPin, OUTPUT);
}

void loop() {
  //Determine if the button is down
  bool prevButtonDown = buttonDown;
  buttonDown = digitalRead(buttonPin) == LOW;

  //Log that the button state has changed
  if(prevButtonDown != buttonDown){
    if(buttonDown) {
      Serial.println("Button is pressed!");
    } else {
      Serial.println("Button is no longer pressed!");
    }
  }

  //Turn on the yellow LED if the button is down
  digitalWrite(ledPin, (buttonDown) ? HIGH : LOW);

  //Give background processes some time
  if(loopN++ % 100000  == 0) {
    WAIT_MS(10);
  }
}