#include "view_math.h"

#include <math.h>
#include <stdio.h>

namespace view {

namespace {
constexpr float kPi = 3.14159265358979f;
}

float consumedDegrees(float remainingFraction) {
  if (remainingFraction > 1.0f) remainingFraction = 1.0f;
  if (remainingFraction < 0.0f) remainingFraction = 0.0f;
  return (1.0f - remainingFraction) * 360.0f;
}

float clockAngleDeg(int dx, int dy) {
  float deg = atan2f(static_cast<float>(dx), static_cast<float>(-dy)) * 180.0f / kPi;
  if (deg < 0.0f) deg += 360.0f;
  if (deg >= 360.0f) deg -= 360.0f;
  return deg;
}

bool pieFilled(int dx, int dy, float radius, float remainingFraction) {
  if (static_cast<float>(dx * dx + dy * dy) > radius * radius) return false;
  if (remainingFraction <= 0.0f) return false;
  if (remainingFraction >= 1.0f) return true;
  return clockAngleDeg(dx, dy) >= consumedDegrees(remainingFraction);
}

void formatMSS(uint32_t remainingMs, char* buf, size_t len) {
  const uint32_t secs = remainingMs / 1000UL + (remainingMs % 1000UL ? 1 : 0);
  snprintf(buf, len, "%lu:%02lu", static_cast<unsigned long>(secs / 60),
           static_cast<unsigned long>(secs % 60));
}

void formatIntervalLabel(uint32_t minutes, char* buf, size_t len) {
  snprintf(buf, len, "[%lum]", static_cast<unsigned long>(minutes));
}

}  // namespace view
