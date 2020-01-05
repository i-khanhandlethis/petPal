#include "scheduler.h"
#include "health/health.h"
//#include "rtcMemory.h"
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <soc/rtc.h>
#include <soc/sens_reg.h>

RTC_DATA_ATTR time_t TIME_DATAAQ = 0;
RTC_DATA_ATTR time_t TIME_CLOUD = 0;
RTC_DATA_ATTR time_t TIME_STEPS = 0;

RTC_DATA_ATTR time_t TIME_NOW = 0;
RTC_DATA_ATTR uint64_t TIME_AT_SLEEP = 0;
RTC_DATA_ATTR uint64_t TIME_AT_WAKE = 0;

RTC_DATA_ATTR uint64_t TIME_SLEPT = 0;

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int devState = 0;
RTC_DATA_ATTR int lastDevStateUpdate = 0;

uint64_t reg_a;
uint64_t reg_b;
uint64_t reg_c;

extern int BatteryVoltage;
extern "C"
{
#include <esp_clk.h>
}
void wakeUpProtocols()
{
  //Time Sync Calculator
  TIME_AT_WAKE = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get());
  uint64_t timeDiffus = TIME_AT_WAKE - TIME_AT_SLEEP;
  uint64_t timeDiffs = (TIME_AT_WAKE - TIME_AT_SLEEP) / 1000000;

  uint64_t timeErrorCatcher = TIME_SLEPT * 1.5;

reg_a = READ_PERI_REG(SENS_SAR_START_FORCE_REG);
reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
reg_c = READ_PERI_REG(SENS_SAR_MEAS_START2_REG);

  if (timeDiffs >= 0 && timeDiffs < timeErrorCatcher)
  {
    TIME_NOW = TIME_NOW + timeDiffs;
    setTime(TIME_NOW);
    Serial.print("(RTC Sync)");
  }
  else
  {
    TIME_NOW = TIME_NOW + TIME_SLEPT;
    setTime(TIME_NOW);
    Serial.print("(RTC Sync Failure)");
    Serial.print("(Rough Sync)");
  }

  Serial.printf("Now is : %d-%02d-%02d %02d:%02d:%02d |", year(now()), month(now()), day(now()), hour(now()), minute(now()), second(now()));
  printf(" Time Slept: %" PRIu64 "us |", timeDiffus);
  Serial.print(" Boot number: ");
  Serial.println(bootCount);
  digitalWrite(26, HIGH);
}

void deepSleepProtocols()
{
  int TIME_TO_SLEEP = wakeSchedule() * uS_TO_S_FACTOR;

  if (devState == 1)
  {
    TIME_TO_SLEEP = 40 * uS_TO_S_FACTOR;
    btStop();
  }
  else if (devState == 0)
  {
    //battVoltageConverter(BatteryVoltage);
  }
WRITE_PERI_REG(SENS_SAR_START_FORCE_REG, reg_a);  // fix ADC registers
WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
WRITE_PERI_REG(SENS_SAR_MEAS_START2_REG, reg_c);
  Serial.print("Setup ESP32 to sleep for " + String(TIME_TO_SLEEP / uS_TO_S_FACTOR) + " Seconds with State " + String(devState) + " | ");
  TIME_SLEPT = TIME_TO_SLEEP / uS_TO_S_FACTOR;
  bootCount++;
  TIME_AT_SLEEP = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get());
  Serial.print("Awake for:");
  int TIME_AWAKE = (TIME_AT_SLEEP - TIME_AT_WAKE) / 1000000;
  Serial.print(TIME_AWAKE);
  Serial.println("s");
  Serial.println("zzz\n\n\n");

  digitalWrite(26, LOW);
  TIME_NOW = now();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, 1);
  ESP.deepSleep(TIME_TO_SLEEP);

  Serial.println("This should never be printed. If it does, call the exorcist.");
}