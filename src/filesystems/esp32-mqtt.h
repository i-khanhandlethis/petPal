#ifndef ESP32_MQTT_H
#define ESP32_MQTT_H
#include <Arduino.h>

String getJwt();
void messageReceived(String &topic, String &payload);
void startMQTT();
void publishTelemetry(String data);
void publishState(String data);
void mqttConnect();
void checkConnect();
void setupCloudIoT();
void mqttdc();


#endif