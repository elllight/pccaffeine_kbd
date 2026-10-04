#pragma once

#include <stdint.h>

// ESP32-C3 0.42" OLED board (spec §2). Verified on hardware in Phase 3.
namespace pins {
constexpr uint8_t kOledSda = 5;
constexpr uint8_t kOledScl = 6;
constexpr uint8_t kLed = 8;     // active-low
constexpr uint8_t kButton = 9;  // BOOT button, active-low
}  // namespace pins
