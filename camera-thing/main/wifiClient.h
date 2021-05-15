// wifiClient.h
// Exports the utils for connecting to a WiFi network

#ifndef WIFI_CLIENT_USED
  #define WIFI_CLIENT_USED

  //Include WiFi.h so we can define webClient
  #include <WiFi.h>
  inline WiFiClient webClient;

  //Setup func
  bool setupWifiClient(int maxTrials, int maxAttempts);
#endif