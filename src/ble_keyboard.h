#pragma once

#include <stdint.h>

// Minimal BLE HID keyboard that can only tap Left Shift (spec §3, §4.1).
namespace ble_keyboard {

void begin(const char* deviceName);

// True while a paired host is connected over an encrypted link.
bool isConnected();

// Left Shift down -> holdMs -> release.
void tapLeftShift(uint32_t holdMs = 50);

}  // namespace ble_keyboard
