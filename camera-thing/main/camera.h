// camera.h
// Exports the utils for dealing with the camera

#include "esp_camera.h"

//Utils
void setupCamera();
void takePicture();
void frameBufferToSerial(camera_fb_t* fb);