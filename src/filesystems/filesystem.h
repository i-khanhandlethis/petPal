#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <Arduino.h>

  
  std::string dogNameRetriever();

  bool setupWifi();
  void setupWifiScanner();
  void saveData(long int t, int state, float Tag1, int Tag2);
  void uploadData();

  bool WiFi_Off();


#endif