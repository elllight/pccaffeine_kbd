#include "screen.h"

#include <U8g2lib.h>
#include <Wire.h>

#include "view_math.h"

namespace screen {

namespace {

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

constexpr int kPieCx = 19;
constexpr int kPieCy = 20;
constexpr int kPieR = 18;
constexpr int kTextX = 41;

void drawPie(float remainingFraction, bool connected) {
  if (connected) {
    const float r = static_cast<float>(kPieR) + 0.5f;
    for (int dy = -kPieR; dy <= kPieR; ++dy) {
      for (int dx = -kPieR; dx <= kPieR; ++dx) {
        if (view::pieFilled(dx, dy, r, remainingFraction)) u8g2.drawPixel(kPieCx + dx, kPieCy + dy);
      }
    }
  }
  u8g2.drawCircle(kPieCx, kPieCy, kPieR);
}

}  // namespace

void begin(uint8_t sda, uint8_t scl) {
  Wire.begin(sda, scl);
  u8g2.begin();
  u8g2.setBusClock(400000);
  u8g2.setFontPosTop();
}

void draw(const State& s) {
  char buf[8];
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);

  drawPie(s.remainingFraction, s.connected);

  // BLE state
  u8g2.setFont(u8g2_font_5x7_tr);
  if (s.connected) {
    u8g2.drawStr(kTextX, 0, "BT ON");
  } else if (s.blinkOn) {
    u8g2.drawStr(kTextX, 0, "PAIR");
  }

  // Remaining time
  u8g2.setFont(u8g2_font_7x13B_tr);
  if (s.connected) {
    view::formatMSS(s.remainingMs, buf, sizeof(buf));
  } else {
    snprintf(buf, sizeof(buf), "-:--");
  }
  u8g2.drawStr(kTextX, 12, buf);

  // Interval
  u8g2.setFont(u8g2_font_5x7_tr);
  view::formatIntervalLabel(s.intervalMinutes, buf, sizeof(buf));
  u8g2.drawStr(kTextX, 31, buf);

  if (s.inverted) {
    u8g2.setDrawColor(2);  // XOR
    u8g2.drawBox(0, 0, 72, 40);
    u8g2.setDrawColor(1);
  }
  u8g2.sendBuffer();
}

}  // namespace screen
