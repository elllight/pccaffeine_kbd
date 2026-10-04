#include "countdown.h"

Countdown::Countdown(uint32_t durationMs) : durationMs_(durationMs) {}

void Countdown::setDuration(uint32_t durationMs, uint32_t now) {
  durationMs_ = durationMs;
  startMs_ = now;
}

void Countdown::setConnected(bool connected, uint32_t now) {
  if (connected == running_) return;
  running_ = connected;
  if (connected) startMs_ = now;
}

bool Countdown::update(uint32_t now) {
  if (!running_) return false;
  if (now - startMs_ < durationMs_) return false;
  startMs_ = now;
  return true;
}

uint32_t Countdown::remainingMs(uint32_t now) const {
  if (!running_) return durationMs_;
  const uint32_t elapsed = now - startMs_;
  return elapsed >= durationMs_ ? 0 : durationMs_ - elapsed;
}

float Countdown::remainingFraction(uint32_t now) const {
  if (durationMs_ == 0) return 0.0f;
  return static_cast<float>(remainingMs(now)) / static_cast<float>(durationMs_);
}
