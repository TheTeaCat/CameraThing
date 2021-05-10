// main.cpp
// main entry points

#include <Arduino.h>
#include <main.h>
#include "esp_camera.h"

/////////////////////////////////////////////////////////////////////////////
// utilities

// delay/yield macros
#define WAIT_A_SEC   vTaskDelay(    1000/portTICK_PERIOD_MS); // 1 second
#define WAIT_SECS(n) vTaskDelay((n*1000)/portTICK_PERIOD_MS); // n seconds
#define WAIT_MS(n)   vTaskDelay(       n/portTICK_PERIOD_MS); // n millis

String ip2str(IPAddress address) { // utility for printing IP addresses
  return
    String(address[0]) + "." + String(address[1]) + "." +
    String(address[2]) + "." + String(address[3]);
}

/////////////////////////////////////////////////////////////////////////////
// Global variables

//Our web server
// WebServer webServer(80);

//The pin of the LED
int ledPin = 26;
//The button pin we're using
int buttonPin = 25;

//Whether the button is currently down
bool buttonDown = false;

//The current loop number
int loopN = 0;

/////////////////////////////////////////////////////////////////////////////
// camera config

#define CONFIG_OV7725_SUPPORT 1

//WROVER-KIT PIN Map
#define CAM_PIN_PWDN    23 //power down is not used
#define CAM_PIN_RESET   19 //software reset will be performed
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
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_GRAYSCALE,//YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QQVGA,//QQVGA-QXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12, //0-63 lower number means higher quality
    .fb_count = 1 //if more than one, i2s runs in continuous mode. Use only with JPEG
};

esp_err_t camera_init(){
    //power up the camera if PWDN pin is defined
    if(CAM_PIN_PWDN != -1){
        pinMode(CAM_PIN_PWDN, OUTPUT);
        digitalWrite(CAM_PIN_PWDN, LOW);
    }

    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}

esp_err_t camera_capture(){
    //acquire a frame
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }
    //replace this with your own function
    //process_image(fb->width, fb->height, fb->format, fb->buf, fb->len);
  
    Serial.println(fb->width);
    Serial.println(fb->height);
    Serial.println(fb->format);
    Serial.println(fb->len);

    String data = "";
    String scale = " .:-=+*#%@";
    for(int i = 0; i < fb->len; i++) {
      uint8_t v = *(fb->buf+i);
      data += scale[v/26];
      data += scale[v/26];
      if(i % fb->width == 0) {
        data += "\n";
      }
    }
    Serial.println(data);

    //return the frame buffer back to the driver for reuse
    esp_camera_fb_return(fb);
    return ESP_OK;
}

/////////////////////////////////////////////////////////////////////////////
// arduino-land entry points

void setup() {
  Serial.begin(115200);
  Serial.println("arduino started");
  Serial.printf("\nwire pins: sda=%d scl=%d\n", SDA, SCL);

  //Setup pin for button
  pinMode(buttonPin, INPUT_PULLUP);

  //Setup pins for LED & button
  pinMode(ledPin, OUTPUT);

  # ifdef CONFIG_OV7725_SUPPORT
  Serial.println("CONFIG_OV7725_SUPPORT set!");
  # endif
  # ifdef CONFIG_ESP32_SPIRAM_SUPPORT
  Serial.println("CONFIG_ESP32_SPIRAM_SUPPORT set!");
  # endif

  //Setup camera (this may take a while)
  Serial.println("Setting up camera...");
  camera_init();
  Serial.println("Set up camera!");
}

void loop() {
  //Determine if the button is down
  bool prevButtonDown = buttonDown;
  buttonDown = digitalRead(buttonPin) == LOW;

  //Log that the button state has changed
  if(prevButtonDown != buttonDown){
    if(buttonDown) {
      Serial.println("Button is pressed! Taking a picture...");
    } else {
      Serial.println("Button is no longer pressed!");
    }
  }

  //Take a picture if the button has just been pressed
  if(!prevButtonDown && buttonDown){
      camera_capture();
  }

  //Turn on the LED if the button is down
  digitalWrite(ledPin, (buttonDown) ? HIGH : LOW);

  //Give background processes some time
  if(loopN++ % 100000  == 0) {
    WAIT_MS(10);
  }
}