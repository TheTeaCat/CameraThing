// asyncLed.cpp
// Defines a handy AsyncLED class for animating an LED without blocking the main
// thread. Uses PWM to make nice animations.

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

//Kills any animations and turns the LED on
void AsyncLED::on() {
  Serial.printf("[AsyncLED.on] [Pin %d] - Turning LED on\n", pin);
  killAnimation();
  ledcWrite(15, 255);
}

//Kills any animations and turns the LED off
void AsyncLED::off(){
  Serial.printf("[AsyncLED.off] [Pin %d] - Turning LED off\n", pin);
  killAnimation();
  ledcWrite(15, 0);
};

//Kills any animations and sets the LED to a given brightness (0-255)
void AsyncLED::set(int dutyCycle){
  Serial.printf("[AsyncLED.off] [Pin %d] - Turning LED off\n", pin);
  killAnimation();
  ledcWrite(15, dutyCycle);
};

/////////////////////////////////////////////////////////////////////////////
// Blink animation
// A blink animation cycles on->off->on with a delay between each switch like a
// square wave
// An example of a blink animation:
//     ___     ___     ___     ___     ___     ___     ___     ___     ___   
//    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |  
//    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |  
//    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |  
//    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |  
// ___|   |___|   |___|   |___|   |___|   |___|   |___|   |___|   |___|   |__

struct blinkAnimationParams {
  int pin;
  int channel;
  int delay;
};

blinkAnimationParams* currBlinkAnimationParams;

void AsyncLED::blink(int delay){
  Serial.printf("[AsyncLED.blink] [Pin %d] - Blinking with %d ms delay\n", pin, delay);

  //If we're already animating, kill the current animation before starting a new 
  //one (we can't have two going at once!)
  if(animating) {
    killAnimation();
  }

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

  //Create new animation task
  xTaskCreatePinnedToCore(
    blinkAnimationLoop, "blinkAnimationLoop", 10000, currBlinkAnimationParams, 1, &animationTask, 0
  );

  //Tell everyone we're animating now
  animating = true;
}

/////////////////////////////////////////////////////////////////////////////
// Triangle animation
// A triangle animation cycles off->on->off like blink but ramps up brightness 
// linearly, then back down linearly over the given period
// An example of a triangle animation:
//           .'.                   .'.                   .'.                 
//         .'   '.               .'   '.               .'   '.               
//       .'       '.           .'       '.           .'       '.           .'
//     .'           '.       .'           '.       .'           '.       .'  
//   .'               '.   .'               '.   .'               '.   .'    
// .'                   '.'                   '.'                   '.'      

struct triangleAnimationParams {
  int pin;
  int channel;
  int period;
};

triangleAnimationParams* currTriangleAnimationParams;

void AsyncLED::triangle(int period) {
  Serial.printf("[AsyncLED.triangle] [Pin %d] - Doin' a funki triangle with %d ms period\n", pin, period);

  //If we're already animating, kill the current animation before starting a new 
  //one (we can't have two going at once!)
  if(animating) {
    killAnimation();
  }

  //Create animation params
  currTriangleAnimationParams = new triangleAnimationParams{pin,channel,period};

  //Declate animation loop used for this animation
  auto triangleAnimationLoop = [](void* p) {
    //Cast void pointer to animation params
    triangleAnimationParams* params = (triangleAnimationParams*)p;

    //Get vals from params struct
    float period = (float)(params->period);

    //Start our animation loop!
    int start = millis();
    for(;;) {
      //Calculate a new duty cycle
      int now = millis();
      float delta = (float)((now-start)%params->period);
      int dutyCycle = 0;
      if(delta < period/2) {
        dutyCycle = round(255.0 * (2 * (delta/period)));
      } else {
        dutyCycle = round(255.0 * (2 - (2 * (delta/period))));
      }
      ledcWrite(params->channel, dutyCycle);
      //Do it again in 30ms
      WAIT_MS(30);
    }
  };

  //Create new animation task
  xTaskCreatePinnedToCore(
    triangleAnimationLoop, "triangleAnimationLoop", 10000, currTriangleAnimationParams, 1, &animationTask, 0
  );

  //Tell everyone we're animating now
  animating = true;
}

/////////////////////////////////////////////////////////////////////////////
// Breathe animation
// A breathing animation cycles off->on->off like triangle or blink, but ramps
// up and down following a 1-cos wave that loops every `period` milliseconds
// An example of a breathe animation:
//               _.-'-._                           _.-'-._                    
//            _.'       '._                     _.'       '._                 
//          _'             '_                 _'             '_               
//        _'                 '_             _'                 '_             
//     _.'                     '._       _.'                     '._       _.
// _.-'                           '-._.-'                           '-._.-'   

struct breatheAnimationParams {
  int pin;
  int channel;
  int period;
};

breatheAnimationParams* currBreatheAnimationParams;

void AsyncLED::breathe(int period) {
  Serial.printf("[AsyncLED.breathe] [Pin %d] - Breathing with %d ms period\n", pin, period);

  //If we're already animating, kill the current animation before starting a new 
  //one (we can't have two going at once!)
  if(animating) {
    killAnimation();
  }

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
      int dutyCycle = round(255.0 * ((1.0 - cos((2*delta/period) * M_PI)) / 2.0));
      ledcWrite(params->channel, dutyCycle);
      //Do it again in 30ms
      WAIT_MS(30);
    }
  };

  //Create new animation task
  xTaskCreatePinnedToCore(
    breatheAnimationLoop, "breatheAnimationLoop", 10000, currBreatheAnimationParams, 1, &animationTask, 0
  );

  //Tell everyone we're animating now
  animating = true;
}

/////////////////////////////////////////////////////////////////////////////
// Throb animation
// A throbbing animation cycles off->on->off like breathe but has a different  
// attack (off->on) to decay (on->off) period, so it can seem less or more 
// aggressive than the standard breathing by altering the ratio between the 
// durations, and overall durations of these two attack and decay values.
// `attack` and `decay` are both periods in milliseconds. This animation is 
// exactly the same as breathe when `attack` is the same as `delay`.
// An example of a throb animation (fast attack, slow decay):
//         _-'--..__                            _-'--..__                     
//       _'         ''._                      _'         ''._                 
//      _               '_                   _               '_               
//     _                  '_                _                  '_             
//    _                     '..__          _                     '..__        
// _-'                           ''--..__-'                           ''--.._

struct throbAnimationParams {
  int pin;
  int channel;
  int attack;
  int decay;
};

throbAnimationParams* currThrobAnimationParams;

void AsyncLED::throb(int attack, int decay) {
  Serial.printf("[AsyncLED.throb] [Pin %d] - Throbbing with %d ms attack and %d ms decay\n", pin, attack, decay);

  //If we're already animating, kill the current animation before starting a new 
  //one (we can't have two going at once!)
  if(animating) {
    killAnimation();
  }

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

  //Create new animation task
  xTaskCreatePinnedToCore(
    throbAnimationLoop, "throbAnimationLoop", 10000, currThrobAnimationParams, 1, &animationTask, 0
  );

  //Tell everyone we're animating now.
  animating = true;
}

/////////////////////////////////////////////////////////////////////////////
// Step animation
// A step animation increases brightness linearly over `period` ms in `steps`
// steps (levels of brightness), then returns to off.
// An example of a 6-step animation, looping just over thrice:
//                     ___                     ___                     ___ 
//                 ___|   |                ___|   |                ___|   |
//             ___|       |            ___|       |            ___|       |
//         ___|           |        ___|           |        ___|           |
//     ___|               |    ___|               |    ___|               |
// ___|                   |___|                   |___|                   |__

struct stepAnimationParams {
  int pin;
  int channel;
  int period;
  int steps;
};

stepAnimationParams* currStepAnimationParams;

void AsyncLED::step(int period, int steps) {
  Serial.printf("[AsyncLED.step] [Pin %d] - Stepping with %d ms period in %d steps\n", pin, period, steps);

  //If we're already animating, kill the current animation before starting a new 
  //one (we can't have two going at once!)
  if(animating) {
    killAnimation();
  }

  //Create animation params
  currStepAnimationParams = new stepAnimationParams{pin,channel,period,steps};

  //Declate animation loop used for this animation
  auto stepAnimationLoop = [](void* p) {
    //Cast void pointer to animation params
    stepAnimationParams* params = (stepAnimationParams*)p;

    //Get vals from params struct
    float period = (float)(params->period);
    float steps = (float)(params->steps);

    //Start our animation loop!
    int start = millis();
    for(;;) {
      //Calculate a new duty cycle
      int now = millis();
      float pos = fmod((float)(now-start), period)/period;
      int dutyCycle = dutyCycle = 255.0 * round(pos*steps - 0.5) / (steps - 1);
      
      //Write the dutyCycle to the channel
      ledcWrite(params->channel, dutyCycle);
      //Do it again in 30ms
      WAIT_MS(30);
    }
  };

  //Create new animation task
  xTaskCreatePinnedToCore(
    stepAnimationLoop, "stepAnimationLoop", 10000, currStepAnimationParams, 1, &animationTask, 0
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
  if (currTriangleAnimationParams != nullptr) {
    Serial.printf("[AsyncLED.killAnimation] [Pin %d] - Deleting blink animation params\n", pin);
    delete(currTriangleAnimationParams);
    currTriangleAnimationParams = nullptr;
  }
  if (currBreatheAnimationParams != nullptr) {
    Serial.printf("[AsyncLED.killAnimation] [Pin %d] - Deleting breathe animation params\n", pin);
    delete(currBreatheAnimationParams);
    currBreatheAnimationParams = nullptr;
  }
  if (currThrobAnimationParams != nullptr) {
    Serial.printf("[AsyncLED.killAnimation] [Pin %d] - Deleting throb animation params\n", pin);
    delete(currThrobAnimationParams);
    currThrobAnimationParams = nullptr;
  }

  //Set animating flag to false so we don't try and delete a null task later
  animating = false;
}
