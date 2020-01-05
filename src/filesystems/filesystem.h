<<<<<<< HEAD
#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <Arduino.h>

  
  std::string dogNameRetriever();

  bool setupWifi();
  void setupWifiScanner();
  void saveData(long int t, int state, float Tag1, int Tag2);
  void uploadData();

  bool WiFi_Off();


=======
#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <Arduino.h>

  
  std::string dogNameRetriever();

  bool setupWifi();
  void setupWifiScanner();
  void saveData(long int t, int state, float Tag1, int Tag2);
  void uploadData();

  bool WiFi_Off();


>>>>>>> 623652be972f6b7d398fd3a2db13eecae52e669e
#endif