#pragma once

#include <stdint.h>

// BOOT button gestures (spec §4.5). Feed it the debounced pressed state.
//   Short: released before kShortMaxMs  -> next interval
//   (released between kShortMaxMs and kLongMs -> cancelled)
//   Long:  held for kLongMs, fires once while still held -> clear pairings
class ButtonGesture {
 public:
  enum class Event : uint8_t { None, Short, Long };

  static constexpr uint32_t kShortMaxMs = 1000;
  static constexpr uint32_t kLongMs = 5000;

  Event update(bool pressed, uint32_t now);

  // True while held long enough to show the reset countdown and Long has not fired.
  bool holding(uint32_t now) const;

  // Whole seconds (rounded up) left until Long fires; 0 when not counting down.
  uint32_t secondsToLong(uint32_t now) const;

 private:
  uint32_t downMs_ = 0;
  bool down_ = false;
  bool longFired_ = false;
};
