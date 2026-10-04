#pragma once

#include <stdint.h>

// Persists the selected interval index in NVS (spec §4.2).
namespace settings {

uint8_t loadIntervalIndex();
void saveIntervalIndex(uint8_t index);

}  // namespace settings
