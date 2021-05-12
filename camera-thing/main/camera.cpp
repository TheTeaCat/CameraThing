// camera.cpp
// Config and utils for dealing with the camera

#include <Arduino.h>
#include "esp_camera.h"
#include "camera.h"

/////////////////////////////////////////////////////////////////////////////
// Config

#define CAM_PIN_PWDN    23
#define CAM_PIN_RESET   19
#define CAM_PIN_XCLK    33
#define CAM_PIN_SIOD    12
#define CAM_PIN_SIOC    13
#define CAM_PIN_D7      36
#define CAM_PIN_D6      15
#define CAM_PIN_D5       4
#define CAM_PIN_D4      32
#define CAM_PIN_D3       5
#define CAM_PIN_D2      14
#define CAM_PIN_D1      18
#define CAM_PIN_D0      22
#define CAM_PIN_VSYNC   34
#define CAM_PIN_HREF    27
#define CAM_PIN_PCLK    39

static camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 10000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_YUV422,//YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QQVGA,//QQVGA-QXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12, //0-63 lower number means higher quality
    .fb_count = 1 //if more than one, i2s runs in continuous mode. Use only with JPEG
};

/////////////////////////////////////////////////////////////////////////////
// Setup

//setupCamera prepares the camera for use.
void setupCamera(){
    //power up the camera if PWDN pin is defined
    if(CAM_PIN_PWDN != -1){
        pinMode(CAM_PIN_PWDN, OUTPUT);
        digitalWrite(CAM_PIN_PWDN, LOW);
    }

    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Framebuffer getter & setter

//takePicture gets an image from the camera's frame buffer and processes it.
void getJPEG(uint8_t** jpgBuffer, size_t* jpgLen){
  //acquire a frame
  camera_fb_t* frameBuffer = esp_camera_fb_get();

  //If it failed, log.
  if (!frameBuffer) {
    Serial.println("[getFrameBuffer] - Camera Capture Failed :(");
  }

  //Compress frameBuffer to JPEG
  bool converted = frame2jpg(frameBuffer, 80, jpgBuffer, jpgLen);
  if (!converted) {
    Serial.println("[getFrameBuffer] - JPEG conversion Failed :(");
  }

  //return the frame buffer back to the driver for reuse
  esp_camera_fb_return(frameBuffer);
}

/////////////////////////////////////////////////////////////////////////////
// Debug utils

//frameBufferToSerial takes a frame buffer and outputs its information to 
//serial, including the image formatted as ASCII characters.
void frameBufferToSerial(camera_fb_t* fb) {
    //Display the frame buffer in serial using an ASCII scale
    String scale = " .:-=+*#%@";
    int len = fb->len;
    for(int i = 0; i < len; i++) {
      Serial.print(scale[*(fb->buf+i)/26]);
      Serial.print(scale[*(fb->buf+i)/26]);
      if(i % (fb->width) == (fb->width)-1) {
        Serial.print("\n");
      }
    }

    //Then display the image metadata
    Serial.printf(
      "Width: %d, Height: %d, Len: %d, Format: %d\n", 
      fb->width, fb->height, fb->len, fb->format
    );
}