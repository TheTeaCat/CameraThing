// asyncLed.cpp
// Defines a handy AsyncLED class for animating an LED without blocking the main
// thread using PWM

#include <Arduino.h>
#include "utils.h"
#include "asyncLed.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor

AsyncLED::AsyncLED(int p, int c) {
  pin = p;
  channel = c;
  ledcSetup(channel, 5000, 8);
  ledcAttachPin(pin, channel);  
}

/////////////////////////////////////////////////////////////////////////////
// Basic utils

void AsyncLED::on() {
  Serial.printf("[AsyncLED.on] [Pin %d] - Turning LED on\n", pin);
  killAnimation();
  ledcWrite(15, 255);
}

void AsyncLED::off(){
  Serial.printf("[AsyncLED.off] [Pin %d] - Turning LED off\n", pin);
  killAnimation();
  ledcWrite(15, 0);
};

void AsyncLED::set(int dutyCycle){
  Serial.printf("[AsyncLED.off] [Pin %d] - Turning LED off\n", pin);
  killAnimation();
  ledcWrite(15, dutyCycle);
};

/////////////////////////////////////////////////////////////////////////////
// Blink animation
// A blink animation cycles on->off->on with a delay between each switch

struct blinkAnimationParams {
  int pin;
  int channel;
  int delay;
};

blinkAnimationParams* currBlinkAnimationParams;

void AsyncLED::blink(int delay){
  Serial.printf("[AsyncLED.blink] [Pin %d] - Blinking with %d ms delay\n", pin, delay);

  //Create animation params
  currBlinkAnimationParams = new blinkAnimationParams{pin,channel,delay};

  //Declate animation loop used for this animation
  auto blinkAnimationLoop = [](void* p) {
    //Cast void pointer to animation params
    blinkAnimationParams* params = (blinkAnimationParams*)p;
    //FINALLY, an async for loop!
    for(;;) {
      ledcWrite(params->channel, 255);
      WAIT_MS(params->delay);
      ledcWrite(params->channel, 0);
      WAIT_MS(params->delay);
    }
  };

  //If we're already animating, kill the current animation before starting a new 
  //one (we can't have two going at once!)
  if(animating) {
    killAnimation();
  }

  //Create new animation task
  xTaskCreatePinnedToCore(
    blinkAnimationLoop, "blinkAnimationLoop", 10000, currBlinkAnimationParams, 1, &animationTask, 0
  );

  //Tell everyone we're animating now.
  animating = true;
}

/////////////////////////////////////////////////////////////////////////////
// Breathe animation
// A breathing animation cycles on->off->on like blink but changes brightness by 
// using PWM and a sine wave that loops every `period` milliseconds

struct breatheAnimationParams {
  int pin;
  int channel;
  int period;
};

breatheAnimationParams* currBreatheAnimationParams;

void AsyncLED::breathe(int period) {
  Serial.printf("[AsyncLED.breathe] [Pin %d] - Breathing with %d ms period\n", pin, period);

  //Create animation params
  currBreatheAnimationParams = new breatheAnimationParams{pin,channel,period};

  //Declate animation loop used for this animation
  auto breatheAnimationLoop = [](void* p) {
    //Cast void pointer to animation params
    breatheAnimationParams* params = (breatheAnimationParams*)p;

    //Get vals from params struct
    float period = (float)(params->period);

    //Start our animation loop!
    int start = millis();
    for(;;) {
      //Calculate a new duty cycle
      int now = millis();
      float delta = (float)(now-start);
      int dutyCycle = round(255.0 * ((1.0 - cos((delta/period) * M_PI)) / 2.0));
      ledcWrite(params->channel, dutyCycle);
      //Do it again in 30ms
      WAIT_MS(30);
    }
  };

  //If we're already animating, kill the current animation before starting a new 
  //one (we can't have two going at once!)
  if(animating) {
    killAnimation();
  }

  //Create new animation task
  xTaskCreatePinnedToCore(
    breatheAnimationLoop, "breatheAnimationLoop", 10000, currBreatheAnimationParams, 1, &animationTask, 0
  );

  //Tell everyone we're animating now.
  animating = true;
}

/////////////////////////////////////////////////////////////////////////////
// Throb animation
// A throbbing animation cycles on->off->on like breathe but has a different  
// attack (off->on) to decay (on->off) period, so it can seem less or more 
// aggressive than the standard breathing by altering the ratio between the 
// durations, and overall durations of these two attack and decay values.
// `attack` and `decay` are both periods in milliseconds.

struct throbAnimationParams {
  int pin;
  int channel;
  int attack;
  int decay;
};

throbAnimationParams* currThrobAnimationParams;

void AsyncLED::throb(int attack, int decay) {
  Serial.printf("[AsyncLED.throb] [Pin %d] - Throbbing with %d ms attack and %d ms decay\n", pin, attack, decay);

  //Create animation params
  currThrobAnimationParams = new throbAnimationParams{pin,channel,attack,decay};

  //Declate animation loop used for this animation
  auto throbAnimationLoop = [](void* p) {
    //Cast void pointer to animation params
    throbAnimationParams* params = (throbAnimationParams*)p;

    //Get vals from params struct
    float attack = (float)(params->attack);
    float decay = (float)(params->decay);

    //Start our animation loop!
    int start = millis();
    for(;;) {
      //Calculate a new duty cycle
      int now = millis();
      float pos = fmod((float)(now-start), (attack+decay));
      int dutyCycle = 0;
      if (pos < attack) {
        dutyCycle = round(255.0 * ((1.0 - cos((pos/attack) * M_PI)) / 2.0));
      } else {
        dutyCycle = round(255.0 * ((1.0 + cos(((pos-attack)/decay) * M_PI)) / 2.0));
      }
      //Write the dutyCycle to the channel
      ledcWrite(params->channel, dutyCycle);
      //Do it again in 30ms
      WAIT_MS(30);
    }
  };

  //If we're already animating, kill the current animation before starting a new 
  //one (we can't have two going at once!)
  if(animating) {
    killAnimation();
  }

  //Create new animation task
  xTaskCreatePinnedToCore(
    throbAnimationLoop, "throbAnimationLoop", 10000, currThrobAnimationParams, 1, &animationTask, 0
  );

  //Tell everyone we're animating now.
  animating = true;
}

/////////////////////////////////////////////////////////////////////////////
// Animation utils

//killAnimation stops any currently running animations.
void AsyncLED::killAnimation() {
  //Kill animation task if running
  if(animating) {
    Serial.printf("[AsyncLED.killAnimation] [Pin %d] - Killing current animation\n", pin);
    vTaskDelete(animationTask);
  } else {
    Serial.printf("[AsyncLED.killAnimation] [Pin %d] -  No current animation detected\n", pin);
  }

  //Dispose of any animation params if they're set
  if (currBlinkAnimationParams != nullptr) {
    Serial.printf("[AsyncLED.killAnimation] [Pin %d] - Deleting blink animation params\n", pin);
    delete(currBlinkAnimationParams);
    currBlinkAnimationParams = nullptr;
  }
  if (currBreatheAnimationParams != nullptr) {
    Serial.printf("[AsyncLED.killAnimation] [Pin %d] - Deleting breathe animation params\n", pin);
    delete(currBreatheAnimationParams);
    currBreatheAnimationParams = nullptr;
  }
  if (currBreatheAnimationParams != nullptr) {
    Serial.printf("[AsyncLED.killAnimation] [Pin %d] - Deleting throb animation params\n", pin);
    delete(currThrobAnimationParams);
    currThrobAnimationParams = nullptr;
  }

  //Set animating flag to false so we don't try and delete a null task later
  animating = false;
}
