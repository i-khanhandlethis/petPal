#ifndef SCHEDULER_H
#define SCHEDULER_H


const int uS_TO_S_FACTOR=1000000;

//Sleep.cpp
void wakeUpProtocols();
void deepSleepProtocols();

//timeSchedule.cpp
int workSchedule();
int wakeSchedule();

#endif