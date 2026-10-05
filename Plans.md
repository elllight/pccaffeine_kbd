# pccaffeine_kbd Plans.md

作成日: 2026-10-04

Source request: 2026-10-04 user request (ESP32-C3 0.42" OLED, Shift every interval, pie countdown, 1/5/8 min cycling). Spec: `spec.md`.

---

## Phase 0: Project baseline

Purpose: PlatformIO 개발환경, 포맷/정적검사 baseline, 호스트 테스트 환경 구축

| Task | 内容 | DoD | Depends | Status |
|------|------|-----|---------|--------|
| 0.1 | `[Setup]` `[lane:gate]` `[tdd:skip:scaffold]` git init, `.gitignore`, `CLAUDE.md`, `platformio.ini` (env `esp32c3`: espressif32/arduino, board esp32-c3-devkitm-1, USB CDC on boot, libs NimBLE-Arduino ^2.5.1 + U8g2 ^2.36.18 pinned; env `native`: Unity), `.clang-format`, skeleton `src/main.cpp`, `lib/core/` | `pio run -e esp32c3` exit 0 AND `pio test -e native` exit 0 (placeholder test) AND `clang-format --dry-run --Werror` on project sources exit 0 | - | cc:完了 [3934c98] |

## Phase 1: Core logic (host-tested)

Purpose: 하드웨어 무관 로직을 TDD로 확정 (spec §4.2, §4.3, §4.4, §4.5)

| Task | 内容 | DoD | Depends | Status |
|------|------|-----|---------|--------|
| 1.1 | `[Feature]` `[lane:gate]` `[tdd:required]` `IntervalCycle`: 1→5→8→1 min cycle, index↔minutes, sanitize stored index (invalid → 1 min) | native tests cover cycle wrap, sanitize invalid/out-of-range; `pio test -e native` PASS | 0.1 | cc:完了 [3934c98] |
| 1.2 | `[Feature]` `[lane:gate]` `[tdd:required]` `Countdown` state machine (millis-injected): runs only while connected, resets to full on connect / interval change / after fire, reports `fired` exactly once per expiry, millis() wraparound safe | native tests cover: pause when disconnected, reset on reconnect, reset on interval change, single fire + auto restart, 32-bit wrap; PASS | 0.1 | cc:完了 [3934c98] |
| 1.3 | `[Feature]` `[lane:gate]` `[tdd:required]` `Debouncer` (≥30 ms, press-edge event, active-low) | native tests cover bounce rejection, one event per press, long hold → single event; PASS | 0.1 | cc:完了 [3934c98] |
| 1.4 | `[Feature]` `[lane:gate]` `[tdd:required]` view helpers: remaining fraction → pie sweep (12 o'clock, clockwise shrink), `M:SS` formatting (ceil seconds), interval label | native tests cover 100%/50%/0% sweep, `8:00`,`0:59`,`0:00` formatting; PASS | 0.1 | cc:完了 [3934c98] |

## Phase 2: Firmware modules + integration

Purpose: BLE HID, OLED 렌더링, NVS, 메인 루프 결합

| Task | 内容 | DoD | Depends | Status |
|------|------|-----|---------|--------|
| 2.1 | `[Feature]` `[lane:gate]` `[tdd:skip:hardware-io]` `BleKeyboard` on NimBLE-Arduino 2.x: HID keyboard report map, name `PCCaffeine`, bonding, re-advertise on disconnect, `isConnected()`, `tapLeftShift()` (press ~50 ms → release) | `pio run -e esp32c3` exit 0; code review shows only Left Shift modifier report is ever sent | 0.1 | cc:完了 [3934c98] |
| 2.2 | `[Feature]` `[lane:gate]` `[tdd:skip:hardware-io]` `Screen` renderer with U8g2 72x40 (left pie, right BLE state / `M:SS` / `[Nm]`, waiting state, invert flash on fire) using 1.4 helpers | `pio run -e esp32c3` exit 0 | 1.4 | cc:完了 [3934c98] |
| 2.3 | `[Feature]` `[lane:gate]` `[tdd:skip:hardware-io]` `Settings` NVS (Preferences) load/save interval index via `IntervalCycle::sanitize` | `pio run -e esp32c3` exit 0 | 1.1 | cc:完了 [3934c98] |
| 2.4 | `[Feature]` `[lane:gate]` `[tdd:skip:integration-on-device]` `main.cpp` integration: button→cycle+save+reset, connection→countdown, fire→tapLeftShift+LED+invert, serial log of state changes | `pio run -e esp32c3` exit 0; `pio test -e native` PASS; `pio check -e esp32c3` no high-severity defects in `src/` `lib/` | 1.1, 1.2, 1.3, 2.1, 2.2, 2.3 | cc:完了 [3934c98] |

## Phase 3: On-device verification (보드 연결 필요 — 사용자 노티)

| Task | 内容 | DoD | Depends | Status |
|------|------|-----|---------|--------|
| 3.1 | `[Verify]` `[lane:gate]` `[tdd:skip:manual-hw]` Flash via `pio run -e esp32c3 -t upload`, confirm chip = ESP32-C3, OLED shows waiting screen, serial log boots | upload exit 0; serial log shows boot banner; user confirms screen visible | 2.4 | cc:完了 [fe45d21] |
| 3.2 | `[Verify]` `[lane:gate]` `[tdd:skip:manual-hw]` Pair `PCCaffeine` with host PC, 1-min interval fires Left Shift | serial log shows `fire` at 60 s ±1 s; host observes Shift event (key viewer or idle time reset) | 3.1 | cc:完了 [fe45d21] |
| 3.3 | `[Verify]` `[lane:gate]` `[tdd:skip:manual-hw]` BOOT button cycles 1→5→8→1, reset on change, value restored after power cycle | serial log shows interval sequence; after reboot interval equals last selection | 3.1 | cc:完了 [fe45d21] |

## Phase 4: Review & docs

| Task | 内容 | DoD | Depends | Status |
|------|------|-----|---------|--------|
| 4.1 | `[Docs]` `[lane:fast]` `[tdd:skip:docs-only]` README (build/flash/pair/use, pinout, BOOT-held-at-power-on caveat) | README exists with sections Build, Flash, Pair, Use | 2.4 | cc:完了 [cb8c2db] |
| 4.2 | `[Review]` `[lane:gate]` `[tdd:skip:review]` harness-review of full code against spec.md | review verdict APPROVE or all findings fixed | 2.4, 4.1 | cc:完了 [3bf1da4] |

## Phase 5: Documentation (2026-10-05 user request)

Purpose: 사용자 매뉴얼(그림/다이어그램)과 설계사양서(SADS, drawio→SVG)를 docs/에 작성

| Task | 内容 | DoD | Depends | Status |
|------|------|-----|---------|--------|
| 5.1 | `[Docs]` `[lane:fast]` `[tdd:skip:docs-only]` `docs/user-manual.md`: 구성품/화면 읽는 법/페어링/버튼/문제해결, 그림(SVG 화면 목업, 상태 흐름도) 포함 | 파일 존재, 링크된 모든 이미지 파일 존재, 섹션 6개 이상(개요·화면·페어링·버튼·LED/피드백·문제해결) | 3.3 | cc:完了 [fe45d21] |
| 5.2 | `[Docs]` `[lane:fast]` `[tdd:skip:docs-only]` `docs/sads.md` 설계사양서 + `docs/diagrams/*.drawio` 원본 + `docs/diagrams/*.svg` 내보내기(시스템 컨텍스트, 하드웨어 블록, SW 모듈 구조, 상태머신, 메인루프 시퀀스) | 각 .drawio에 대응하는 .svg 존재하고 sads.md에서 링크됨; drawio CLI export exit 0 | 3.3 | cc:完了 [fe45d21] |

## Phase 6: Multi-host (2026-10-05 user request)

Purpose: 최대 3대 PC 동시 연결, 화면에 현재/최대 연결 수 표시 (spec §3 Multi-host, §4.3, §4.4)

| Task | 内容 | DoD | Depends | Status |
|------|------|-----|---------|--------|
| 6.1 | `[Feature]` `[lane:gate]` `[tdd:required]` core `HostSet`(최대 3, 핸들 추가/제거, 중복 무시, 가득 참 판정) + `view::formatLinkLabel(n,max)` → `BT n/3` | native tests: add/remove/dup/overflow/unknown-remove, 라벨 `BT 0/3`·`BT 3/3`; `pio test -e native` PASS | - | cc:TODO |
| 6.2 | `[Feature]` `[lane:gate]` `[tdd:skip:hardware-io]` `ble_keyboard`: 암호화 링크를 HostSet으로 추적, `connectedCount()` 제공, 3대 미만이면 연결 후에도 광고 재개; build flags MAX_CONNECTIONS=3, MAX_BONDS=5, MAX_CCCDS=16 | `pio run -e esp32c3` 경고 0 | 6.1 | cc:TODO |
| 6.3 | `[Feature]` `[lane:gate]` `[tdd:skip:hardware-io]` main/screen: 상단 `BT n/3`(n=0이면 깜빡임), 연결 수 변화 로그 `[ble] hosts=n/3`, `[stat]`에 `hosts=n/3 adv=0/1` | `pio run` 경고 0, `pio check` HIGH 0, clang-format OK | 6.2 | cc:TODO |
| 6.4 | `[Verify]` `[lane:gate]` `[tdd:skip:manual-hw]` 실기: 1대 연결 시 `BT 1/3` 표시와 `adv=1` 유지, Shift 수신(HIDIdleTime 리셋). 2대 이상 동시 연결은 PC가 추가로 있을 때만 | 시리얼 로그 + 사용자 육안; 다중 PC 미검증 시 `unknown`으로 보고 | 6.3 | cc:TODO |
| 6.5 | `[Docs]` `[lane:fast]` `[tdd:skip:docs-only]` user-manual(여러 PC 연결 절, 화면 `BT n/3`), sads(ADR-8, R-3 해소, 인터페이스), README, 다이어그램 재생성 | 문서 링크 유효, SVG 재내보내기 exit 0 | 6.3 | cc:TODO |
| 6.6 | `[Review]` `[lane:gate]` `[tdd:skip:review]` 독립 리뷰 (HostSet 스레드 안전성, 광고 재개 로직) | verdict APPROVE 또는 지적 사항 반영 | 6.3 | cc:TODO |

## 事前確認
- 事項: PlatformIO 패키지/툴체인 다운로드 (registry.platformio.org, dl.espressif.com) — 외부 수신만, 송신 없음
  理由: `pio run` / `pio test` 최초 실행 시 espressif32 플랫폼·라이브러리 설치 필요
  scope: Phase 0 / Task 0.1
- 事項: destructive — 보드 플래시 덮어쓰기 (`pio run -t upload`)
  理由: 펌웨어 기록 시 기존 보드 펌웨어가 지워짐 (보드 연결 후 사용자 노티 이후에만 실행)
  scope: Phase 3 / Task 3.1–3.3, Phase 6 / Task 6.4
