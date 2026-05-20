#include <Arduino.h> 
#include <HPDL1414.h>

// Конфігурація пінів (як у попередньому кроці)
const byte dataPins[7] = {2, 3, 4, 5, 6, 7, 8}; 
const byte addrPins[2] = {9, 10};               
const byte wrenPins[]  = {11};                  

HPDL1414 hpdl(dataPins, addrPins, wrenPins, sizeof(wrenPins));

void setup() {
  hpdl.begin();
  hpdl.clear();
}

void loop() {
  // --- Етап 1: Почергове виведення цифр від 0 до 9 на весь екран ---
  for (int i = 0; i <= 9; i++) {
    char buffer[5]; // Буфер для 4-х символів + термінатор нуля '\0'
    
    // Формуємо рядок типу "0000", "1111", "2222" і т.д.
    snprintf(buffer, sizeof(buffer), "%d%d%d%d", i, i, i, i);
    
    hpdl.clear();
    hpdl.print(buffer);
    delay(800); // Затримка менше секунди, щоб цифри змінювалися жвавіше
  }

  delay(1000); // Невелика пауза перед наступним ефектом

  // --- Етап 2: Ефект "лічильника" від 0000 до 0050 ---
  // Покаже, як індикатор працює з динамічними числами
  for (int count = 0; count <= 50; count++) {
    char buffer[5];
    
    // Флаг %04d означає: вивести число, доповнивши його нулями зліва до 4 знаків
    snprintf(buffer, sizeof(buffer), "%04d", count);
    
    hpdl.clear();
    hpdl.print(buffer);
    delay(150); // Швидке цокання лічильника
  }

  delay(2000); // Пауза перед повторенням всього циклу loop
}