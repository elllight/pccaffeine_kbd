#include "settings.h"

#include <Arduino.h>
#include <Preferences.h>

#include "interval_cycle.h"

namespace settings {

namespace {
constexpr const char* kNamespace = "pccaffeine";
constexpr const char* kKeyInterval = "interval";
}  // namespace

uint8_t loadIntervalIndex() {
  Preferences prefs;
  if (!prefs.begin(kNamespace, true)) return interval::kDefaultIndex;
  const uint8_t raw = prefs.getUChar(kKeyInterval, interval::kDefaultIndex);
  prefs.end();
  return interval::sanitize(raw);
}

void saveIntervalIndex(uint8_t index) {
  Preferences prefs;
  if (!prefs.begin(kNamespace, false)) {
    Serial.println("[nvs] open failed, interval not saved");
    return;
  }
  prefs.putUChar(kKeyInterval, interval::sanitize(index));
  prefs.end();
}

}  // namespace settings
