#pragma once

#include <stdint.h>

// Button debouncer that reports one event per press edge (spec §4.5).
// `rawPressed` is the logical state (caller converts active-low wiring).
class Debouncer {
 public:
  explicit Debouncer(uint32_t stableMs = 30) : stableMs_(stableMs) {}

  // Returns true once when a press has been stable for `stableMs`.
  bool update(bool rawPressed, uint32_t now);

  bool pressed() const { return stable_; }

 private:
  uint32_t stableMs_;
  uint32_t lastChangeMs_ = 0;
  bool lastRaw_ = false;
  bool stable_ = false;
};
