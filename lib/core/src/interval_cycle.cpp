#include "interval_cycle.h"

namespace interval {

namespace {
constexpr uint8_t kMinutes[kCount] = {1, 5, 8};
}

uint8_t sanitize(int32_t raw) {
  if (raw < 0 || raw >= kCount) return kDefaultIndex;
  return static_cast<uint8_t>(raw);
}

uint8_t next(uint8_t index) {
  return static_cast<uint8_t>((sanitize(index) + 1) % kCount);
}

uint32_t minutesAt(uint8_t index) {
  return kMinutes[sanitize(index)];
}

uint32_t durationMs(uint8_t index) {
  return minutesAt(index) * 60UL * 1000UL;
}

}  // namespace interval
