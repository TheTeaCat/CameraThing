// geolocate.h
// Exports the setup and utils for GPS featherwing

#include <Arduino.h>
#include <Adafruit_GPS.h>

//Setup
void setupGPS();

//Utils
void geolocate(float* lat, float* lon, int timeout);