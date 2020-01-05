<<<<<<< HEAD
#include "health.h"
#include <Arduino.h>
#include <stdio.h>
#include <esp32-hal-adc.h>
#define VARIANT "esp32"

int sensorPin = 25; // select the input pin for the potentiometer
int sensorValue = 0;
int battVoltageConverter(int v)
{
    sensorValue = v;
        delay(10);
    Serial.print("Battery voltage: ");
    float voltage = 3.3 / 4095 * sensorValue;
    voltage = voltage * 48 / 33;
    Serial.print(voltage);
    Serial.print("v,");
    Serial.println(sensorValue);
    return sensorValue;
}
=======
#include "health.h"
#include <Arduino.h>
#include <stdio.h>
#include <esp32-hal-adc.h>
#define VARIANT "esp32"

int sensorPin = 25; // select the input pin for the potentiometer
int sensorValue = 0;
int battVoltageConverter(int v)
{
    sensorValue = v;
        delay(10);
    Serial.print("Battery voltage: ");
    float voltage = 3.3 / 4095 * sensorValue;
    voltage = voltage * 48 / 33;
    Serial.print(voltage);
    Serial.print("v,");
    Serial.println(sensorValue);
    return sensorValue;
}
>>>>>>> 623652be972f6b7d398fd3a2db13eecae52e669e
