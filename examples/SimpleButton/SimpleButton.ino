// SimpleButton.ino - Простой пример управления светодиодом с помощью кнопки A

#include "Vader3Gamepad.h"

Vader3Gamepad gamepad;

// Используем встроенный светодиод ESP32 (обычно GPIO 2)
static constexpr int LED_PIN = 2;
static constexpr unsigned long DEBOUNCE_DELAY_MS = 100;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);

  // Запускаем поиск геймпада
  if (!gamepad.begin()) {
    Serial.println("Ошибка инициализации геймпада!");
  }
}

void loop() {
  // Обновляем состояние геймпада (обязательно!)
  gamepad.update();

  // Если геймпад подключен
  if (gamepad.isConnected()) {
    // Включаем светодиод при нажатии кнопки A
    digitalWrite(LED_PIN, gamepad.pressed(BTN_A) ? HIGH : LOW);

    // Пример обработки однократного нажатия (переключение)
    if (gamepad.justPressed(BTN_B)) {
      Serial.println("Кнопка B нажата!");
    }
  }

  delay(DEBOUNCE_DELAY_MS);
}
