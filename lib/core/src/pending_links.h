#pragma once

#include <stdint.h>

// Raw BLE links that have not finished pairing yet, with their connect time.
// Used to drop links that never pair so they cannot hold a connection slot
// forever (spec §3 Multi-host). Not thread-safe on its own.
class PendingLinks {
 public:
  static constexpr uint8_t kCapacity = 4;    // == CONFIG_BT_NIMBLE_MAX_CONNECTIONS
  static constexpr uint16_t kNone = 0xFFFF;  // == BLE_HS_CONN_HANDLE_NONE

  // Returns false for an invalid handle, a duplicate (original time kept) or when full.
  bool add(uint16_t handle, uint32_t now);
  bool remove(uint16_t handle);

  // Writes handles pending for at least timeoutMs into out (kCapacity slots); returns count.
  uint8_t expired(uint32_t now, uint32_t timeoutMs, uint16_t* out) const;

 private:
  uint16_t handles_[kCapacity] = {kNone, kNone, kNone, kNone};
  uint32_t sinceMs_[kCapacity] = {};
};
