#include <Arduino.h>
#include "sleep.h"

void deepSleepProtocols(int t, int state){
  TIME_TO_SLEEP = t * uS_TO_S_FACTOR;
  wakeUpState = state;
  Serial.println("Setup ESP32 to sleep for " + String(t) + " Seconds with State "+String(wakeUpState));
  Serial.println("Going to sleep now");
    ESP.deepSleep(TIME_TO_SLEEP);

  Serial.println("This should never be printed. If it does, call the exorcist.");
}