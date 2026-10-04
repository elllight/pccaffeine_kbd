#pragma once

#include <stddef.h>
#include <stdint.h>

// Pure helpers for the 72x40 status screen (spec §4.4).
namespace view {

// Degrees of the pie already consumed, clockwise from 12 o'clock (0..360).
float consumedDegrees(float remainingFraction);

// Clockwise angle from 12 o'clock in degrees [0, 360) for a screen offset
// (dx to the right, dy downwards).
float clockAngleDeg(int dx, int dy);

// True if the pixel at (dx, dy) from the centre belongs to the remaining
// (filled) part of the pie. Elapsed time eats the pie clockwise like a clock hand.
bool pieFilled(int dx, int dy, float radius, float remainingFraction);

// "M:SS" with seconds rounded up so the display never shows 0:00 early.
void formatMSS(uint32_t remainingMs, char* buf, size_t len);

// "[Nm]"
void formatIntervalLabel(uint32_t minutes, char* buf, size_t len);

}  // namespace view
