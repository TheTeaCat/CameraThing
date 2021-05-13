// tweeter.h
// Utils for querying the tweeter service's endpoints

//Sets up the network conn we'll use
bool setupNetworkConn();

//Gets from /health
bool checkTweeterAccessible(int timeout);

//Posts to /tweet
bool makeTweetRequest(int timeout, bool geolocationEnabled, float lat, float lon, uint8_t **jpgBuffer, size_t *jpgLen);