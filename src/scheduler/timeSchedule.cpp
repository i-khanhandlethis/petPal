<<<<<<< HEAD
#include "scheduler.h"
#include "rtcMemory.h"
#include <TimeLib.h>

int cloudThreshold = 20 * 60, dataThreshold = 15 * 60, stepThreshold = 9 * 60;
int workSchedule()
{

    uint64_t timediff = now() - TIME_CLOUD;
    if (timediff > cloudThreshold)
    {
        Serial.print("(Cloud sync)");
        return 1;
    }

    timediff = now() - TIME_DATAAQ;
    if (timediff > dataThreshold)
    {
        Serial.print("(Data Pull)");
        return 2;
    }

    timediff = now() - TIME_STEPS;
    if (timediff > stepThreshold)
    {
        Serial.print("(Step Pull)");
        return 3;
    }
    Serial.println("No schedule to trigger. prolly first boot\nIf it aint, try a hard reset");
    return 0;
}

int wakeSchedule()
{
    double lowestTimeNeeded = now();

    //time needed = 15 mins - timepassed since last check
    lowestTimeNeeded = cloudThreshold - (now() - TIME_CLOUD);
    int TimeFlag = 1;

    int nextStepTime = minute(now()) % 10;
    nextStepTime = 10 - nextStepTime;
    Serial.print("next step minutes");
    Serial.println(nextStepTime);
    double timeTillDataAQ = dataThreshold - (now() - TIME_DATAAQ);
    if (timeTillDataAQ < lowestTimeNeeded)
    {
        lowestTimeNeeded = timeTillDataAQ;
        TimeFlag = 2;
    }

    double timeTillSteps = nextStepTime * 60;
    if (timeTillSteps < lowestTimeNeeded)
    {
        lowestTimeNeeded = timeTillSteps;
        TimeFlag = 3;
    }

    Serial.print("The next activity is: ");
    switch (TimeFlag)
    {
    case 1:
        Serial.println("Cloud sending");
        break;
    case 2:
        Serial.println("Data acquisition");
        break;
    case 3:
        Serial.println("Steps acquisition");
        break;
    }
    lowestTimeNeeded = lowestTimeNeeded + 5;

    if (lowestTimeNeeded < 10)
    {
        return 10;
    }
    else
    {
        return lowestTimeNeeded;
    }
}
=======
#include "scheduler.h"
#include "rtcMemory.h"
#include <TimeLib.h>

int cloudThreshold = 20 * 60, dataThreshold = 15 * 60, stepThreshold = 9 * 60;
int workSchedule()
{

    uint64_t timediff = now() - TIME_CLOUD;
    if (timediff > cloudThreshold)
    {
        Serial.print("(Cloud sync)");
        return 1;
    }

    timediff = now() - TIME_DATAAQ;
    if (timediff > dataThreshold)
    {
        Serial.print("(Data Pull)");
        return 2;
    }

    timediff = now() - TIME_STEPS;
    if (timediff > stepThreshold)
    {
        Serial.print("(Step Pull)");
        return 3;
    }
    Serial.println("No schedule to trigger. prolly first boot\nIf it aint, try a hard reset");
    return 0;
}

int wakeSchedule()
{
    double lowestTimeNeeded = now();

    //time needed = 15 mins - timepassed since last check
    lowestTimeNeeded = cloudThreshold - (now() - TIME_CLOUD);
    int TimeFlag = 1;

    int nextStepTime = minute(now()) % 10;
    nextStepTime = 10 - nextStepTime;
    Serial.print("next step minutes");
    Serial.println(nextStepTime);
    double timeTillDataAQ = dataThreshold - (now() - TIME_DATAAQ);
    if (timeTillDataAQ < lowestTimeNeeded)
    {
        lowestTimeNeeded = timeTillDataAQ;
        TimeFlag = 2;
    }

    double timeTillSteps = nextStepTime * 60;
    if (timeTillSteps < lowestTimeNeeded)
    {
        lowestTimeNeeded = timeTillSteps;
        TimeFlag = 3;
    }

    Serial.print("The next activity is: ");
    switch (TimeFlag)
    {
    case 1:
        Serial.println("Cloud sending");
        break;
    case 2:
        Serial.println("Data acquisition");
        break;
    case 3:
        Serial.println("Steps acquisition");
        break;
    }
    lowestTimeNeeded = lowestTimeNeeded + 5;

    if (lowestTimeNeeded < 10)
    {
        return 10;
    }
    else
    {
        return lowestTimeNeeded;
    }
}
>>>>>>> 623652be972f6b7d398fd3a2db13eecae52e669e
