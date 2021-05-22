///////////////////////////////////////////////////////////////////////////
// Network credentials
// Note: Either WiFi OR 2G must be used; not both.

//Credentials for using the webClient in wifiClient.h
//If WIFI_SSID is defined then the CameraThing will attempt to use WiFi.
#define WIFI_SSID "Put your SSID in here! :)"
#define WIFI_PASS "Put your password in here! :)"

//GPRS credentials for the webClient in gprsClient.h
//If APN is defined then the CameraThing will attempt to use 2G
#define APN "Put the APN of your provider here :)"
#define GPRS_USER "Put the username for your APN here :)"
#define GPRS_PASS "Put the password for your APN here :)"

//SMS_TARGET is the number to which SMSs will be sent when a tweet is made
#define SMS_TARGET "Your number with +code"

///////////////////////////////////////////////////////////////////////////
// Cloud services

//Details for your tweeter service
#define TWEETER_HOST "Put the hostname of your tweeter service here! :)"
#define TWEETER_PORT Put the port of your tweeter here! :)
#define TWEETER_AUTH_TOKEN "Put the auth token for your tweeter here! :)"
