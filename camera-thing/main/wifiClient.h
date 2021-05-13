// wifiManager.h
// Exports the utils for connecting to a wifi connection to test sending data to
// the tweeter service

//Setup
bool setupWifiManager(int maxTrials, int maxAttempts);

//Tweeter service communication utils
bool checkTweeterAccessible(int timeout);
bool makeTweetRequest(int timeout, float lat, float lon, uint8_t **jpgBuffer, size_t *jpgLen);