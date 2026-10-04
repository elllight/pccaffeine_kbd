#pragma once

#include <stdint.h>

// Periodic countdown that only runs while a BLE host is connected (spec §4.3).
// Time is injected as a millis()-style uint32_t, so wraparound is handled by
// unsigned subtraction.
class Countdown {
 public:
  explicit Countdown(uint32_t durationMs);

  // Changes the interval and restarts the countdown from full (if running).
  void setDuration(uint32_t durationMs, uint32_t now);

  // Rising edge starts from full; falling edge pauses. Repeated values are no-ops.
  void setConnected(bool connected, uint32_t now);

  // Returns true exactly once per expiry, then restarts from full at `now`.
  bool update(uint32_t now);

  bool running() const { return running_; }
  uint32_t durationMs() const { return durationMs_; }
  uint32_t remainingMs(uint32_t now) const;
  float remainingFraction(uint32_t now) const;

 private:
  uint32_t durationMs_;
  uint32_t startMs_ = 0;
  bool running_ = false;
};
