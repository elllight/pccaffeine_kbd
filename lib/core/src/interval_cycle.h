#pragma once

#include <stdint.h>

// Keystroke interval selection: 1 min -> 5 min -> 8 min -> 1 min ... (spec §4.2)
namespace interval {

constexpr uint8_t kCount = 3;
constexpr uint8_t kDefaultIndex = 0;

// Maps any stored/raw value to a valid index; invalid values fall back to 1 min.
uint8_t sanitize(int32_t raw);

// Index of the interval that follows `index` (wraps around).
uint8_t next(uint8_t index);

uint32_t minutesAt(uint8_t index);
uint32_t durationMs(uint8_t index);

}  // namespace interval
