#ifndef RTCMEMORY_H
#define RTCMEMORY_H
#include <Arduino.h>

//for checking last wakeup times
extern RTC_DATA_ATTR time_t TIME_DATAAQ;
extern RTC_DATA_ATTR time_t TIME_CLOUD;
extern RTC_DATA_ATTR time_t TIME_STEPS;

//for syncing
extern RTC_DATA_ATTR time_t TIME_NOW;

//for sleep schedule
extern RTC_DATA_ATTR uint64_t TIME_AT_SLEEP;
extern RTC_DATA_ATTR uint64_t TIME_AT_WAKE;

//for passing forward intent to next boot cycles 
extern RTC_DATA_ATTR int bootCount;
extern RTC_DATA_ATTR int devState;
extern RTC_DATA_ATTR int lastDevStateUpdate;

//extern RTC_DATA_ATTR std::string DOGNAME = "PetPal";

#endif 