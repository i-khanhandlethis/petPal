<<<<<<< HEAD
#include "filesystem.h"
#include <Arduino.h>
#include <stdlib.h>
#include <FirebaseESP32.h>
#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

FirebaseData firebaseData;

const char *ssid;
const char *password;
const char *timezone;
const char *ID;
const char *bootState;

char daysavetime = 1;

std::string dogNameRetriever()
{
  File configFile = SPIFFS.open("/config.json", "r");

  if (!configFile)
  {
    Serial.println("Failed to open config file");
  }

  size_t size = configFile.size();

  //Serial.println("Size");

  //Serial.println(size);
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

 // Serial.println(buf.get());
  configFile.close();
  StaticJsonDocument<1024> jsonDoc;

  auto error = deserializeJson(jsonDoc, buf.get());

  if (error)
  {
    Serial.println("Failed to parse config file");
    Serial.println(error.c_str());
  }

  String retrievedName = jsonDoc["name"];
  Serial.print("Retrieved name is:");
  Serial.println(retrievedName);
  return retrievedName.c_str();
}

bool setupWifi()
{

  File configFile = SPIFFS.open("/config.json", "r");

  if (!configFile)
  {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();

  Serial.print("Size:");

  Serial.println(size);
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  //Serial.println(buf.get());
  configFile.close();
  StaticJsonDocument<1024> jsonDoc;

  auto error = deserializeJson(jsonDoc, buf.get());

  if (error)
  {
    Serial.println("Failed to parse config file");
    Serial.println(error.c_str());
    return false;
  }

  ssid = jsonDoc["ssid"];
  password = jsonDoc["password"];
  timezone = jsonDoc["timezone"];
  ID = jsonDoc["id"];
  bootState = jsonDoc["bootState"];

  Serial.print("Loaded ssid:");
  Serial.println(ssid);
  Serial.print("Loaded password:");
  Serial.println(password);
  Serial.print("Loaded timezone:");
  Serial.println(timezone);
  Serial.print("Loaded ID:");
  Serial.println(ID);
  Serial.print("Woke up with bootState:");
  Serial.println(bootState);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 20)
  {
    delay(500);
    Serial.print(".");
    count++;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
    return false;
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Contacting Time Server");

  long tz = atol(timezone);

  configTime(3600 * tz, daysavetime * 3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");

  Serial.println("Done downloading time data");

  struct tm tmstruct;
  tmstruct.tm_year = 0;
  if (getLocalTime(&tmstruct, 10000) == true)
  {
    Serial.println("Timeset success");

    setTime(tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec, tmstruct.tm_mday, (tmstruct.tm_mon) + 1, (tmstruct.tm_year) + 1900);

    Serial.printf("Now is : %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
    Serial.println("");
    Serial.print(now());
    Serial.println("s");
  }
  else
  {
    Serial.println("Error with time sync. not syncing this time");
  }

  return true;
}

void setupWifiScanner()
{
}

void saveData(time_t t, int State, float Tag1, int Tag2)
{

  char fileNameString[100];

  sprintf(fileNameString, "/%d_%d_%d.txt", year(t), month(t), day(t));

  Serial.println(fileNameString);

  File saveFile = SPIFFS.open(fileNameString, "r");

  if (!saveFile)
  {
    Serial.println("Failed to open config file");
  }

  size_t size = saveFile.size();

  Serial.println("Size");
  Serial.println(size);

  if (size > 1024)
  {
    Serial.println("Config file size is too large");
  }

  saveFile.close();
}

void uploadData()
{
  //Set file (read file from Flash memory and set to database)
  char fileNameString[100];
  time_t t = now();
  sprintf(fileNameString, "/%s_%d_%d_%d.txt", ID, year(t), month(t), day(t));

  Serial.println(fileNameString);

  String path = "/ESP32_Test";

  if (Firebase.setFile(firebaseData, StorageType::SPIFFS, path + "/Binary/File/data", fileNameString))
  {
    Serial.println("PASSED");
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.fileTransferError());
    Serial.println("------------------------------------");
    Serial.println();
  }
}

void printRulesContent(FirebaseData &data)
{
  size_t tokenCount = data.jsonObject().parse(false).getJsonObjectIteratorCount();
  String key;
  String value;
  FirebaseJsonObject jsonParseResult;
  Serial.println();
  for (size_t i = 0; i < tokenCount; i++)
  {
    data.jsonObject().jsonObjectiterator(i, key, value);
    value.replace("\n", "");
    value.replace(" ", "");
    jsonParseResult = data.jsonObject().parseResult();
    Serial.print("KEY: ");
    Serial.print(key);
    Serial.print(", ");
    Serial.print("VALUE: ");
    Serial.print(value);
    Serial.print(", ");
    Serial.print("TYPE: ");
    Serial.println(jsonParseResult.type);
  }
}

bool WiFi_Off() {
WiFi.disconnect();
WiFi.mode(WIFI_OFF);

if (WiFi.status() != WL_CONNECTED)
return (true);
else
return (false);
=======
#include "filesystem.h"
#include <Arduino.h>
#include <stdlib.h>
#include <FirebaseESP32.h>
#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

FirebaseData firebaseData;

const char *ssid;
const char *password;
const char *timezone;
const char *ID;
const char *bootState;

char daysavetime = 1;

std::string dogNameRetriever()
{
  File configFile = SPIFFS.open("/config.json", "r");

  if (!configFile)
  {
    Serial.println("Failed to open config file");
  }

  size_t size = configFile.size();

  //Serial.println("Size");

  //Serial.println(size);
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

 // Serial.println(buf.get());
  configFile.close();
  StaticJsonDocument<1024> jsonDoc;

  auto error = deserializeJson(jsonDoc, buf.get());

  if (error)
  {
    Serial.println("Failed to parse config file");
    Serial.println(error.c_str());
  }

  String retrievedName = jsonDoc["name"];
  Serial.print("Retrieved name is:");
  Serial.println(retrievedName);
  return retrievedName.c_str();
}

bool setupWifi()
{

  File configFile = SPIFFS.open("/config.json", "r");

  if (!configFile)
  {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();

  Serial.print("Size:");

  Serial.println(size);
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  //Serial.println(buf.get());
  configFile.close();
  StaticJsonDocument<1024> jsonDoc;

  auto error = deserializeJson(jsonDoc, buf.get());

  if (error)
  {
    Serial.println("Failed to parse config file");
    Serial.println(error.c_str());
    return false;
  }

  ssid = jsonDoc["ssid"];
  password = jsonDoc["password"];
  timezone = jsonDoc["timezone"];
  ID = jsonDoc["id"];
  bootState = jsonDoc["bootState"];

  Serial.print("Loaded ssid:");
  Serial.println(ssid);
  Serial.print("Loaded password:");
  Serial.println(password);
  Serial.print("Loaded timezone:");
  Serial.println(timezone);
  Serial.print("Loaded ID:");
  Serial.println(ID);
  Serial.print("Woke up with bootState:");
  Serial.println(bootState);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 20)
  {
    delay(500);
    Serial.print(".");
    count++;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
    return false;
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Contacting Time Server");

  long tz = atol(timezone);

  configTime(3600 * tz, daysavetime * 3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");

  Serial.println("Done downloading time data");

  struct tm tmstruct;
  tmstruct.tm_year = 0;
  if (getLocalTime(&tmstruct, 10000) == true)
  {
    Serial.println("Timeset success");

    setTime(tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec, tmstruct.tm_mday, (tmstruct.tm_mon) + 1, (tmstruct.tm_year) + 1900);

    Serial.printf("Now is : %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
    Serial.println("");
    Serial.print(now());
    Serial.println("s");
  }
  else
  {
    Serial.println("Error with time sync. not syncing this time");
  }

  return true;
}

void setupWifiScanner()
{
}

void saveData(time_t t, int State, float Tag1, int Tag2)
{

  char fileNameString[100];

  sprintf(fileNameString, "/%d_%d_%d.txt", year(t), month(t), day(t));

  Serial.println(fileNameString);

  File saveFile = SPIFFS.open(fileNameString, "r");

  if (!saveFile)
  {
    Serial.println("Failed to open config file");
  }

  size_t size = saveFile.size();

  Serial.println("Size");
  Serial.println(size);

  if (size > 1024)
  {
    Serial.println("Config file size is too large");
  }

  saveFile.close();
}

void uploadData()
{
  //Set file (read file from Flash memory and set to database)
  char fileNameString[100];
  time_t t = now();
  sprintf(fileNameString, "/%s_%d_%d_%d.txt", ID, year(t), month(t), day(t));

  Serial.println(fileNameString);

  String path = "/ESP32_Test";

  if (Firebase.setFile(firebaseData, StorageType::SPIFFS, path + "/Binary/File/data", fileNameString))
  {
    Serial.println("PASSED");
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.fileTransferError());
    Serial.println("------------------------------------");
    Serial.println();
  }
}

void printRulesContent(FirebaseData &data)
{
  size_t tokenCount = data.jsonObject().parse(false).getJsonObjectIteratorCount();
  String key;
  String value;
  FirebaseJsonObject jsonParseResult;
  Serial.println();
  for (size_t i = 0; i < tokenCount; i++)
  {
    data.jsonObject().jsonObjectiterator(i, key, value);
    value.replace("\n", "");
    value.replace(" ", "");
    jsonParseResult = data.jsonObject().parseResult();
    Serial.print("KEY: ");
    Serial.print(key);
    Serial.print(", ");
    Serial.print("VALUE: ");
    Serial.print(value);
    Serial.print(", ");
    Serial.print("TYPE: ");
    Serial.println(jsonParseResult.type);
  }
}

bool WiFi_Off() {
WiFi.disconnect();
WiFi.mode(WIFI_OFF);

if (WiFi.status() != WL_CONNECTED)
return (true);
else
return (false);
>>>>>>> 623652be972f6b7d398fd3a2db13eecae52e669e
}