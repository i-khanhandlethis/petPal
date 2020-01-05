<<<<<<< HEAD
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


=======
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


>>>>>>> 623652be972f6b7d398fd3a2db13eecae52e669e
#endif