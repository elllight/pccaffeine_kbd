# PCCaffeine KBD — Product Spec

Last updated: 2026-10-04

## 1. Purpose

A tiny BLE keyboard that presses **Left Shift once every N minutes** so the host PC never
enters its screen saver / lock-by-idle. It does nothing else.

## 2. Hardware (fixed)

| Item | Value |
|------|-------|
| Board | ESP32-C3 SuperMini-type board with 0.42" OLED (AliExpress item 1005007929382296) |
| MCU | ESP32-C3 (RISC-V, BLE 5, **no USB OTG → USB HID impossible**) |
| Display | SSD1306 72x40, I2C, SDA=GPIO5, SCL=GPIO6 (U8g2 `SSD1306_72X40_ER`) |
| Button | On-board BOOT button, GPIO9, active-low |
| LED | On-board LED, GPIO8 (active-low) |
| Power | USB-C (power only; serial/JTAG for flashing and logs) |

Pin numbers are the common values for this board; they are verified on real hardware in Phase 3
(`unknown` until then).

## 3. Host link

- Transport: **Bluetooth LE HID keyboard** (HOGP). Device name: **`PCCaffeine`**.
- Bonding enabled so the PC reconnects automatically after pairing once.
- Re-advertises automatically after disconnect.
- "Connected" in this spec means a **paired host on an encrypted link** (pairing/encryption
  complete), not a raw GAP link — before that the host has not subscribed and keystrokes are lost.

## 4. Behaviour

### 4.1 Keystroke
- On timer expiry: send HID report with **Left Shift** modifier pressed, hold ~50 ms, send release.
- No other keys are ever sent.

### 4.2 Intervals
- Allowed intervals cycle: **1 min → 5 min → 8 min → 1 min …**
- Short press of BOOT button advances to the next interval.
- On interval change the countdown **resets immediately** to the full new interval.
- Selected interval is **persisted in NVS** and restored at boot. Invalid/missing stored value → 1 min.
- Factory default (first boot): 1 min.

### 4.3 Countdown and connection
- Countdown runs **only while a BLE host is connected**.
- When not connected: timer is paused, display shows a pairing/waiting state.
- When a connection is (re)established: countdown starts from the full interval.
- After each keystroke the countdown restarts from the full interval (periodic repeat).

### 4.4 Display (72x40)
- Left: pie chart (~36 px diameter). Filled sector = remaining fraction. Elapsed time
  eats the pie clockwise from 12 o'clock like a clock hand (at 50 % the right half is empty).
- Right column (top → bottom):
  - BLE state: connected / advertising (blinking while waiting)
  - Remaining time `M:SS`
  - Current interval `[1m]` / `[5m]` / `[8m]`
- On keystroke: brief visual feedback (display invert ~200 ms) and LED blink.
- While disconnected: pie shown empty/outline and text shows waiting state.

### 4.5 Button
- Debounced (≥ 30 ms). One action per press (on press edge).
- Holding the button during power-on enters ROM download mode (ESP32-C3 hardware behaviour; not a bug).

## 5. Non-goals
- USB HID (impossible on ESP32-C3).
- Mouse jiggle, other keys, configuration over BLE/serial, battery operation.

## 6. Quality gates
- Pure logic (interval cycle, countdown state machine, debouncer, pie geometry, time format)
  lives in `lib/core` and is unit-tested on host: `pio test -e native` must pass.
- Firmware build: `pio run -e esp32c3` must succeed with zero warnings from project sources.
- Formatting: `.clang-format` baseline; static check `pio check -e esp32c3` with no high-severity defects in project sources.
