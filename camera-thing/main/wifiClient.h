// wifiManager.h
// Exports the utils for connecting to a wifi connection to test sending data to
// the tweeter service

//Setup
void setupWifiManager();

//Utils
void checkTweeterAccessible(int timeout);
void makeTweetRequest(int timeout, float lat, float lon, camera_fb_t* frameBuffer);