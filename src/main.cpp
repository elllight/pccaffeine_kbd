// PCCaffeine KBD — taps Left Shift over BLE every 1/5/8 minutes (see spec.md).
#include <Arduino.h>

#include "ble_keyboard.h"
#include "button_gesture.h"
#include "countdown.h"
#include "debouncer.h"
#include "interval_cycle.h"
#include "pins.h"
#include "screen.h"
#include "settings.h"
#include "view_math.h"

namespace {

constexpr const char* kDeviceName = "PCCaffeine";
constexpr uint32_t kFrameMs = 100;
constexpr uint32_t kFlashMs = 200;
constexpr uint32_t kBlinkMs = 500;
constexpr uint32_t kStatusLogMs = 10000;

uint8_t intervalIndex = interval::kDefaultIndex;
Countdown countdown(interval::durationMs(interval::kDefaultIndex));
Debouncer button(30);
ButtonGesture gesture;
bool wasConnected = false;
uint32_t flashUntilMs = 0;
bool flashing = false;
uint32_t lastFrameMs = 0;
uint32_t lastStatusLogMs = 0;

void setLed(bool on) {
  digitalWrite(pins::kLed, on ? LOW : HIGH);
}

void onButtonPressed(uint32_t now) {
  intervalIndex = interval::next(intervalIndex);
  settings::saveIntervalIndex(intervalIndex);
  countdown.setDuration(interval::durationMs(intervalIndex), now);
  Serial.printf("[btn] interval=%lum\n",
                static_cast<unsigned long>(interval::minutesAt(intervalIndex)));
}

void startFlash(uint32_t now) {
  flashing = true;
  flashUntilMs = now + kFlashMs;
  setLed(true);
}

void onPairingReset(uint32_t now) {
  const int cleared = ble_keyboard::clearPairings();
  startFlash(now);
  Serial.printf("[btn] pairing reset, %d bond(s) cleared\n", cleared);
}

void onConnectionChanged(bool connected) {
  Serial.printf("[ble] %s\n",
                connected ? "first host ready, timer started" : "no hosts, timer paused");
}

void onFire(uint32_t now) {
  ble_keyboard::tapLeftShift();
  startFlash(now);
  Serial.printf("[fire] LeftShift at %lus (interval=%lum)\n",
                static_cast<unsigned long>(now / 1000),
                static_cast<unsigned long>(interval::minutesAt(intervalIndex)));
}

void logStatus(uint32_t now) {
  char remaining[8];
  view::formatMSS(countdown.remainingMs(now), remaining, sizeof(remaining));
  Serial.printf("[stat] hosts=%u/%u adv=%d bonds=%d interval=%lum remaining=%s\n",
                ble_keyboard::connectedCount(), ble_keyboard::kMaxHosts,
                ble_keyboard::isAdvertising(), ble_keyboard::bondCount(),
                static_cast<unsigned long>(interval::minutesAt(intervalIndex)), remaining);
}

void render(uint32_t now) {
  screen::State s;
  s.connected = countdown.running();
  s.hostCount = ble_keyboard::connectedCount();
  s.maxHosts = ble_keyboard::kMaxHosts;
  s.resetSeconds = static_cast<uint8_t>(gesture.secondsToLong(now));
  s.blinkOn = (now / kBlinkMs) % 2 == 0;
  s.remainingFraction = countdown.remainingFraction(now);
  s.remainingMs = countdown.remainingMs(now);
  s.intervalMinutes = interval::minutesAt(intervalIndex);
  s.inverted = flashing;
  screen::draw(s);
}

}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(pins::kButton, INPUT_PULLUP);
  pinMode(pins::kLed, OUTPUT);
  setLed(false);

  intervalIndex = settings::loadIntervalIndex();
  countdown.setDuration(interval::durationMs(intervalIndex), millis());

  screen::begin(pins::kOledSda, pins::kOledScl);
  ble_keyboard::begin(kDeviceName);

  Serial.printf("[boot] PCCaffeine KBD, interval=%lum\n",
                static_cast<unsigned long>(interval::minutesAt(intervalIndex)));
}

void loop() {
  const uint32_t now = millis();

  button.update(digitalRead(pins::kButton) == LOW, now);
  switch (gesture.update(button.pressed(), now)) {
    case ButtonGesture::Event::Short:
      onButtonPressed(now);
      break;
    case ButtonGesture::Event::Long:
      onPairingReset(now);
      break;
    case ButtonGesture::Event::None:
      break;
  }

  const bool connected = ble_keyboard::isConnected();
  if (connected != wasConnected) {
    wasConnected = connected;
    countdown.setConnected(connected, now);
    onConnectionChanged(connected);
  }

  if (countdown.update(now)) onFire(now);

  if (flashing && static_cast<int32_t>(now - flashUntilMs) >= 0) {
    flashing = false;
    setLed(false);
  }

  if (now - lastFrameMs >= kFrameMs) {
    lastFrameMs = now;
    render(now);
  }

  if (now - lastStatusLogMs >= kStatusLogMs) {
    lastStatusLogMs = now;
    logStatus(now);
  }

  delay(5);
}
