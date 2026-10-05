#include "ble_keyboard.h"

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

#include <atomic>
#include <mutex>

#include "host_set.h"
#include "pending_links.h"

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

constexpr uint32_t kPairingTimeoutMs = 30000;  // drop links that never finish pairing

NimBLEHIDDevice* hid = nullptr;
NimBLECharacteristic* input = nullptr;

static_assert(HostSet::kNone == BLE_HS_CONN_HANDLE_NONE, "HostSet sentinel must match NimBLE");
static_assert(PendingLinks::kNone == BLE_HS_CONN_HANDLE_NONE, "sentinel must match NimBLE");
static_assert(HostSet::kCapacity == kMaxHosts, "HostSet capacity must match kMaxHosts");
static_assert(CONFIG_BT_NIMBLE_MAX_CONNECTIONS > kMaxHosts,
              "keep a spare raw connection so an unpaired link cannot block a real host");
static_assert(PendingLinks::kCapacity == CONFIG_BT_NIMBLE_MAX_CONNECTIONS,
              "PendingLinks must track every raw connection");

// Paired hosts (encrypted links) and raw links still pairing. Written from the
// NimBLE host task callbacks, read from loop(); guarded by stateMutex. A raw GAP
// link is not enough: before pairing completes the host has not subscribed to
// the input report.
HostSet hosts;
PendingLinks pending;
std::mutex stateMutex;
std::atomic<uint8_t> hostCount{0};
std::atomic<bool> resetting{false};

// Callbacks run on the NimBLE host task, which must never block on Serial
// (USB CDC writes can stall). They post events here; maintain() logs them.
enum class EventType : uint8_t {
  LinkUp,
  LinkDown,
  PairingFailed,
  HostsChanged,
  RejectedDuringReset
};
struct Event {
  EventType type;
  uint16_t handle;
  int reason;
  uint8_t hosts;
};
QueueHandle_t events = nullptr;

void post(EventType type, uint16_t handle, int reason = 0) {
  if (events == nullptr) return;
  const Event e{type, handle, reason, hostCount.load()};
  xQueueSend(events, &e, 0);  // drop the log line rather than block the BLE stack
}

void logEvents() {
  Event e;
  while (events != nullptr && xQueueReceive(events, &e, 0) == pdTRUE) {
    switch (e.type) {
      case EventType::LinkUp:
        Serial.printf("[ble] link up (handle=%u), waiting for pairing\n", e.handle);
        break;
      case EventType::LinkDown:
        Serial.printf("[ble] link down (handle=%u, reason=0x%x)\n", e.handle, e.reason);
        break;
      case EventType::PairingFailed:
        Serial.printf("[ble] pairing failed (handle=%u)\n", e.handle);
        break;
      case EventType::HostsChanged:
        Serial.printf("[ble] hosts=%u/%u\n", e.hosts, kMaxHosts);
        break;
      case EventType::RejectedDuringReset:
        Serial.printf("[ble] link %u rejected during pairing reset\n", e.handle);
        break;
    }
  }
}

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* server, NimBLEConnInfo& info) override {
    const uint16_t handle = info.getConnHandle();
    if (resetting) {
      // Advertising must stay off while bonds are deleted (see clearPairings).
      server->disconnect(handle);
      post(EventType::RejectedDuringReset, handle);
      return;
    }
    {
      std::lock_guard<std::mutex> lock(stateMutex);
      pending.add(handle, millis());
    }
    post(EventType::LinkUp, handle);
    // A connection stops advertising; keep the device visible while a host slot is free.
    if (hostCount < kMaxHosts) NimBLEDevice::startAdvertising();
  }

  void onAuthenticationComplete(NimBLEConnInfo& info) override {
    const uint16_t handle = info.getConnHandle();
    if (!info.isEncrypted()) {
      post(EventType::PairingFailed, handle);
      return;
    }
    bool changed;
    {
      std::lock_guard<std::mutex> lock(stateMutex);
      pending.remove(handle);
      changed = hosts.add(handle);
      hostCount = hosts.count();
    }
    if (changed) post(EventType::HostsChanged, handle);
    if (hostCount >= kMaxHosts) NimBLEDevice::stopAdvertising();
  }

  void onDisconnect(NimBLEServer*, NimBLEConnInfo& info, int reason) override {
    const uint16_t handle = info.getConnHandle();
    bool changed;
    {
      std::lock_guard<std::mutex> lock(stateMutex);
      pending.remove(handle);
      changed = hosts.remove(handle);
      hostCount = hosts.count();
    }
    post(EventType::LinkDown, handle, reason);
    if (changed) post(EventType::HostsChanged, handle);
    // advertiseOnDisconnect(true) restarts advertising.
  }
};

ServerCallbacks serverCallbacks;

void sendReport(uint8_t modifiers) {
  uint8_t report[8] = {modifiers, 0, 0, 0, 0, 0, 0, 0};
  input->setValue(report, sizeof(report));
  if (!input->notify()) Serial.println("[ble] notify failed");
}

}  // namespace

void begin(const char* deviceName) {
  events = xQueueCreate(16, sizeof(Event));
  NimBLEDevice::init(deviceName);
  NimBLEDevice::setSecurityAuth(true, false, true);  // bonding, no MITM, secure connections
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(&serverCallbacks, false);
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

uint8_t connectedCount() {
  return hostCount.load();
}

bool isConnected() {
  return connectedCount() > 0;
}

bool isAdvertising() {
  return NimBLEDevice::getAdvertising()->isAdvertising();
}

void maintain() {
  logEvents();
  if (resetting) return;

  uint16_t stale[PendingLinks::kCapacity];
  uint8_t staleCount;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    staleCount = pending.expired(millis(), kPairingTimeoutMs, stale);
  }
  NimBLEServer* server = NimBLEDevice::getServer();
  for (uint8_t i = 0; i < staleCount; ++i) {
    Serial.printf("[ble] link %u did not pair within %lus, disconnecting\n", stale[i],
                  static_cast<unsigned long>(kPairingTimeoutMs / 1000));
    server->disconnect(stale[i]);
  }

  if (hostCount >= kMaxHosts) {
    if (isAdvertising()) NimBLEDevice::stopAdvertising();
    return;
  }
  if (isAdvertising() || server->getConnectedCount() >= CONFIG_BT_NIMBLE_MAX_CONNECTIONS) return;
  if (NimBLEDevice::startAdvertising()) Serial.println("[ble] advertising resumed");
}

int bondCount() {
  return NimBLEDevice::getNumBonds();
}

int clearPairings() {
  const int bonds = NimBLEDevice::getNumBonds();
  NimBLEServer* server = NimBLEDevice::getServer();

  // ble_gap_unpair() refuses with BLE_HS_EBUSY while advertising if the peer
  // distributed an IRK (all Apple hosts do), so keep advertising off until done.
  // `resetting` stops onConnect from restarting it and rejects new links meanwhile.
  resetting = true;
  server->advertiseOnDisconnect(false);
  NimBLEDevice::stopAdvertising();
  for (uint16_t handle : server->getPeerDevices()) server->disconnect(handle);

  bool ok = false;
  for (int attempt = 0; attempt < 10 && !ok; ++attempt) {
    if (attempt > 0) delay(50);
    NimBLEDevice::stopAdvertising();
    ok = NimBLEDevice::deleteAllBonds();
  }

  server->advertiseOnDisconnect(true);
  resetting = false;
  NimBLEDevice::startAdvertising();
  return ok ? bonds : -1;
}

void tapLeftShift(uint32_t holdMs) {
  if (!isConnected()) return;
  sendReport(kModLeftShift);
  delay(holdMs);
  sendReport(0);
}

}  // namespace ble_keyboard
