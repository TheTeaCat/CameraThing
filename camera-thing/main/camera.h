// camera.h
// Exports the utils for dealing with the camera

#include "esp_camera.h"

//Utils
bool setupCamera();

//Image getters
bool getJPEG(uint8_t** jpgBuffer, size_t* jpgLen);