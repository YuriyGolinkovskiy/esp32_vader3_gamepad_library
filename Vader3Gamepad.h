#ifndef Vader3Gamepad_h
#define Vader3Gamepad_h

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLESecurity.h>

// BLE Constants
static constexpr uint16_t VADER_HID_SERVICE_UUID = 0x1812;
static constexpr uint16_t VADER_REPORT_CHAR_UUID = 0x2A4D;

// Scan parameters
static constexpr uint16_t SCAN_INTERVAL = 1349;
static constexpr uint16_t SCAN_WINDOW = 449;
static constexpr uint16_t SCAN_DURATION_SECONDS = 5;
static constexpr uint32_t RECONNECT_DELAY_MS = 3000;
static constexpr uint16_t BLE_MTU_SIZE = 185;

// Data indices
static constexpr uint8_t AXIS_LEFT_X = 0;
static constexpr uint8_t AXIS_LEFT_Y = 1;
static constexpr uint8_t AXIS_RIGHT_X = 2;
static constexpr uint8_t AXIS_RIGHT_Y = 3;
static constexpr uint8_t AXIS_LEFT_TRIGGER = 12;
static constexpr uint8_t AXIS_RIGHT_TRIGGER = 13;
static constexpr uint8_t DPAD_BYTE_INDEX = 8;
static constexpr uint8_t DPAD_MASK = 0x0F;

// Array sizes
static constexpr uint8_t BUTTON_COUNT = 20;
static constexpr uint8_t AXIS_COUNT = 6;

// D-pad values (from BLE report)
static constexpr uint8_t DPAD_VAL_NONE = 0x00;
static constexpr uint8_t DPAD_VAL_UP = 0x01;
static constexpr uint8_t DPAD_VAL_UP_RIGHT = 0x02;
static constexpr uint8_t DPAD_VAL_RIGHT = 0x03;
static constexpr uint8_t DPAD_VAL_DOWN_RIGHT = 0x04;
static constexpr uint8_t DPAD_VAL_DOWN = 0x05;
static constexpr uint8_t DPAD_VAL_DOWN_LEFT = 0x06;
static constexpr uint8_t DPAD_VAL_LEFT = 0x07;
static constexpr uint8_t DPAD_VAL_UP_LEFT = 0x08;

enum VaderButton {
  BTN_A = 0,
  BTN_B = 1,
  BTN_X = 2,
  BTN_Y = 3,
  BTN_LB = 4,
  BTN_RB = 5,
  BTN_LT = 6,
  BTN_RT = 7,
  BTN_BACK = 8,
  BTN_START = 9,
  BTN_L3 = 10,
  BTN_R3 = 11,
  BTN_HOME = 12,
  BTN_TURBO = 13,
  BTN_C1 = 14,
  BTN_C2 = 15,
  BTN_C3 = 16,
  BTN_C4 = 17,
  BTN_C5 = 18,
  BTN_C6 = 19
};

enum DPadDirection {
  DPAD_NONE = 0,
  DPAD_UP = 1,
  DPAD_UP_RIGHT = 2,
  DPAD_RIGHT = 3,
  DPAD_DOWN_RIGHT = 4,
  DPAD_DOWN = 5,
  DPAD_DOWN_LEFT = 6,
  DPAD_LEFT = 7,
  DPAD_UP_LEFT = 8
};

class Vader3Gamepad {
public:
  Vader3Gamepad();
  
  bool begin(int scanSeconds = 5);
  void end();
  void update();
  bool isConnected();
  
  uint8_t getLeftXraw();
  uint8_t getLeftYraw();
  uint8_t getRightXraw();
  uint8_t getRightYraw();
  uint8_t getLeftTriggerRaw();
  uint8_t getRightTriggerRaw();
  
  int16_t getLeftX();
  int16_t getLeftY();
  int16_t getRightX();
  int16_t getRightY();
  int16_t getLeftTrigger();
  int16_t getRightTrigger();
  
  bool pressed(VaderButton btn);
  bool justPressed(VaderButton btn);
  bool justReleased(VaderButton btn);
  
  DPadDirection getDPad();
  bool dpadPressed(DPadDirection dir);
  bool dpadJustPressed(DPadDirection dir);
  int getDPadX();
  int getDPadY();

private:
  static BLEClient* pClient;
  static BLERemoteCharacteristic* pInputReport;
  static Vader3Gamepad* instance;
  static bool connected;
  static bool doConnect;
  static BLEAdvertisedDevice* myDevice;
  static unsigned long lastDisconnectTime;
  
  static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);
  static bool connectToServer();
  static void startScan(int scanSeconds);
  
  bool currentButtons[20];
  bool lastButtons[20];
  
  uint8_t currentDPad;
  uint8_t lastDPad;
  
  uint8_t rawAxes[6];
  int16_t axes[6];

  bool initialized;
  BLESecurity* pSecurity;

  int axisLeftX, axisLeftY, axisRightX, axisRightY, axisLeftTrigger, axisRightTrigger;
  
  struct ButtonMapping {
    int byteIndex;
    int bit;
  } buttonMapping[20];
  
  void initDefaultMapping();
  void parseData(uint8_t* data, size_t length);
  int16_t normalize(uint8_t raw);
  
  friend class MyGamepadClientCallback;
  friend class MyGamepadAdvertisedDeviceCallbacks;
};

#endif