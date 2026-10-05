#include "button_gesture.h"

ButtonGesture::Event ButtonGesture::update(bool pressed, uint32_t now) {
  if (pressed) {
    if (!down_) {
      down_ = true;
      longFired_ = false;
      downMs_ = now;
    } else if (!longFired_ && now - downMs_ >= kLongMs) {
      longFired_ = true;
      return Event::Long;
    }
    return Event::None;
  }
  if (!down_) return Event::None;
  down_ = false;
  if (!longFired_ && now - downMs_ < kShortMaxMs) return Event::Short;
  return Event::None;
}

bool ButtonGesture::holding(uint32_t now) const {
  return down_ && !longFired_ && now - downMs_ >= kShortMaxMs;
}

uint32_t ButtonGesture::secondsToLong(uint32_t now) const {
  if (!holding(now)) return 0;
  const uint32_t left = kLongMs - (now - downMs_);
  return (left + 999) / 1000;
}
