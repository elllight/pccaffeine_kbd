#include "debouncer.h"

bool Debouncer::update(bool rawPressed, uint32_t now) {
  if (rawPressed != lastRaw_) {
    lastRaw_ = rawPressed;
    lastChangeMs_ = now;
  }
  if (rawPressed == stable_ || now - lastChangeMs_ < stableMs_) return false;
  stable_ = rawPressed;
  return stable_;
}
