#include <Arduino.h>
#include <HPDL1414.h>
#include <WiFi.h>
#include "time.h"

const char* ssid     = "Xiaomi_ANNA";
const char* password = "23263483";

const char* ntpServer  = "pool.ntp.org";
const char* timeZone   = "EET-2EEST,M3.5.0/3,M10.5.0/4"; 

const byte dataPins[7] = {13, 12, 14, 27, 5, 18, 19}; // D0, D1, D2, D3, D4, D5, D6
const byte addrPins[2] = {21, 22};                   // A0, A1
const byte wrenPins[]  = {23};                       // !WR

HPDL1414 hpdl(dataPins, addrPins, wrenPins, sizeof(wrenPins));

void scrollText(const char* text, int delayTime) {
  int textLen = strlen(text);
  
  if (textLen <= 4) {
    hpdl.clear();
    hpdl.print(text);
    return;
  }

  char displayBuf[5];
  
  for (int i = 0; i <= textLen - 4; i++) {
    strncpy(displayBuf, text + i, 4);
    displayBuf[4] = '\0';
    
    hpdl.clear();
    hpdl.print(displayBuf);
    delay(delayTime);
  }
}

void setup() {
  Serial.begin(115200);
  hpdl.begin();
  hpdl.clear();

  hpdl.print("WIFI");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi підключено!");

  configTzTime(timeZone, ntpServer);
  
  hpdl.clear();
  hpdl.print("SYNC");
  
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Очікування синхронізації часу...");
    delay(500);
  }
  Serial.println("Час успішно отримано!");
  hpdl.clear();
}

void loop() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Не вдалося отримати час");
    return;
  }

  char timeStr[5];
  strftime(timeStr, sizeof(timeStr), "%H%M", &timeinfo);
  
  hpdl.clear();
  hpdl.print(timeStr);
  delay(5000); 
  char scrollBuffer[50];

  strftime(scrollBuffer, sizeof(scrollBuffer), "   %d.%m.%Y - %A   ", &timeinfo);

  scrollText(scrollBuffer, 300);
  
  delay(500); 
}