// BasicUsage.ino - Пример использования библиотеки Vader3Gamepad

#include "Vader3Gamepad.h"

Vader3Gamepad gamepad;

const char* buttonNames[] = {
  "A", "B", "X", "Y", "LB", "RB", "LT", "RT",
  "BACK", "START", "L3", "R3", "HOME", "TURBO",
  "C1", "C2", "C3", "C4", "C5", "C6"
};

const char* dpadNames[] = {"NONE", "UP", "UP-RIGHT", "RIGHT", "DOWN-RIGHT", 
                           "DOWN", "DOWN-LEFT", "LEFT", "UP-LEFT"};

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════════╗");
  Serial.println("║     Vader 3 Pro Gamepad Tester            ║");
  Serial.println("╚════════════════════════════════════════════╝\n");
  
  gamepad.begin();
}

void loop() {
  gamepad.update();
  
  // Отслеживаем подключение сами
  static bool wasConnected = false;
  bool isConnected = gamepad.isConnected();
  
  if (isConnected && !wasConnected) {
    Serial.println("✅ Геймпад подключен!");
  }
  if (!isConnected && wasConnected) {
    Serial.println("❌ Геймпад отключен!");
  }
  wasConnected = isConnected;
  
  if (!isConnected) {
    static unsigned long lastMsg = 0;
    if (millis() - lastMsg > 2000) {
      Serial.println("⏳ Ожидание подключения геймпада...");
      lastMsg = millis();
    }
    delay(100);
    return;
  }
  
  // Кнопки
  for (int i = 0; i < 20; i++) {
    if (gamepad.justPressed((VaderButton)i)) {
      Serial.printf("🔘 НАЖАТА: %s\n", buttonNames[i]);
    }
    
    if (gamepad.justReleased((VaderButton)i)) {
      Serial.printf("🔘 ОТПУЩЕНА: %s\n", buttonNames[i]);
    }
  }
  
  // Крестовина
  static DPadDirection lastDpad = DPAD_NONE;
  DPadDirection dpad = gamepad.getDPad();
  if (dpad != lastDpad) {
    Serial.printf("◉ Крестовина: %s\n", dpadNames[dpad]);
    lastDpad = dpad;
  }
  
  // Стики
  static int lastLX = 0, lastLY = 0, lastRX = 0, lastRY = 0;
  int lx = gamepad.getLeftX();
  int ly = gamepad.getLeftY();
  int rx = gamepad.getRightX();
  int ry = gamepad.getRightY();
  
  if (abs(lx - lastLX) > 10 || abs(ly - lastLY) > 10) {
    Serial.printf("🎮 Левый стик: X=%4d Y=%4d\n", lx, ly);
    lastLX = lx; lastLY = ly;
  }
  
  if (abs(rx - lastRX) > 10 || abs(ry - lastRY) > 10) {
    Serial.printf("🎮 Правый стик: X=%4d Y=%4d\n", rx, ry);
    lastRX = rx; lastRY = ry;
  }
  
  // Триггеры
  static int lastLT = 0, lastRT = 0;
  int lt = gamepad.getLeftTrigger();
  int rt = gamepad.getRightTrigger();
  
  if (abs(lt - lastLT) > 5) {
    Serial.printf("🎯 Левый триггер: %d\n", lt);
    lastLT = lt;
  }
  
  if (abs(rt - lastRT) > 5) {
    Serial.printf("🎯 Правый триггер: %d\n", rt);
    lastRT = rt;
  }
  
  delay(20);
}