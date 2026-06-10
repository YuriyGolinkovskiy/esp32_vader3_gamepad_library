#ifndef Vader3Gamepad_h
#define Vader3Gamepad_h

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLESecurity.h>

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