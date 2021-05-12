// camera.h
// Exports the utils for dealing with the camera

#include "esp_camera.h"

//Utils
void setupCamera();

//Framebuffer accessors
camera_fb_t* getFrameBuffer();
void returnFrameBuffer(camera_fb_t* fb);

//Framebuffer renderer for debugging
void frameBufferToSerial(camera_fb_t* fb);