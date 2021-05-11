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

void setupGPS() {
  //9600 NMEA is default baud for Adafruit GPS board
  GPS.begin(9600);
  //We only one the GPGGA
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_GGAONLY);
  //Set update rate to 1HS
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  //We ain't using an antenna stop giving me $PGTOP sentences I don't care
  GPS.sendCommand(PGCMD_NOANTENNA);
}

/////////////////////////////////////////////////////////////////////////////
// Utils

// Uncomment this to send the last NMEA from the GPS to serial
// # define PRINTLASTNMEA

void geolocate(float* lat, float* lon, int timeout) {
  int started = millis();
  for(;;) {
    //Check timeout first
    int deltaT = millis()-started;
    if (deltaT > timeout) {
      return;
    }

    //Read from GPS
    GPS.read();

    //Attempt to parse new NMEA if recieved...
    if (GPS.newNMEAreceived()) {
      //Print it to serial before parsing if debugging
      #ifdef PRINTLASTNMEA
        Serial.println(GPS.lastNMEA());
      #endif

      if (!GPS.parse(GPS.lastNMEA())) {
        //If it doesn't parse, loop again
        Serial.printf("[GEOLOCATE] - Failed to parse NMEA, deltaT: %d ms\n", deltaT);
        continue;
      }

      //Otherwise print success if debugging...
      Serial.printf("[GEOLOCATE] - Successfully parsed NMEA, deltaT: %d ms\n", deltaT);
      Serial.printf("[GEOLOCATE] - lat: %f, long: %f\n", GPS.latitudeDegrees, GPS.longitudeDegrees);

      //...and set the lat and lon in the pointers given
      *lat = GPS.latitudeDegrees;
      *lon = GPS.longitudeDegrees;
      return;
    };
  }
}