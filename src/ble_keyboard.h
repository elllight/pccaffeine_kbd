#pragma once

#include <stdint.h>

// Minimal BLE HID keyboard that can only tap Left Shift (spec §3, §4.1).
// Up to kMaxHosts PCs may be connected at once; each keystroke goes to all of them.
namespace ble_keyboard {

constexpr uint8_t kMaxHosts = 3;

void begin(const char* deviceName);

// Number of paired hosts connected over an encrypted link (0..kMaxHosts).
uint8_t connectedCount();

// True while at least one paired host is connected.
bool isConnected();

// True while advertising (accepting another host).
bool isAdvertising();

// Left Shift down -> holdMs -> release.
void tapLeftShift(uint32_t holdMs = 50);

}  // namespace ble_keyboard
