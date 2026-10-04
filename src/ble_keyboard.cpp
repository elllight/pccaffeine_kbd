#include "ble_keyboard.h"

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

#include <atomic>

namespace ble_keyboard {

namespace {

constexpr uint8_t kReportId = 1;
constexpr uint8_t kModLeftShift = 0x02;
constexpr uint16_t kAppearanceKeyboard = 0x03C1;

// Standard keyboard input report: modifiers, reserved, 6 key codes.
const uint8_t kReportMap[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x06,       // Usage (Keyboard)
    0xA1, 0x01,       // Collection (Application)
    0x85, kReportId,  //   Report ID
    0x05, 0x07,       //   Usage Page (Key Codes)
    0x19, 0xE0,       //   Usage Minimum (224)
    0x29, 0xE7,       //   Usage Maximum (231)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x75, 0x01,       //   Report Size (1)
    0x95, 0x08,       //   Report Count (8)
    0x81, 0x02,       //   Input (Data, Variable, Absolute) ; modifiers
    0x95, 0x01,       //   Report Count (1)
    0x75, 0x08,       //   Report Size (8)
    0x81, 0x01,       //   Input (Constant) ; reserved
    0x95, 0x06,       //   Report Count (6)
    0x75, 0x08,       //   Report Size (8)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x65,       //   Logical Maximum (101)
    0x05, 0x07,       //   Usage Page (Key Codes)
    0x19, 0x00,       //   Usage Minimum (0)
    0x29, 0x65,       //   Usage Maximum (101)
    0x81, 0x00,       //   Input (Data, Array) ; key codes
    0xC0,             // End Collection
};

NimBLEHIDDevice* hid = nullptr;
NimBLECharacteristic* input = nullptr;

// Handle of the encrypted (paired) host link. Written from the NimBLE host task,
// read from loop(). A raw GAP link is not enough: before pairing completes the
// host has not subscribed to the input report and notify() is silently dropped.
std::atomic<uint16_t> secureConn{BLE_HS_CONN_HANDLE_NONE};

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer*, NimBLEConnInfo& info) override {
    Serial.printf("[ble] link up (handle=%u), waiting for pairing\n", info.getConnHandle());
  }

  void onAuthenticationComplete(NimBLEConnInfo& info) override {
    if (info.isEncrypted()) {
      secureConn = info.getConnHandle();
    } else {
      Serial.printf("[ble] pairing failed (handle=%u)\n", info.getConnHandle());
    }
  }

  void onDisconnect(NimBLEServer*, NimBLEConnInfo& info, int reason) override {
    uint16_t handle = info.getConnHandle();
    secureConn.compare_exchange_strong(handle, BLE_HS_CONN_HANDLE_NONE);
    Serial.printf("[ble] link down (reason=0x%x)\n", reason);
  }
};

ServerCallbacks serverCallbacks;

void sendReport(uint8_t modifiers) {
  uint8_t report[8] = {modifiers, 0, 0, 0, 0, 0, 0, 0};
  input->setValue(report, sizeof(report));
  if (!input->notify(secureConn.load())) Serial.println("[ble] notify failed");
}

}  // namespace

void begin(const char* deviceName) {
  NimBLEDevice::init(deviceName);
  NimBLEDevice::setSecurityAuth(true, false, true);  // bonding, no MITM, secure connections
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(&serverCallbacks);
  server->advertiseOnDisconnect(true);

  hid = new NimBLEHIDDevice(server);
  input = hid->getInputReport(kReportId);
  hid->setManufacturer("PCCaffeine");
  hid->setPnp(0x02, 0xE502, 0xA111, 0x0210);
  hid->setHidInfo(0x00, 0x01);
  hid->setReportMap(const_cast<uint8_t*>(kReportMap), sizeof(kReportMap));
  hid->setBatteryLevel(100);

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->setAppearance(kAppearanceKeyboard);
  adv->addServiceUUID(hid->getHidService()->getUUID());
  adv->setName(deviceName);
  adv->enableScanResponse(true);
  adv->start();  // also starts the GATT server
}

bool isConnected() {
  return secureConn.load() != BLE_HS_CONN_HANDLE_NONE;
}

void tapLeftShift(uint32_t holdMs) {
  if (!isConnected()) return;
  sendReport(kModLeftShift);
  delay(holdMs);
  sendReport(0);
}

}  // namespace ble_keyboard
