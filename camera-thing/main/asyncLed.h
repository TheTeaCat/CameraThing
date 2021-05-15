// asyncLed.h
// Defines and exports the AsyncLED class

class AsyncLED {
  private:
    int pin; //The pin of the LED
    int channel; //The PWM channel the LED is assigned to

    bool animating = false; //Flag for if an animation is running
    TaskHandle_t animationTask; //Holds the task running the current animation
    void killAnimation(); //Kills the current animation

  public:
    //Constructor
    AsyncLED(int p, int c);

    //Basic modifiers
    void on();
    void off();
    void set(int dutyCycle);

    //Animations
    void flash(int delay);
    void blink(int offPeriod, int onPeriod);
    void triangle(int period);
    void breathe(int period);
    void throb(int attackTime, int decayTime);
    void step(int period, int steps);
};