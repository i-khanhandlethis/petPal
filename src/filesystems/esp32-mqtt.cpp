#include "esp32-mqtt.h"
#include "ciotc_config.h"
#include "scheduler/rtcMemory.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <CloudIoTCore.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
// Initialize the Genuino WiFi SSL client library / RTC
WiFiClientSecure *netClient;
MQTTClient *mqttClient;

// Clout IoT configuration that you don't need to change
CloudIoTCoreDevice *device;
unsigned long iss = 0;
String jwt;

///////////////////////////////
// Helpers specific to this board
///////////////////////////////



String getJwt()
{
    if (iss == 0 || time(nullptr) - iss > 3600)
    { // TODO: exp in device
        iss = time(nullptr);
        Serial.println("Refreshing JWT");
        jwt = device->createJWT(iss);
    }
    else
    {
        Serial.println("Reusing still-valid JWT");
    }
    return jwt;
}

///////////////////////////////
// MQTT common functions
///////////////////////////////
void messageReceived(String &topic, String &payload)
{
    Serial.println("incoming: " + topic + " - " + payload);
}


void startMQTT()
{
    mqttClient->begin("mqtt.googleapis.com", 8883, *netClient);
    mqttClient->onMessage(messageReceived);
}

void publishTelemetry(String data)
{
    mqttClient->publish(device->getEventsTopic(), data);
}

// Helper that just sends default sensor
void publishState(String data)
{
    mqttClient->publish(device->getStateTopic(), data);
}

void mqttConnect()
{
    Serial.print("\nconnecting...");
    if (!mqttClient->connect(device->getClientId().c_str(), "unused", getJwt().c_str(), false))
    {
        Serial.println("Error connecting");
        Serial.println(mqttClient->lastError());
        Serial.println(mqttClient->returnCode());
        TIME_CLOUD = TIME_CLOUD - (10*60);
        return;
    }
    Serial.println("\nconnected!");
    mqttClient->subscribe(device->getConfigTopic());
    mqttClient->subscribe(device->getCommandsTopic());
    publishState("connected");


        File file = SPIFFS.open("/config.json");

    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, file);
    if (error)
      Serial.println(F("Failed to read file, using default configuration"));

    String sendID = doc["id"];
    String sendNAME = doc["name"];
    file.close();
    //String testsend = "{\"ID\":\""+sendID+"\",\"NAME\":\""+sendNAME+"\"}";
    //publishTelemetry(testsend);
    File file2 = SPIFFS.open("/data.txt");

    if (!file2)
    {
      Serial.println("Failed to open file for reading");
      return;
    }
    else Serial.println("Data.txt opened");
    int count = 0;
    while (file2.available())
    {
      String var = file2.readStringUntil('_');
      String datatosend = "{\"ID\":\""+sendID+"\",\"NAME\":\""+sendNAME+"\","+var;

      count++;
      publishTelemetry(datatosend);
      Serial.println(datatosend);
    }
    Serial.print(count);
    Serial.println(" lines sent.");
    file2.close();
    Serial.println("Data.txt closed");

    SPIFFS.remove("/data.txt");
    mqttdc();
}

///////////////////////////////
// Orchestrates various methods from preceeding code.
///////////////////////////////
void connect()
{
    Serial.print("checking wifi...");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }
    mqttConnect();
}

void checkConnect()
{
    mqttClient->loop();
    delay(10); // <- fixes some issues with WiFi stability

    if (!mqttClient->connected())
    {
        connect();
    }
}

void setupCloudIoT()
{
    device = new CloudIoTCoreDevice(
        project_id, location, registry_id, device_id,
        private_key_str);

    netClient = new WiFiClientSecure();
    mqttClient = new MQTTClient(20000);
    startMQTT();
}

void mqttdc()
{
    
    mqttClient->disconnect();
}