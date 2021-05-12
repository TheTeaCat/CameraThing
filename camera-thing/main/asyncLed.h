// asyncLed.h
// Defines and exports the AsyncLED class

class AsyncLED {
  private:
    //The pin of the LED
    int pin;
    int channel;

    //Animation magic
    bool animating = false;
    TaskHandle_t animationTask;
    void killAnimation();

  public:
    //Constructor
    AsyncLED(int p, int c);

    //Basic modifiers
    void on();
    void off();
    void set(int dutyCycle);

    //Animations
    void blink(int delay);
    void triangle(int period);
    void breathe(int period);
    void throb(int attackTime, int decayTime);
};