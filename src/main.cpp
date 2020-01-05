<<<<<<< HEAD
#include <stdio.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <EEPROM.h>
#include <FS.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <TimeLib.h>
#include <Update.h>
#include <WiFi.h>
#include <Wire.h>
#include <WebServer.h>
#define VARIANT "esp32"

#include "scheduler/scheduler.h"
#include "scheduler/rtcMemory.h"
#include "filesystems/filesystem.h"
#include "filesystems/esp32-mqtt.h"
#include "movementalgorithims/MovementCycle.h"
//fillerrr
//Pins declaration
const byte int1Pin = 21; //Use pin 2 for int.0 on uno
//const byte int2Pin = 22; //Use pin 2 for int.0 on uno
RTC_DATA_ATTR std::string DOGNAME = "PetPal";
RTC_DATA_ATTR int rtc_steps = 0;

int TIME_TO_SLEEP = 30;
int wakeUpState = 0;

bool deviceConnected = false;
int actionMode = 0;
#define CURRENT_VERSION VERSION
#define CLOUD_FUNCTION_URL "http://us-central1-petpal-247009.cloudfunctions.net/getDownloadUrl"

#define USE_SERIAL Serial

WiFiClient client;
WebServer server(80);

BLECharacteristic *pCharacteristic;
BLECharacteristic *DISTANCECharacteristic;
BLECharacteristic *MOVEMENTCharacteristic;
BLECharacteristic *TIMESTAMPCharacteristic;
BLECharacteristic *IDCharacteristic;

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID

#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_HB_FILTERED "6E400006-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_DISTANCE "0000FFE1-0000-1000-8000-008024DCCA9E"
#define CHARACTERISTIC_UUID_MOVEMENT "6E400008-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TIMESTAMP "6E400009-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_ID "6E40000A-B5A3-F393-E0A9-E50E24DCCA9E"

const char *filename = "/config.json"; // <- SD library uses 8.3 filenames

char *recievedValue;
String recievedValueString = "";

bool STATE_POLLING = false;
//RTC_DATA_ATTR bool STATE_DATAAQ = false;
bool STATE_CLOUD = false;
bool STATE_BLE = false;

void setupBLE();
class MyServerCallbacks : public BLEServerCallbacks
{
public:
  //BLE Server class
  void onConnect(BLEServer *pServer)
  {
    Serial.println("BT connected;");
    deviceConnected = true;
  }

  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("BT disconnected;");
    deviceConnected = false;
  }
};
TaskHandle_t Task2;

char *Val1;
char *Val2;
char *Val3;
int BatteryVoltage = 0;
void battVoltage(){
  BatteryVoltage = analogRead(25);
}
void parseBLEInput()
{
  if (recievedValueString.length() != 0)
  {
    Val1 = strtok(recievedValue, ",");
    Serial.println(Val1);

    if (strcmp(Val1, "L") == 0)
    {
      Serial.print("Config detected. Parsing configuration now");
      actionMode = 0;

      Val1 = strtok(NULL, ",");
      String nameString(Val1);
      Serial.print("NameString: ");
      Serial.println(nameString);

      Val1 = strtok(NULL, ",");
      String useridString(Val1);
      Serial.print("UserIDString: ");
      Serial.println(useridString);

      Val1 = strtok(NULL, ",");
      String tzString(Val1);
      Serial.print("tzString: ");
      Serial.println(tzString);

      Val1 = strtok(NULL, ",");
      String ssidString(Val1);
      Serial.print("ssidString: ");
      Serial.println(ssidString);

      Val1 = strtok(NULL, ",");
      String pwString(Val1);
      Serial.print("pwString: ");
      Serial.println(pwString);

      SPIFFS.remove(filename);

      File configFile = SPIFFS.open("/config.json", "w");

      if (!configFile)
      {
        Serial.println("Failed to open config file for writing");
      }

      StaticJsonDocument<2000> doc;

      doc["name"] = nameString;
      doc["id"] = useridString;
      doc["timezone"] = tzString;
      doc["ssid"] = ssidString;
      doc["password"] = pwString;
      DOGNAME = nameString.c_str();

      if (serializeJson(doc, configFile) == 0)
      {
        Serial.println(F("Failed to write file"));
      }

      configFile.close();

      if (setupWifi() == true)
      {
        Serial.print("wifi connect Success!");

        DISTANCECharacteristic->setValue("success");
        DISTANCECharacteristic->notify();
        delay(1000);
      }
      else
      {
        Serial.print("Connection Failure. Try another wifi");

        DISTANCECharacteristic->setValue("failure");
        DISTANCECharacteristic->notify();
      }

    }
    if (strcmp(Val1, "S") == 0)
    {
      Serial.print("Recieved poll command. Commencing Poll");
      actionMode = 1;
    }
    if (strcmp(Val1, "w") == 0)
    {
      devState = 1;
      rtc_steps = 0;
      recievedValueString = "";
      recievedValue = const_cast<char *>(recievedValueString.c_str());
      deepSleepProtocols();
    }
    if(strcmp(Val1, "E") == 0){
      //btStop();
      deepSleepProtocols();
    }
    recievedValueString = "";
    recievedValue = const_cast<char *>(recievedValueString.c_str());
  }
}

//BLE Functions
class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      Serial.println("*********");
      //recievedValue = "";
      for (int i = 0; i < rxValue.length(); i++)
      {
        //  Serial.print(rxValue[i]);
        recievedValueString = recievedValueString + rxValue[i];
      }
      Serial.print("Received String: ");
      Serial.print(recievedValueString);
      recievedValue = const_cast<char *>(recievedValueString.c_str());
      Serial.print(" > > > Converted to Char* : ");
      Serial.println(recievedValue);
      Serial.println();
      Serial.println("*********");
    }
  }
};

/* 
 * Check if needs to update the device and returns the download url.
 */
String getDownloadUrl()
{
  HTTPClient http;
  String downloadUrl;
  USE_SERIAL.print("[HTTP] begin...\n");

  String url = CLOUD_FUNCTION_URL;
  url += String("?version=") + CURRENT_VERSION;
  url += String("&variant=") + VARIANT;
  http.begin(url);

  USE_SERIAL.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      USE_SERIAL.println(payload);
      downloadUrl = payload;
    }
    else
    {
      USE_SERIAL.println("Device is up to date!");
    }
  }
  else
  {
    USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  return downloadUrl;
}

/* 
 * Download binary image and use Update library to update the device.
 */
bool downloadUpdate(String url)
{
  HTTPClient http;
  USE_SERIAL.print("[HTTP] Download begin...\n");

  http.begin(url);

  USE_SERIAL.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {

      size_t contentLength = http.getSize();
      USE_SERIAL.println("contentLength : " + String(contentLength));

      if (contentLength > 0)
      {
        bool canBegin = Update.begin(contentLength);
        if (canBegin)
        {
          WiFiClient stream = http.getStream();
          USE_SERIAL.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quiet for a while.. Patience!");
          size_t written = Update.writeStream(stream);

          if (written == contentLength)
          {
            USE_SERIAL.println("Written : " + String(written) + " successfully");
          }
          else
          {
            USE_SERIAL.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
          }

          if (Update.end())
          {
            USE_SERIAL.println("OTA done!");
            if (Update.isFinished())
            {
              USE_SERIAL.println("Update successfully completed. Rebooting.");
              ESP.restart();
              return true;
            }
            else
            {
              USE_SERIAL.println("Update not finished? Something went wrong!");
              return false;
            }
          }
          else
          {
            USE_SERIAL.println("Error Occurred. Error #: " + String(Update.getError()));
            return false;
          }
        }
        else
        {
          USE_SERIAL.println("Not enough space to begin OTA");
          client.flush();
          return false;
        }
      }
      else
      {
        USE_SERIAL.println("There was no content in the response");
        client.flush();
        return false;
      }
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

/* 
 * Show current device version
 */
void handleRoot()
{
  server.send(200, "text/plain", "v" + String(CURRENT_VERSION));
}
unsigned long starts = millis();

void coreTaskTwo(void *pvParameters)
{
}

void setup()
{
  //starts = millis();
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  pinMode(26, OUTPUT);
  wakeUpProtocols();
  battVoltage();
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system.\n RESTART device");
    deepSleepProtocols();
  }

  if (bootCount == 0)
  {
    setupXLGYRO();
    setTime(0, 0, 0, 1, 1, 2019);
    
    TIME_STEPS = now();
    TIME_DATAAQ = now();
    TIME_CLOUD = now();
    if (setupWifi() == true)
    {
      setupCloudIoT();
      checkConnect();
    }
                    if(WiFi_Off() == true)
        Serial.println("WiFi Turned Off Completely.");
      else
      Serial.println("WiFi did not turn off.");
    DOGNAME = dogNameRetriever();
    deepSleepProtocols();
  }
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0)
  {
    if (devState == 0)
    {
      DOGNAME = dogNameRetriever();
      Serial.println("Button pressed!");
      BLEDevice::init(DOGNAME); //Give it a name
      //esp_bt_controller_enable(ESP_BT_MODE_BLE);
      BLEServer *pServer = BLEDevice::createServer();              //Create the BLE Server
      pServer->setCallbacks(new MyServerCallbacks());              // Create the BLE Service
      BLEService *pService = pServer->createService(SERVICE_UUID); // Create a BLE Service

      DISTANCECharacteristic = pService->createCharacteristic(
          CHARACTERISTIC_UUID_DISTANCE,
          BLECharacteristic::PROPERTY_READ |
              BLECharacteristic::PROPERTY_WRITE |
              BLECharacteristic::PROPERTY_NOTIFY);
      DISTANCECharacteristic->addDescriptor(new BLE2902());

      BLECharacteristic *pCharacteristic = pService->createCharacteristic(
          CHARACTERISTIC_UUID_RX,
          BLECharacteristic::PROPERTY_WRITE);

      pCharacteristic->setCallbacks(new MyCallbacks());

      Serial.println("Waiting a client connection to notify...");

      // Start the service
      pService->start();

      // Start advertising
      pServer->getAdvertising()->start();

      //esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);

      WiFi.mode(WIFI_STA);
      //WiFi.disconnect();

      double BLEstartscan = now();
      int BLEwaitSeconds = 0;
      while (!deviceConnected && BLEwaitSeconds != 25)
      {
        delay(700);
        digitalWrite(26, LOW);
        BLEwaitSeconds = now() - BLEstartscan;
        Serial.print("(Connecting)been waiting for:");
        Serial.print(BLEwaitSeconds);
        Serial.println("s");
        delay(300);
        digitalWrite(26, HIGH);
      }
      if (deviceConnected)
        Serial.println("Device connected!");
      else
        deepSleepProtocols();

      BLEstartscan = now();
      BLEwaitSeconds = 0;
      int delaytime = 25;
      while (deviceConnected && BLEwaitSeconds < delaytime)
      {
        parseBLEInput();
        if (actionMode == 1)
        {
          Serial.println("Device is connected. Starting WiFi Scan");
          int n = WiFi.scanNetworks();
          String forprinting;

          Serial.println("scan done");
          if (n == 0)
          {
            Serial.println("no networks found");
          }
          else
          {
            Serial.print(n);
            Serial.println(" networks found");
            for (int i = 0; i < n; ++i)
            {
              //Serial.println(WiFi.SSID(i));
              //delay(10);
              if (i > 0)
                forprinting = forprinting + "," + WiFi.SSID(i);
              else
                forprinting = WiFi.SSID(i);
            }
            Serial.println(forprinting);

            delaytime = 60;
            BLEwaitSeconds = 0;

            Serial.println("waiting for instructions for 1 min");
          }
          std::string sendval(forprinting.c_str());

          DISTANCECharacteristic->setValue(sendval);
          DISTANCECharacteristic->notify();
          //delay(1000);
          actionMode = 0;
        }
        digitalWrite(26, HIGH);
        BLEwaitSeconds = now() - BLEstartscan;
        Serial.print("(parsing)been waiting for:");
        Serial.print(BLEwaitSeconds);
        Serial.println("s");
        delay(200);
        digitalWrite(26, LOW);
        delay(200);
      }
    }
    if (devState == 1)
    {
      Serial.println("Disconnecting and resuming normal");
      devState = 0;
      lastDevStateUpdate = 0;
    }
    deepSleepProtocols();
  }
  else if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.print("Timer triggered | ");
    //Commence natural boot
    if (devState == 0)
    {
      Serial.print("Dev state is 0 | ");

      switch (workSchedule())
      {
      case 0:
        Serial.println("No tasks needed to be done. gonna sleep yoooo");
        break;
      case 1:
        Serial.println("Clouding yo");
        TIME_CLOUD = now();
        if (setupWifi() == true)
        {

          Serial.print("wifi connect Success!");
          String version = String("<p>Current Version - v") + String(CURRENT_VERSION) + String("</p>");
          USE_SERIAL.println(version);

          // Check if we need to download a new version
          String downloadUrl = getDownloadUrl();
          if (downloadUrl.length() > 0)
          {
            bool success = downloadUpdate(downloadUrl);
            if (!success)
            {
              USE_SERIAL.println("Error updating device");
            }
          }

          server.on("/", handleRoot);
          server.begin();
          USE_SERIAL.println("HTTP server started");

          USE_SERIAL.print("IP address: ");
          USE_SERIAL.println(WiFi.localIP());

          setupCloudIoT();
          checkConnect();
        }
        else
        {
          Serial.println("WiFi Failure");
        }
        if (WiFi_Off() == true)
          Serial.println("WiFi Turned Off Completely.");
        else
          Serial.println("WiFi did not turn off.");
        break;
      case 2:
        Serial.println("Accelerate to space");
        TIME_DATAAQ = now();
        //rawXLGPoll(TIME_DATAAQ);
        break;
      case 3:
        Serial.println("Step jer");
        TIME_STEPS = now();

        if (checkPedometer() > 0)
        {
          Serial.println("steps taken!");
        }
        else
        {
          Serial.println("No steps taken.");
        }
        break;
      default:
        Serial.print("wtf is wrong with this ");
        break;
      }

      deepSleepProtocols();
    }

    //Commence walker state
    else if (devState == 1)
    {

      Serial.println("TimerBLEBoot!");
      BLEDevice::init(DOGNAME); //Give it a name
      //esp_bt_controller_enable(ESP_BT_MODE_BLE);
      BLEServer *pServer = BLEDevice::createServer();              //Create the BLE Server
      pServer->setCallbacks(new MyServerCallbacks());              // Create the BLE Service
      BLEService *pService = pServer->createService(SERVICE_UUID); // Create a BLE Service

      DISTANCECharacteristic = pService->createCharacteristic(
          CHARACTERISTIC_UUID_DISTANCE,
          BLECharacteristic::PROPERTY_READ |
              BLECharacteristic::PROPERTY_WRITE |
              BLECharacteristic::PROPERTY_NOTIFY);
      DISTANCECharacteristic->addDescriptor(new BLE2902());

      BLECharacteristic *pCharacteristic = pService->createCharacteristic(
          CHARACTERISTIC_UUID_RX,
          BLECharacteristic::PROPERTY_WRITE);

      pCharacteristic->setCallbacks(new MyCallbacks());

      Serial.println("Waiting a client connection to notify...");

      // Start the service
      pService->start();
      
      // Start advertising
      pServer->getAdvertising()->start();

      int walkerSteps = checkPedometer();

      time_t waitTime = now();
      int waitTimeSeconds = 0;

        lastDevStateUpdate = 0;
        String forprinting;
        forprinting = walkerSteps+rtc_steps;
        std::string sendval(forprinting.c_str());

      while (!deviceConnected && waitTimeSeconds < 20)
      {
        waitTimeSeconds = now() - waitTime;
        delay(1000);
      }
      if (deviceConnected)
      {
        DISTANCECharacteristic->setValue(sendval);
        DISTANCECharacteristic->notify();
        Serial.println("Sent Value");
        while (waitTimeSeconds < 30)
        {
          parseBLEInput();
          waitTimeSeconds = now() - waitTime;
          delay(300);
        }
      }
      else
      {
        lastDevStateUpdate++;
        Serial.print("Not been connected since: ");
        Serial.println(lastDevStateUpdate);
        if (lastDevStateUpdate > 30)
        {
          Serial.println("Not connected for too long. Going back to normal routine");
          devState = 0;
          lastDevStateUpdate = 0;
        }
      }

      deepSleepProtocols();
    }
  }

  /*
  xTaskCreatePinnedToCore(
      coreTaskTwo,
      "coreTaskTwo",
      5000,
      NULL,
      1,
      &Task2,
      0);
      */
}

int time_change = 0;

void loop()
{
  deepSleepProtocols();
=======
#include <stdio.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <EEPROM.h>
#include <FS.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <TimeLib.h>
#include <Update.h>
#include <WiFi.h>
#include <Wire.h>
#include <WebServer.h>
#define VARIANT "esp32"

#include "scheduler/scheduler.h"
#include "scheduler/rtcMemory.h"
#include "filesystems/filesystem.h"
#include "filesystems/esp32-mqtt.h"
#include "movementalgorithims/MovementCycle.h"

//Pins declaration
const byte int1Pin = 21; //Use pin 2 for int.0 on uno
//const byte int2Pin = 22; //Use pin 2 for int.0 on uno
RTC_DATA_ATTR std::string DOGNAME = "PetPal";
RTC_DATA_ATTR int rtc_steps = 0;

int TIME_TO_SLEEP = 30;
int wakeUpState = 0;

bool deviceConnected = false;
int actionMode = 0;
#define CURRENT_VERSION VERSION
#define CLOUD_FUNCTION_URL "http://us-central1-petpal-247009.cloudfunctions.net/getDownloadUrl"

#define USE_SERIAL Serial

WiFiClient client;
WebServer server(80);

BLECharacteristic *pCharacteristic;
BLECharacteristic *DISTANCECharacteristic;
BLECharacteristic *MOVEMENTCharacteristic;
BLECharacteristic *TIMESTAMPCharacteristic;
BLECharacteristic *IDCharacteristic;

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID

#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_HB_FILTERED "6E400006-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_DISTANCE "0000FFE1-0000-1000-8000-008024DCCA9E"
#define CHARACTERISTIC_UUID_MOVEMENT "6E400008-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TIMESTAMP "6E400009-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_ID "6E40000A-B5A3-F393-E0A9-E50E24DCCA9E"

const char *filename = "/config.json"; // <- SD library uses 8.3 filenames

char *recievedValue;
String recievedValueString = "";

bool STATE_POLLING = false;
//RTC_DATA_ATTR bool STATE_DATAAQ = false;
bool STATE_CLOUD = false;
bool STATE_BLE = false;

void setupBLE();
class MyServerCallbacks : public BLEServerCallbacks
{
public:
  //BLE Server class
  void onConnect(BLEServer *pServer)
  {
    Serial.println("BT connected;");
    deviceConnected = true;
  }

  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("BT disconnected;");
    deviceConnected = false;
  }
};
TaskHandle_t Task2;

char *Val1;
char *Val2;
char *Val3;
int BatteryVoltage = 0;
void battVoltage(){
  BatteryVoltage = analogRead(25);
}
void parseBLEInput()
{
  if (recievedValueString.length() != 0)
  {
    Val1 = strtok(recievedValue, ",");
    Serial.println(Val1);

    if (strcmp(Val1, "L") == 0)
    {
      Serial.print("Config detected. Parsing configuration now");
      actionMode = 0;

      Val1 = strtok(NULL, ",");
      String nameString(Val1);
      Serial.print("NameString: ");
      Serial.println(nameString);

      Val1 = strtok(NULL, ",");
      String useridString(Val1);
      Serial.print("UserIDString: ");
      Serial.println(useridString);

      Val1 = strtok(NULL, ",");
      String tzString(Val1);
      Serial.print("tzString: ");
      Serial.println(tzString);

      Val1 = strtok(NULL, ",");
      String ssidString(Val1);
      Serial.print("ssidString: ");
      Serial.println(ssidString);

      Val1 = strtok(NULL, ",");
      String pwString(Val1);
      Serial.print("pwString: ");
      Serial.println(pwString);

      SPIFFS.remove(filename);

      File configFile = SPIFFS.open("/config.json", "w");

      if (!configFile)
      {
        Serial.println("Failed to open config file for writing");
      }

      StaticJsonDocument<2000> doc;

      doc["name"] = nameString;
      doc["id"] = useridString;
      doc["timezone"] = tzString;
      doc["ssid"] = ssidString;
      doc["password"] = pwString;
      DOGNAME = nameString.c_str();

      if (serializeJson(doc, configFile) == 0)
      {
        Serial.println(F("Failed to write file"));
      }

      configFile.close();

      if (setupWifi() == true)
      {
        Serial.print("wifi connect Success!");

        DISTANCECharacteristic->setValue("success");
        DISTANCECharacteristic->notify();
        delay(1000);
      }
      else
      {
        Serial.print("Connection Failure. Try another wifi");

        DISTANCECharacteristic->setValue("failure");
        DISTANCECharacteristic->notify();
      }

    }
    if (strcmp(Val1, "S") == 0)
    {
      Serial.print("Recieved poll command. Commencing Poll");
      actionMode = 1;
    }
    if (strcmp(Val1, "w") == 0)
    {
      devState = 1;
      rtc_steps = 0;
      recievedValueString = "";
      recievedValue = const_cast<char *>(recievedValueString.c_str());
      deepSleepProtocols();
    }
    if(strcmp(Val1, "E") == 0){
      //btStop();
      deepSleepProtocols();
    }
    recievedValueString = "";
    recievedValue = const_cast<char *>(recievedValueString.c_str());
  }
}

//BLE Functions
class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      Serial.println("*********");
      //recievedValue = "";
      for (int i = 0; i < rxValue.length(); i++)
      {
        //  Serial.print(rxValue[i]);
        recievedValueString = recievedValueString + rxValue[i];
      }
      Serial.print("Received String: ");
      Serial.print(recievedValueString);
      recievedValue = const_cast<char *>(recievedValueString.c_str());
      Serial.print(" > > > Converted to Char* : ");
      Serial.println(recievedValue);
      Serial.println();
      Serial.println("*********");
    }
  }
};

/* 
 * Check if needs to update the device and returns the download url.
 */
String getDownloadUrl()
{
  HTTPClient http;
  String downloadUrl;
  USE_SERIAL.print("[HTTP] begin...\n");

  String url = CLOUD_FUNCTION_URL;
  url += String("?version=") + CURRENT_VERSION;
  url += String("&variant=") + VARIANT;
  http.begin(url);

  USE_SERIAL.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      USE_SERIAL.println(payload);
      downloadUrl = payload;
    }
    else
    {
      USE_SERIAL.println("Device is up to date!");
    }
  }
  else
  {
    USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  return downloadUrl;
}

/* 
 * Download binary image and use Update library to update the device.
 */
bool downloadUpdate(String url)
{
  HTTPClient http;
  USE_SERIAL.print("[HTTP] Download begin...\n");

  http.begin(url);

  USE_SERIAL.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {

      size_t contentLength = http.getSize();
      USE_SERIAL.println("contentLength : " + String(contentLength));

      if (contentLength > 0)
      {
        bool canBegin = Update.begin(contentLength);
        if (canBegin)
        {
          WiFiClient stream = http.getStream();
          USE_SERIAL.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quiet for a while.. Patience!");
          size_t written = Update.writeStream(stream);

          if (written == contentLength)
          {
            USE_SERIAL.println("Written : " + String(written) + " successfully");
          }
          else
          {
            USE_SERIAL.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
          }

          if (Update.end())
          {
            USE_SERIAL.println("OTA done!");
            if (Update.isFinished())
            {
              USE_SERIAL.println("Update successfully completed. Rebooting.");
              ESP.restart();
              return true;
            }
            else
            {
              USE_SERIAL.println("Update not finished? Something went wrong!");
              return false;
            }
          }
          else
          {
            USE_SERIAL.println("Error Occurred. Error #: " + String(Update.getError()));
            return false;
          }
        }
        else
        {
          USE_SERIAL.println("Not enough space to begin OTA");
          client.flush();
          return false;
        }
      }
      else
      {
        USE_SERIAL.println("There was no content in the response");
        client.flush();
        return false;
      }
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

/* 
 * Show current device version
 */
void handleRoot()
{
  server.send(200, "text/plain", "v" + String(CURRENT_VERSION));
}
unsigned long starts = millis();

void coreTaskTwo(void *pvParameters)
{
}

void setup()
{
  //starts = millis();
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  pinMode(26, OUTPUT);
  wakeUpProtocols();
  battVoltage();
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system.\n RESTART device");
    deepSleepProtocols();
  }

  if (bootCount == 0)
  {
    setupXLGYRO();
    setTime(0, 0, 0, 1, 1, 2019);
    
    TIME_STEPS = now();
    TIME_DATAAQ = now();
    TIME_CLOUD = now();
    if (setupWifi() == true)
    {
      setupCloudIoT();
      checkConnect();
    }
                    if(WiFi_Off() == true)
        Serial.println("WiFi Turned Off Completely.");
      else
      Serial.println("WiFi did not turn off.");
    DOGNAME = dogNameRetriever();
    deepSleepProtocols();
  }
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0)
  {
    if (devState == 0)
    {
      DOGNAME = dogNameRetriever();
      Serial.println("Button pressed!");
      BLEDevice::init(DOGNAME); //Give it a name
      //esp_bt_controller_enable(ESP_BT_MODE_BLE);
      BLEServer *pServer = BLEDevice::createServer();              //Create the BLE Server
      pServer->setCallbacks(new MyServerCallbacks());              // Create the BLE Service
      BLEService *pService = pServer->createService(SERVICE_UUID); // Create a BLE Service

      DISTANCECharacteristic = pService->createCharacteristic(
          CHARACTERISTIC_UUID_DISTANCE,
          BLECharacteristic::PROPERTY_READ |
              BLECharacteristic::PROPERTY_WRITE |
              BLECharacteristic::PROPERTY_NOTIFY);
      DISTANCECharacteristic->addDescriptor(new BLE2902());

      BLECharacteristic *pCharacteristic = pService->createCharacteristic(
          CHARACTERISTIC_UUID_RX,
          BLECharacteristic::PROPERTY_WRITE);

      pCharacteristic->setCallbacks(new MyCallbacks());

      Serial.println("Waiting a client connection to notify...");

      // Start the service
      pService->start();

      // Start advertising
      pServer->getAdvertising()->start();

      //esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);

      WiFi.mode(WIFI_STA);
      //WiFi.disconnect();

      double BLEstartscan = now();
      int BLEwaitSeconds = 0;
      while (!deviceConnected && BLEwaitSeconds != 25)
      {
        delay(700);
        digitalWrite(26, LOW);
        BLEwaitSeconds = now() - BLEstartscan;
        Serial.print("(Connecting)been waiting for:");
        Serial.print(BLEwaitSeconds);
        Serial.println("s");
        delay(300);
        digitalWrite(26, HIGH);
      }
      if (deviceConnected)
        Serial.println("Device connected!");
      else
        deepSleepProtocols();

      BLEstartscan = now();
      BLEwaitSeconds = 0;
      int delaytime = 25;
      while (deviceConnected && BLEwaitSeconds < delaytime)
      {
        parseBLEInput();
        if (actionMode == 1)
        {
          Serial.println("Device is connected. Starting WiFi Scan");
          int n = WiFi.scanNetworks();
          String forprinting;

          Serial.println("scan done");
          if (n == 0)
          {
            Serial.println("no networks found");
          }
          else
          {
            Serial.print(n);
            Serial.println(" networks found");
            for (int i = 0; i < n; ++i)
            {
              //Serial.println(WiFi.SSID(i));
              //delay(10);
              if (i > 0)
                forprinting = forprinting + "," + WiFi.SSID(i);
              else
                forprinting = WiFi.SSID(i);
            }
            Serial.println(forprinting);

            delaytime = 60;
            BLEwaitSeconds = 0;

            Serial.println("waiting for instructions for 1 min");
          }
          std::string sendval(forprinting.c_str());

          DISTANCECharacteristic->setValue(sendval);
          DISTANCECharacteristic->notify();
          //delay(1000);
          actionMode = 0;
        }
        digitalWrite(26, HIGH);
        BLEwaitSeconds = now() - BLEstartscan;
        Serial.print("(parsing)been waiting for:");
        Serial.print(BLEwaitSeconds);
        Serial.println("s");
        delay(200);
        digitalWrite(26, LOW);
        delay(200);
      }
    }
    if (devState == 1)
    {
      Serial.println("Disconnecting and resuming normal");
      devState = 0;
      lastDevStateUpdate = 0;
    }
    deepSleepProtocols();
  }
  else if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.print("Timer triggered | ");
    //Commence natural boot
    if (devState == 0)
    {
      Serial.print("Dev state is 0 | ");

      switch (workSchedule())
      {
      case 0:
        Serial.println("No tasks needed to be done. gonna sleep yoooo");
        break;
      case 1:
        Serial.println("Clouding yo");
        TIME_CLOUD = now();
        if (setupWifi() == true)
        {

          Serial.print("wifi connect Success!");
          String version = String("<p>Current Version - v") + String(CURRENT_VERSION) + String("</p>");
          USE_SERIAL.println(version);

          // Check if we need to download a new version
          String downloadUrl = getDownloadUrl();
          if (downloadUrl.length() > 0)
          {
            bool success = downloadUpdate(downloadUrl);
            if (!success)
            {
              USE_SERIAL.println("Error updating device");
            }
          }

          server.on("/", handleRoot);
          server.begin();
          USE_SERIAL.println("HTTP server started");

          USE_SERIAL.print("IP address: ");
          USE_SERIAL.println(WiFi.localIP());

          setupCloudIoT();
          checkConnect();
        }
        else
        {
          Serial.println("WiFi Failure");
        }
        if (WiFi_Off() == true)
          Serial.println("WiFi Turned Off Completely.");
        else
          Serial.println("WiFi did not turn off.");
        break;
      case 2:
        Serial.println("Accelerate to space");
        TIME_DATAAQ = now();
        //rawXLGPoll(TIME_DATAAQ);
        break;
      case 3:
        Serial.println("Step jer");
        TIME_STEPS = now();

        if (checkPedometer() > 0)
        {
          Serial.println("steps taken!");
        }
        else
        {
          Serial.println("No steps taken.");
        }
        break;
      default:
        Serial.print("wtf is wrong with this ");
        break;
      }

      deepSleepProtocols();
    }

    //Commence walker state
    else if (devState == 1)
    {

      Serial.println("TimerBLEBoot!");
      BLEDevice::init(DOGNAME); //Give it a name
      //esp_bt_controller_enable(ESP_BT_MODE_BLE);
      BLEServer *pServer = BLEDevice::createServer();              //Create the BLE Server
      pServer->setCallbacks(new MyServerCallbacks());              // Create the BLE Service
      BLEService *pService = pServer->createService(SERVICE_UUID); // Create a BLE Service

      DISTANCECharacteristic = pService->createCharacteristic(
          CHARACTERISTIC_UUID_DISTANCE,
          BLECharacteristic::PROPERTY_READ |
              BLECharacteristic::PROPERTY_WRITE |
              BLECharacteristic::PROPERTY_NOTIFY);
      DISTANCECharacteristic->addDescriptor(new BLE2902());

      BLECharacteristic *pCharacteristic = pService->createCharacteristic(
          CHARACTERISTIC_UUID_RX,
          BLECharacteristic::PROPERTY_WRITE);

      pCharacteristic->setCallbacks(new MyCallbacks());

      Serial.println("Waiting a client connection to notify...");

      // Start the service
      pService->start();
      
      // Start advertising
      pServer->getAdvertising()->start();

      int walkerSteps = checkPedometer();

      time_t waitTime = now();
      int waitTimeSeconds = 0;

        lastDevStateUpdate = 0;
        String forprinting;
        forprinting = walkerSteps+rtc_steps;
        std::string sendval(forprinting.c_str());

      while (!deviceConnected && waitTimeSeconds < 20)
      {
        waitTimeSeconds = now() - waitTime;
        delay(1000);
      }
      if (deviceConnected)
      {
        DISTANCECharacteristic->setValue(sendval);
        DISTANCECharacteristic->notify();
        Serial.println("Sent Value");
        while (waitTimeSeconds < 30)
        {
          parseBLEInput();
          waitTimeSeconds = now() - waitTime;
          delay(300);
        }
      }
      else
      {
        lastDevStateUpdate++;
        Serial.print("Not been connected since: ");
        Serial.println(lastDevStateUpdate);
        if (lastDevStateUpdate > 30)
        {
          Serial.println("Not connected for too long. Going back to normal routine");
          devState = 0;
          lastDevStateUpdate = 0;
        }
      }

      deepSleepProtocols();
    }
  }

  /*
  xTaskCreatePinnedToCore(
      coreTaskTwo,
      "coreTaskTwo",
      5000,
      NULL,
      1,
      &Task2,
      0);
      */
}

int time_change = 0;

void loop()
{
  deepSleepProtocols();
>>>>>>> 623652be972f6b7d398fd3a2db13eecae52e669e
}