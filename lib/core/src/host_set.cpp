#include "host_set.h"

bool HostSet::add(uint16_t handle) {
  if (handle == kNone || full() || contains(handle)) return false;
  for (uint16_t& slot : handles_) {
    if (slot == kNone) {
      slot = handle;
      ++count_;
      return true;
    }
  }
  return false;
}

bool HostSet::remove(uint16_t handle) {
  if (handle == kNone) return false;
  for (uint16_t& slot : handles_) {
    if (slot == handle) {
      slot = kNone;
      --count_;
      return true;
    }
  }
  return false;
}

bool HostSet::contains(uint16_t handle) const {
  if (handle == kNone) return false;
  for (uint16_t slot : handles_) {
    if (slot == handle) return true;
  }
  return false;
}
