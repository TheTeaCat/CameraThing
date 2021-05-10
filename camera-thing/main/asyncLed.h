#include <Arduino.h>

// delay/yield macros
#define WAIT_A_SEC   vTaskDelay(    1000/portTICK_PERIOD_MS); // 1 second
#define WAIT_SECS(n) vTaskDelay((n*1000)/portTICK_PERIOD_MS); // n seconds
#define WAIT_MS(n)   vTaskDelay(       n/portTICK_PERIOD_MS); // n millis


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
    void breathe(int period);
};