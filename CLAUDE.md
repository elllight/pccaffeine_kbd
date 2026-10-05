# pccaffeine_kbd

Anti-screensaver BLE keyboard firmware for an ESP32-C3 board with a 0.42" (72x40) OLED.
Product contract: `spec.md` (SSOT). Task ledger: `Plans.md`.

## Layout
- `lib/core/` — hardware-independent logic (interval cycle, countdown, debouncer, button gesture, host set, view math). Host-tested.
- `src/` — Arduino firmware: `ble_keyboard` (NimBLE HID, up to 3 hosts, pairing reset), `screen` (U8g2), `settings` (NVS), `main.cpp`.
- `docs/` — user manual, SADS, draw.io diagrams + exported SVG (`tools/gen_diagrams.py`, `tools/export_diagrams.sh`).
- `test/test_*/` — Unity tests for `lib/core`, run on host.

## Commands
- Build: `pio run -e esp32c3`
- Unit tests: `pio test -e native`
- Flash + monitor: `pio run -e esp32c3 -t upload && pio device monitor -e esp32c3`
- Static check: `pio check -e esp32c3 --skip-packages`
- Format: `clang-format -i src/* lib/core/src/* test/*/*.cpp`

## Rules
- Keep logic that can be tested on host in `lib/core` (no Arduino headers there).
- Only Left Shift may ever be sent to the host (spec §4.1).
- ESP32-C3 has no USB OTG: USB HID is not an option.
- Max hosts is tied together by static_asserts (`kMaxHosts`, `HostSet::kCapacity`, `CONFIG_BT_NIMBLE_MAX_CONNECTIONS`).
- macOS: after upload, replug USB to boot (RTS/DTR reset can leave the chip in download mode).
- When behaviour changes, update `spec.md` first, then `docs/sads.md`, `docs/user-manual.md`, README and diagrams.
