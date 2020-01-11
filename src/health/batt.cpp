#include "health.h"
#include <Arduino.h>
#include <stdio.h>
#include "filesystems/filesystem.h"
#include <esp32-hal-adc.h>
#include <soc/sens_reg.h>

#define VARIANT "esp32"
extern int bootCount;
int sensorPin = 25; // select the input pin for the potentiometer
int sensorValue = 0;

 extern uint64_t reg_a;
 extern uint64_t reg_b;
 extern uint64_t reg_c;

int battVoltage()
{
    sensorValue = analogRead(sensorPin);
    Serial.print("Battery voltage: ");
    float voltage = 3.3 / 4095 * sensorValue;
    voltage = voltage * 48 / 33;
    Serial.print(voltage);
    Serial.print("v,");
    Serial.println(sensorValue);
    
    return sensorValue;
}

void adcFixer(){
  WRITE_PERI_REG(SENS_SAR_START_FORCE_REG, reg_a); // fix ADC registers
  WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
  WRITE_PERI_REG(SENS_SAR_MEAS_START2_REG, reg_c);
}