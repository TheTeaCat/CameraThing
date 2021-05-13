// geolocate.cpp
// Config, setup and utils for making the GPS featherwing give us lat and longs

#include <Arduino.h>
#include <Adafruit_GPS.h>
#include "geolocate.h"

//Globals
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);

/////////////////////////////////////////////////////////////////////////////
// Setup

//setupGPS sets up the GPS featherwing. Returns false for fail, true for 
//success.
bool setupGPS() {
  //9600 NMEA is default baud for Adafruit GPS board. May fail here if GPS board
  //fails to init.
  bool success = GPS.begin(9600);
  if (!success) {
    return false;
  }

  //We only one the GPGGA
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_GGAONLY);

  //Set update rate to 1HS
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  //We ain't using an antenna stop giving me $PGTOP sentences I don't care
  GPS.sendCommand(PGCMD_NOANTENNA);

  //Return true for success!
  return true;
}

/////////////////////////////////////////////////////////////////////////////
// Utils

// Uncomment this to send the last NMEA from the GPS to serial
// # define DEBUG_LAST_NMEA_TO_SERIAL

//geolocate takes two float pointers and a timeout and attempts to get a 
//longitude and latitude to write to the pointers within timeout milliseconds.
//Returns false if it fails, or true for success.
bool geolocate(float* lat, float* lon, int timeout) {
  int started = millis();

  for(;;) {
    //Check timeout first
    int deltaT = millis()-started;
    if (deltaT > timeout) {
      //Return false for fail if we timeout
      return false;
    }

    //Read from GPS
    GPS.read();

    //Attempt to parse new NMEA if recieved...
    if (GPS.newNMEAreceived()) {
      //Print it to serial before parsing if debugging
      #ifdef DEBUG_LAST_NMEA_TO_SERIAL
        Serial.println(GPS.lastNMEA());
      #endif

      //If it doesn't parse, loop again
      if (!GPS.parse(GPS.lastNMEA())) {
        Serial.printf("[Geolocate] - Failed to parse NMEA, deltaT: %d ms\n", deltaT);
        continue;
      }

      //Otherwise print success...
      Serial.printf("[Geolocate] - Successfully parsed NMEA, deltaT: %d ms\n", deltaT);
      Serial.printf("[Geolocate] - lat: %f, long: %f\n", GPS.latitudeDegrees, GPS.longitudeDegrees);

      //...and set the lat and lon in the pointers given
      *lat = GPS.latitudeDegrees;
      *lon = GPS.longitudeDegrees;

      //then return true for success!
      return true;
    };
  }
}