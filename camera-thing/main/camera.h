// camera.h
// Exports the utils for dealing with the camera

#include "esp_camera.h"

//Setup method
bool setupCamera();

//Image getters
bool getJPEG(uint8_t** jpgBuffer, size_t* jpgLen);

//Debug utils
void frameBufferToSerial(camera_fb_t* frameBuffer);