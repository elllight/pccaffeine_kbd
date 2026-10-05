#pragma once

#include <stdint.h>

// Set of paired host connection handles, bounded to kCapacity (spec §3 Multi-host).
// Not thread-safe on its own; the BLE adapter guards it.
class HostSet {
 public:
  static constexpr uint8_t kCapacity = 3;
  static constexpr uint16_t kNone = 0xFFFF;  // == BLE_HS_CONN_HANDLE_NONE

  // Returns true if the handle was newly added (false: duplicate, invalid or full).
  bool add(uint16_t handle);

  // Returns true if the handle was present and removed.
  bool remove(uint16_t handle);

  bool contains(uint16_t handle) const;
  uint8_t count() const { return count_; }
  bool full() const { return count_ >= kCapacity; }

 private:
  uint16_t handles_[kCapacity] = {kNone, kNone, kNone};
  uint8_t count_ = 0;
};
