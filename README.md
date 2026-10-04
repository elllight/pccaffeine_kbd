# PCCaffeine KBD

화면보호기/잠금 진입을 막기 위해 **정해진 간격마다 Left Shift를 한 번 누르는 BLE 키보드**입니다.
ESP32-C3 + 0.42" OLED(72x40) 보드용 PlatformIO 펌웨어입니다. 제품 사양은 [spec.md](spec.md)를 참고하세요.

> ESP32-C3에는 USB OTG가 없어서 USB HID 키보드로는 동작할 수 없습니다. 호스트와는 **블루투스(BLE)** 로 연결하고, USB는 전원 공급과 플래시에만 씁니다.

## 하드웨어

| 기능 | 핀 |
|------|----|
| OLED SDA / SCL (SSD1306 72x40, I2C) | GPIO5 / GPIO6 |
| 버튼 (온보드 BOOT, active-low) | GPIO9 |
| LED (active-low) | GPIO8 |

## Build

```sh
uv tool install platformio      # 또는 pipx install platformio
pio run -e esp32c3              # 펌웨어 빌드
pio test -e native              # 호스트 단위 테스트 (lib/core)
```

## Flash

1. USB-C **데이터 케이블**로 보드를 연결합니다. macOS에서는 `/dev/cu.usbmodem*` 포트로 보입니다.
2. 업로드 후 시리얼 로그를 확인합니다.

```sh
pio run -e esp32c3 -t upload
pio device monitor -e esp32c3
```

포트가 보이지 않거나 업로드가 실패하면 **BOOT 버튼을 누른 채 RESET 버튼을 눌렀다 떼고**, 그다음 BOOT 버튼을 떼서 다운로드 모드로 진입한 뒤 다시 업로드하세요.

## Pair

1. 전원을 넣으면 화면에 `PAIR`가 깜빡이고 시간은 `-:--`로 표시됩니다.
2. PC의 블루투스 설정에서 **`PCCaffeine`** 을 찾아 연결합니다(PIN 입력 없음).
3. 연결되면 `BT ON`이 표시되고 카운트다운이 시작됩니다. 한 번 페어링하면 이후에는 자동으로 재연결됩니다.

## Use

- **파이차트**: 남은 시간 비율입니다. 시계 바늘처럼 12시 방향부터 시계방향으로 줄어듭니다.
- **`M:SS`**: 다음 키 입력까지 남은 시간입니다.
- **`[1m]` / `[5m]` / `[8m]`**: 현재 간격입니다.
- 시간이 0이 되면 Left Shift를 1회 누르고, 화면이 잠깐 반전되며 LED가 깜빡인 뒤 다시 처음부터 카운트합니다.
- **BOOT 버튼 짧게 누르기**: 1분 → 5분 → 8분 → 1분 … 순으로 바뀌며, 카운트다운은 새 간격으로 즉시 다시 시작됩니다.
- 선택한 간격은 저장되어 전원을 껐다 켜도 유지됩니다.
- 블루투스 연결이 끊기면 타이머는 멈추고, 다시 연결되면 처음부터 시작합니다.

> 주의: **전원을 넣는 순간 BOOT 버튼을 누르고 있으면** 칩이 다운로드 모드로 들어가 펌웨어가 실행되지 않습니다(ESP32-C3 하드웨어 동작). 이때는 버튼을 떼고 RESET을 누르세요.

## 구조

- `lib/core/` — 하드웨어와 무관한 로직(간격 순환, 카운트다운, 디바운서, 파이/시간 계산). 호스트에서 테스트합니다.
- `src/` — `ble_keyboard`(NimBLE HID), `screen`(U8g2), `settings`(NVS), `main.cpp`
- `test/` — Unity 단위 테스트
