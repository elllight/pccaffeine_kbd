#pragma once

#include <stdint.h>

// 72x40 SSD1306 status screen (spec §4.4).
namespace screen {

struct State {
  bool connected;
  bool blinkOn;             // toggles while waiting for a host
  float remainingFraction;  // 0..1
  uint32_t remainingMs;
  uint32_t intervalMinutes;
  bool inverted;  // keystroke feedback flash
};

void begin(uint8_t sda, uint8_t scl);
void draw(const State& s);

}  // namespace screen
