#include <Arduino.h>
#include "esp_camera.h"

//Main
void setup();
void loop();

//Camera utils
void setupCamera();
void takePicture();
void frameBufferToSerial(camera_fb_t*);