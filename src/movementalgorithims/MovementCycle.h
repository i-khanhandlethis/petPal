#ifndef MOVEMENTCYCLE_H
#define MOVEMENTCYCLE_H
#include <TimeLib.h>

void setupXLGYRO();
bool checkLayingState();
int checkPedometer();
void rawXLGPoll(time_t timestamp);

#endif