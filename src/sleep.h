#ifndef SLEEP_H
#define SLEEP_H

extern int TIME_TO_SLEEP;
extern int wakeUpState;

const int uS_TO_S_FACTOR=1000000;

void deepSleepProtocols(int t, int state);


#endif