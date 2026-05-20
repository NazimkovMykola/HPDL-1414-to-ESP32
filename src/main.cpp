#include <Arduino.h>
#include <HPDL1414.h>

// Нова конфігурація пінів для ESP32
// Порядок у масиві даних: D0, D1, D2, D3, D4, D5, D6
const byte dataPins[7] = {13, 12, 14, 27, 5, 18, 19}; 
// Піни адреси: A0, A1
const byte addrPins[2] = {21, 22};               
// Пін вибору/запису (!WR)
const byte wrenPins[]  = {23};                  

HPDL1414 hpdl(dataPins, addrPins, wrenPins, sizeof(wrenPins));

void setup() {
  hpdl.begin();
  hpdl.clear();
}

void loop() {
  // 1. Виведення цифр від 0 до 9
  for (int i = 0; i <= 9; i++) {
    char buffer[5];
    snprintf(buffer, sizeof(buffer), "%d%d%d%d", i, i, i, i);
    hpdl.clear();
    hpdl.print(buffer);
    delay(600); 
  }

  delay(1000);

  // 2. Ефект швидкого лічильника
  for (int count = 0; count <= 99; count++) {
    char buffer[5];
    snprintf(buffer, sizeof(buffer), "%04d", count);
    hpdl.clear();
    hpdl.print(buffer);
    delay(100); 
  }

  delay(2000);
}