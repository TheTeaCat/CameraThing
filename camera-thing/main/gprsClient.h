// wifiManager.h
// Exports the utils for connecting to 2G

#ifndef GPRS_CLIENT_USED
  #define GPRS_CLIENT_USED

  //Config for TinyGsmClient.h
  #define TINY_GSM_MODEM_SIM800
  #define TINY_GSM_RX_BUFFER 1024
  #include <TinyGsmClient.h>

  //Initialise modem
  #define SerialAT Serial2
  inline TinyGsm modem(SerialAT);
  // Optionally, use the following for debugging:
  // #include <StreamDebugger.h>
  // StreamDebugger debugger(SerialAT, Serial);
  // TinyGsm modem(debugger);

  //Initialise client
  inline TinyGsmClient webClient(modem);

  //Setup func
  bool setupGPRSClient();

  //Func to send tweet SMS
  bool sendTweetText(String tweetURL);
#endif