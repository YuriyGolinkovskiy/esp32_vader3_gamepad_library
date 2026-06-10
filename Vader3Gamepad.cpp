#include "Vader3Gamepad.h"

// Статические переменные
BLEClient* Vader3Gamepad::pClient = nullptr;
BLERemoteCharacteristic* Vader3Gamepad::pInputReport = nullptr;
Vader3Gamepad* Vader3Gamepad::instance = nullptr;
bool Vader3Gamepad::connected = false;
bool Vader3Gamepad::doConnect = false;
BLEAdvertisedDevice* Vader3Gamepad::myDevice = nullptr;
unsigned long Vader3Gamepad::lastDisconnectTime = 0;

// BLE UUIDs
static BLEUUID hidServiceUUID(VADER_HID_SERVICE_UUID);
static BLEUUID reportCharUUID(VADER_REPORT_CHAR_UUID);

// BLE Callbacks
class MyGamepadClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) override {
    Vader3Gamepad::connected = true;
  }
  
  void onDisconnect(BLEClient* pclient) override {
    Vader3Gamepad::connected = false;
    Vader3Gamepad::lastDisconnectTime = millis();
    
    // Очищаем указатели
    if (Vader3Gamepad::pClient) {
      delete Vader3Gamepad::pClient;
      Vader3Gamepad::pClient = nullptr;
    }
    Vader3Gamepad::pInputReport = nullptr;
  }
};

class MyGamepadAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    if (advertisedDevice.haveServiceUUID() && 
        advertisedDevice.isAdvertisingService(hidServiceUUID)) {
      
      BLEDevice::getScan()->stop();
      
      if (Vader3Gamepad::myDevice == nullptr) {
        Vader3Gamepad::myDevice = new BLEAdvertisedDevice(advertisedDevice);
      }
      Vader3Gamepad::doConnect = true;
    }
  }
};

// Конструктор
Vader3Gamepad::Vader3Gamepad() : initialized(false), pSecurity(nullptr) {
  instance = this;

  memset(currentButtons, 0, sizeof(currentButtons));
  memset(lastButtons, 0, sizeof(lastButtons));
  memset(rawAxes, 0, sizeof(rawAxes));
  memset(axes, 0, sizeof(axes));

  currentDPad = DPAD_NONE;
  lastDPad = DPAD_NONE;

  initDefaultMapping();
}

void Vader3Gamepad::initDefaultMapping() {
  axisLeftX = AXIS_LEFT_X;
  axisLeftY = AXIS_LEFT_Y;
  axisRightX = AXIS_RIGHT_X;
  axisRightY = AXIS_RIGHT_Y;
  axisLeftTrigger = AXIS_LEFT_TRIGGER;
  axisRightTrigger = AXIS_RIGHT_TRIGGER;
  
  buttonMapping[BTN_A] = {8, 4};
  buttonMapping[BTN_B] = {8, 5};
  buttonMapping[BTN_X] = {8, 6};
  buttonMapping[BTN_Y] = {8, 7};
  
  buttonMapping[BTN_LB] = {9, 0};
  buttonMapping[BTN_RB] = {9, 1};
  buttonMapping[BTN_LT] = {9, 2};
  buttonMapping[BTN_RT] = {9, 3};
  buttonMapping[BTN_BACK] = {9, 4};
  buttonMapping[BTN_START] = {9, 5};
  buttonMapping[BTN_L3] = {9, 6};
  buttonMapping[BTN_R3] = {9, 7};
  
  buttonMapping[BTN_C1] = {10, 0};
  buttonMapping[BTN_C2] = {10, 1};
  buttonMapping[BTN_C3] = {10, 2};
  buttonMapping[BTN_C4] = {10, 3};
  buttonMapping[BTN_C5] = {10, 4};
  buttonMapping[BTN_C6] = {10, 5};
  buttonMapping[BTN_TURBO] = {10, 7};
  
  buttonMapping[BTN_HOME] = {11, 7};
}

bool Vader3Gamepad::begin(int scanSeconds) {
  if (initialized) {
    return false;
  }

  BLEDevice::init("ESP32-Gamepad-Client");

  pSecurity = new BLESecurity();
  pSecurity->setCapability(ESP_IO_CAP_NONE);
  pSecurity->setAuthenticationMode(true, false, true);
  BLEDevice::setSecurityCallbacks(new BLESecurityCallbacks());

  startScan(scanSeconds);
  initialized = true;
  return true;
}

void Vader3Gamepad::end() {
  if (pClient) {
    pClient->disconnect();
    delete pClient;
    pClient = nullptr;
  }
  if (myDevice) {
    delete myDevice;
    myDevice = nullptr;
  }
  if (pSecurity) {
    delete pSecurity;
    pSecurity = nullptr;
  }
  pInputReport = nullptr;
  connected = false;
  doConnect = false;
  lastDisconnectTime = 0;
  initialized = false;
}

void Vader3Gamepad::startScan(int scanSeconds) {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyGamepadAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(SCAN_INTERVAL);
  pBLEScan->setWindow(SCAN_WINDOW);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(scanSeconds, false);
}

bool Vader3Gamepad::connectToServer() {
  if (myDevice == nullptr) return false;
  
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyGamepadClientCallback());
  
  if (!pClient->connect(myDevice)) {
    return false;
  }
  
  pClient->setMTU(BLE_MTU_SIZE);
  
  BLERemoteService* pRemoteService = pClient->getService(hidServiceUUID);
  if (pRemoteService == nullptr) {
    pClient->disconnect();
    return false;
  }
  
  pInputReport = pRemoteService->getCharacteristic(reportCharUUID);
  if (pInputReport == nullptr || !pInputReport->canNotify()) {
    pClient->disconnect();
    return false;
  }
  
  pInputReport->registerForNotify(notifyCallback);
  connected = true;
  return true;
}

void Vader3Gamepad::update() {
  // Если не подключены и не пытаемся подключиться
  if (!connected && !doConnect) {
    // Ждем RECONNECT_DELAY_MS после отключения перед переподключением
    if (lastDisconnectTime == 0 || (millis() - lastDisconnectTime > RECONNECT_DELAY_MS)) {
      // Очищаем старый device и начинаем новый скан
      if (myDevice != nullptr) {
        delete myDevice;
        myDevice = nullptr;
      }
      startScan(SCAN_DURATION_SECONDS);
      doConnect = false;
    }
  }
  
  // Если нашли устройство - подключаемся
  if (doConnect) {
    if (connectToServer()) {
      // Подключились успешно
    }
    doConnect = false;
  }
}

bool Vader3Gamepad::isConnected() {
  return connected && pClient && pClient->isConnected();
}

uint8_t Vader3Gamepad::getLeftXraw() { return rawAxes[0]; }
uint8_t Vader3Gamepad::getLeftYraw() { return rawAxes[1]; }
uint8_t Vader3Gamepad::getRightXraw() { return rawAxes[2]; }
uint8_t Vader3Gamepad::getRightYraw() { return rawAxes[3]; }
uint8_t Vader3Gamepad::getLeftTriggerRaw() { return rawAxes[4]; }
uint8_t Vader3Gamepad::getRightTriggerRaw() { return rawAxes[5]; }

int16_t Vader3Gamepad::getLeftX() { return axes[0]; }
int16_t Vader3Gamepad::getLeftY() { return axes[1]; }
int16_t Vader3Gamepad::getRightX() { return axes[2]; }
int16_t Vader3Gamepad::getRightY() { return axes[3]; }
int16_t Vader3Gamepad::getLeftTrigger() { return axes[4]; }
int16_t Vader3Gamepad::getRightTrigger() { return axes[5]; }

bool Vader3Gamepad::pressed(VaderButton btn) {
  return currentButtons[btn];
}

bool Vader3Gamepad::justPressed(VaderButton btn) {
  return currentButtons[btn] && !lastButtons[btn];
}

bool Vader3Gamepad::justReleased(VaderButton btn) {
  return !currentButtons[btn] && lastButtons[btn];
}

DPadDirection Vader3Gamepad::getDPad() {
  return (DPadDirection)currentDPad;
}

bool Vader3Gamepad::dpadPressed(DPadDirection dir) {
  return currentDPad == dir;
}

bool Vader3Gamepad::dpadJustPressed(DPadDirection dir) {
  return currentDPad == dir && lastDPad != dir;
}

int Vader3Gamepad::getDPadX() {
  switch(currentDPad) {
    case DPAD_LEFT:
    case DPAD_DOWN_LEFT:
    case DPAD_UP_LEFT:
      return -1;
    case DPAD_RIGHT:
    case DPAD_DOWN_RIGHT:
    case DPAD_UP_RIGHT:
      return 1;
    default:
      return 0;
  }
}

int Vader3Gamepad::getDPadY() {
  switch(currentDPad) {
    case DPAD_UP:
    case DPAD_UP_LEFT:
    case DPAD_UP_RIGHT:
      return -1;
    case DPAD_DOWN:
    case DPAD_DOWN_LEFT:
    case DPAD_DOWN_RIGHT:
      return 1;
    default:
      return 0;
  }
}

int16_t Vader3Gamepad::normalize(uint8_t raw) {
  if (raw == 255) return 0;
  if (raw < 128) return raw;
  return raw - 255;
}

void Vader3Gamepad::parseData(uint8_t* data, size_t length) {
  memcpy(lastButtons, currentButtons, sizeof(currentButtons));
  lastDPad = currentDPad;

  if (length >= 20) {
    rawAxes[0] = data[axisLeftX];
    rawAxes[1] = data[axisLeftY];
    rawAxes[2] = data[axisRightX];
    rawAxes[3] = data[axisRightY];
    
    // Проверка границ для триггеров
    if (axisLeftTrigger < length) rawAxes[4] = data[axisLeftTrigger];
    if (axisRightTrigger < length) rawAxes[5] = data[axisRightTrigger];

    axes[0] = normalize(rawAxes[0]);
    axes[2] = normalize(rawAxes[2]);
    axes[1] = normalize(rawAxes[1]);
    axes[3] = normalize(rawAxes[3]);
    axes[4] = rawAxes[4];
    axes[5] = rawAxes[5];

    uint8_t dpadValue = data[DPAD_BYTE_INDEX] & DPAD_MASK;
    switch(dpadValue) {
      case DPAD_VAL_UP: currentDPad = DPAD_UP; break;
      case DPAD_VAL_UP_RIGHT: currentDPad = DPAD_UP_RIGHT; break;
      case DPAD_VAL_RIGHT: currentDPad = DPAD_RIGHT; break;
      case DPAD_VAL_DOWN_RIGHT: currentDPad = DPAD_DOWN_RIGHT; break;
      case DPAD_VAL_DOWN: currentDPad = DPAD_DOWN; break;
      case DPAD_VAL_DOWN_LEFT: currentDPad = DPAD_DOWN_LEFT; break;
      case DPAD_VAL_LEFT: currentDPad = DPAD_LEFT; break;
      case DPAD_VAL_UP_LEFT: currentDPad = DPAD_UP_LEFT; break;
      default: currentDPad = DPAD_NONE; break;
    }

    for (int i = 0; i < BUTTON_COUNT; i++) {
      int byteIdx = buttonMapping[i].byteIndex;
      int bit = buttonMapping[i].bit;
      if (byteIdx < length) {
        currentButtons[i] = (data[byteIdx] >> bit) & 1;
      }
    }
  }
}

void Vader3Gamepad::notifyCallback(BLERemoteCharacteristic* pChar, 
                                   uint8_t* pData, size_t length, bool isNotify) {
  if (instance) {
    instance->parseData(pData, length);
  }
}