#include <Arduino.h>
#include "esp_camera.h"

//Utils
void setupCamera();
void takePicture();
void frameBufferToSerial(camera_fb_t* fb);