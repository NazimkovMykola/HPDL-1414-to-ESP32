#include <Arduino.h>
#include <HPDL1414.h>
#include <WiFi.h>
#include "time.h"

// ---------- Налаштування ----------
const char* ssid     = "Xiaomi_ANNA";
const char* password = "23263483";
const char* ntpServer  = "pool.ntp.org";
const char* timeZone   = "EET-2EEST,M3.5.0/3,M10.5.0/4";

const byte dataPins[7] = {13, 12, 14, 27, 5, 18, 19}; // D0..D6
const byte addrPins[2] = {21, 22};                    // A0, A1
const byte wrenPins[]  = {23};                        // !WR
HPDL1414 hpdl(dataPins, addrPins, wrenPins, sizeof(wrenPins));

// Як часто повторно синхронізувати час через NTP (внутрішній RTC ESP32
// дрейфує на кілька секунд/добу, тому раз на 6 годин — з запасом надійно)
const unsigned long SYNC_INTERVAL_MS = 6UL * 60UL * 60UL * 1000UL; // 6 год
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000; // не висіти вічно
const int WIFI_MAX_ATTEMPTS = 3;

unsigned long lastSyncMillis = 0;
bool timeEverSynced = false;

// ---------- Прокрутка тексту ----------
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

// ---------- Wi-Fi + NTP синхронізація ----------
// Повертає true, якщо вдалося отримати час. Вмикає Wi-Fi лише на час спроби
// і завжди вимикає його перед виходом (навіть при невдачі) — це і дає економію.
bool syncTimeViaWiFi() {
  bool success = false;

  hpdl.clear();
  hpdl.print("WIFI");

  WiFi.mode(WIFI_STA);

  for (int attempt = 1; attempt <= WIFI_MAX_ATTEMPTS && !success; attempt++) {
    WiFi.begin(ssid, password);

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttempt < WIFI_CONNECT_TIMEOUT_MS) {
      delay(300);
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWi-Fi підключено!");
      hpdl.clear();
      hpdl.print("SYNC");

      configTzTime(timeZone, ntpServer);

      struct tm timeinfo;
      unsigned long syncStart = millis();
      // Чекаємо синхронізації максимум 10 секунд, а не вічно
      while (!getLocalTime(&timeinfo, 100) && millis() - syncStart < 10000) {
      }

      if (getLocalTime(&timeinfo)) {
        success = true;
      }
    } else {
      WiFi.disconnect(true);
    }
  }

  // Незалежно від результату — вимикаємо радіо повністю, це і є основна економія
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  btStop(); // на випадок якщо BT стек ініціалізовано десь у бібліотеках
  hpdl.clear();
  return success;
}

void setup() {
  hpdl.begin();
  hpdl.clear();

  // Вимикаємо Bluetooth одразу, він не використовується
  btStop();

  timeEverSynced = syncTimeViaWiFi();
  lastSyncMillis = millis();

  if (!timeEverSynced) {
    // Не блокуємось назавжди — просто покажемо помилку і продовжимо,
    // спробуємо ще раз за розкладом у loop()
    hpdl.clear();
    hpdl.print("ERR ");
    delay(2000);
  }
}

void loop() {
  // Періодична ресинхронізація за розкладом (або якщо часу ще не було)
  unsigned long now = millis();
  bool needResync = (!timeEverSynced) ||
                     (now - lastSyncMillis >= SYNC_INTERVAL_MS) ||
                     (now < lastSyncMillis); // захист від переповнення millis()

  if (needResync) {
    bool ok = syncTimeViaWiFi();
    if (ok) {
      timeEverSynced = true;
    }
    lastSyncMillis = millis();
    // Якщо невдача — не блокуємось, продовжуємо показувати останній
    // відомий (внутрішній) час і спробуємо знову за SYNC_INTERVAL_MS,
    // або можна скоротити наступну спробу — див. нижче.
    if (!ok && !timeEverSynced) {
      // Ще жодного разу не було успішної синхронізації —
      // спробуємо частіше, а не чекати повний інтервал
      lastSyncMillis = now - SYNC_INTERVAL_MS + 60000UL; // повтор через хвилину
    }
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    hpdl.clear();
    hpdl.print("----");
    delay(1000);
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
