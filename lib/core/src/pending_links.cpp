#include "pending_links.h"

bool PendingLinks::add(uint16_t handle, uint32_t now) {
  if (handle == kNone) return false;
  int freeSlot = -1;
  for (int i = 0; i < kCapacity; ++i) {
    if (handles_[i] == handle) return false;
    if (handles_[i] == kNone && freeSlot < 0) freeSlot = i;
  }
  if (freeSlot < 0) return false;
  handles_[freeSlot] = handle;
  sinceMs_[freeSlot] = now;
  return true;
}

bool PendingLinks::remove(uint16_t handle) {
  if (handle == kNone) return false;
  for (uint16_t& slot : handles_) {
    if (slot == handle) {
      slot = kNone;
      return true;
    }
  }
  return false;
}

uint8_t PendingLinks::expired(uint32_t now, uint32_t timeoutMs, uint16_t* out) const {
  uint8_t n = 0;
  for (int i = 0; i < kCapacity; ++i) {
    if (handles_[i] != kNone && now - sinceMs_[i] >= timeoutMs) out[n++] = handles_[i];
  }
  return n;
}
