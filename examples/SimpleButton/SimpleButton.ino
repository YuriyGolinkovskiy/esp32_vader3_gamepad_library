// SimpleButton.ino - Простой пример управления светодиодом с помощью кнопки A

#include <Vader3Gamepad.h>

Vader3Gamepad gamepad;

// Используем встроенный светодиод ESP32 (обычно GPIO 2)
const int LED_PIN = 2;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  
  // Запускаем поиск геймпада
  gamepad.begin();
}

void loop() {
  // Обновляем состояние геймпада (обязательно!)
  gamepad.update();

  // Если геймпад подключен
  if (gamepad.isConnected()) {
    // Включаем светодиод при нажатии кнопки A
    if (gamepad.pressed(BTN_A)) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }

    // Пример обработки однократного нажатия (переключение)
    if (gamepad.justPressed(BTN_B)) {
      Serial.println("Кнопка B нажата!");
    }
  }
}
