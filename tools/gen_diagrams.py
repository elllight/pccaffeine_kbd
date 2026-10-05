#!/usr/bin/env python3
"""Generates the draw.io sources in docs/diagrams/.

The .drawio files are the editable originals; SVGs are exported from them with
the draw.io CLI (see tools/export_diagrams.sh). Edit this script or the .drawio
files directly, then re-export.
"""
from html import escape
from pathlib import Path

OUT = Path(__file__).resolve().parent.parent / "docs" / "diagrams"

FONT = "fontFamily=Helvetica;fontSize=13;"
BOX = "rounded=1;whiteSpace=wrap;html=1;arcSize=8;" + FONT
CORE = BOX + "fillColor=#dae8fc;strokeColor=#6c8ebf;"
APP = BOX + "fillColor=#d5e8d4;strokeColor=#82b366;"
DRV = BOX + "fillColor=#fff2cc;strokeColor=#d6b656;"
EXT = BOX + "fillColor=#f5f5f5;strokeColor=#999999;fontColor=#333333;"
HW = BOX + "fillColor=#f8cecc;strokeColor=#b85450;"
NOTE = "text;html=1;align=left;verticalAlign=top;whiteSpace=wrap;" + FONT + "fontSize=12;fontColor=#555555;"
TITLE = "text;html=1;align=left;verticalAlign=middle;fontStyle=1;" + FONT + "fontSize=18;"
GROUP = "rounded=1;whiteSpace=wrap;html=1;arcSize=3;dashed=1;fillColor=none;verticalAlign=top;align=left;spacingLeft=10;spacingTop=4;fontStyle=1;" + FONT
EDGE = "edgeStyle=orthogonalEdgeStyle;rounded=1;html=1;endArrow=block;endFill=1;" + FONT + "fontSize=12;labelBackgroundColor=#ffffff;"
STATE = "rounded=1;whiteSpace=wrap;html=1;arcSize=30;" + FONT
DECISION = "rhombus;whiteSpace=wrap;html=1;" + FONT + "fontSize=12;fillColor=#fff2cc;strokeColor=#d6b656;"
OLED_BG = "rounded=0;html=1;fillColor=#000000;strokeColor=#333333;strokeWidth=3;"
OLED_TXT = "text;html=1;align=left;verticalAlign=top;fontFamily=Courier New;fontStyle=1;fontColor=#ffffff;"
CALLOUT = "text;html=1;align=left;verticalAlign=middle;whiteSpace=wrap;" + FONT + "fontSize=13;"


class Diagram:
    def __init__(self, name, width=1000, height=700):
        self.name, self.width, self.height = name, width, height
        self.cells, self.n = [], 2

    def _id(self):
        self.n += 1
        return f"c{self.n}"

    def box(self, value, x, y, w, h, style=BOX):
        cid = self._id()
        self.cells.append(
            f'<mxCell id="{cid}" value="{escape(value)}" style="{style}" vertex="1" parent="1">'
            f'<mxGeometry x="{x}" y="{y}" width="{w}" height="{h}" as="geometry"/></mxCell>')
        return cid

    def edge(self, src, tgt, value="", style=EDGE, points=()):
        cid = self._id()
        pts = "".join(f'<mxPoint x="{px}" y="{py}"/>' for px, py in points)
        pts = f'<Array as="points">{pts}</Array>' if pts else ""
        self.cells.append(
            f'<mxCell id="{cid}" value="{escape(value)}" style="{style}" edge="1" parent="1" '
            f'source="{src}" target="{tgt}"><mxGeometry relative="1" as="geometry">{pts}</mxGeometry></mxCell>')
        return cid

    def write(self):
        body = "".join(self.cells)
        xml = (f'<mxfile host="drawio"><diagram id="{self.name}" name="{self.name}">'
               f'<mxGraphModel dx="{self.width}" dy="{self.height}" grid="1" gridSize="10" guides="1" '
               f'tooltips="1" connect="1" arrows="1" fold="1" page="1" pageScale="1" '
               f'pageWidth="{self.width}" pageHeight="{self.height}" background="#ffffff" math="0" shadow="0">'
               f'<root><mxCell id="0"/><mxCell id="1" parent="0"/>{body}</root>'
               f'</mxGraphModel></diagram></mxfile>\n')
        (OUT / f"{self.name}.drawio").write_text(xml, encoding="utf-8")


# ---------------------------------------------------------------- OLED mock-up
SCALE = 5  # 72x40 px panel drawn at 5x


def oled(d, x, y, *, remaining, top, time_text, interval, inverted=False, show_pie=True):
    """Draws the 72x40 screen at (x, y) using the same layout as src/screen.cpp."""
    fg, bg = ("#000000", "#ffffff") if inverted else ("#ffffff", "#000000")
    d.box("", x, y, 72 * SCALE, 40 * SCALE, OLED_BG.replace("#000000", bg, 1))
    cx, cy, r = 19 * SCALE, 20 * SCALE, 18 * SCALE
    if show_pie and remaining > 0:
        # draw.io pie: angles are fractions of a turn, clockwise from 12 o'clock.
        style = (f"shape=mxgraph.basic.pie;html=1;startAngle={1 - remaining:.4f};endAngle=1;"
                 f"fillColor={fg};strokeColor=none;")
        if remaining >= 1:
            style = f"ellipse;html=1;fillColor={fg};strokeColor=none;"
        d.box("", x + cx - r, y + cy - r, 2 * r, 2 * r, style)
    d.box("", x + cx - r, y + cy - r, 2 * r, 2 * r, f"ellipse;html=1;fillColor=none;strokeColor={fg};strokeWidth=4;")
    txt = OLED_TXT.replace("#ffffff", fg)
    tx = x + 41 * SCALE
    if top:
        d.box(top, tx, y + 0 * SCALE, 31 * SCALE, 9 * SCALE, txt + "fontSize=26;")
    d.box(time_text, tx, y + 11 * SCALE, 31 * SCALE, 15 * SCALE, txt + "fontSize=40;")
    d.box(interval, tx, y + 30 * SCALE, 31 * SCALE, 9 * SCALE, txt + "fontSize=26;")


def screen_guide():
    d = Diagram("screen-guide", 1000, 360)
    d.box("화면 읽는 법", 20, 10, 400, 40, TITLE)
    ox, oy = 40, 80
    oled(d, ox, oy, remaining=0.70, top="BT 1/3", time_text="3:30", interval="[5m]")
    badge = "ellipse;html=1;fillColor=#e51400;strokeColor=#ffffff;strokeWidth=2;fontColor=#ffffff;fontStyle=1;" + FONT
    for num, bx, by in (("1", ox + 30, oy + 150), ("2", ox + 328, oy + 8), ("3", ox + 330, oy + 60),
                        ("4", ox + 328, oy + 158)):
        d.box(num, bx, by, 28, 28, badge)
    legend = [
        "<b>① 파이차트</b> — 남은 시간 비율. 시계 바늘처럼 12시 방향부터 시계방향으로 비워집니다.",
        "<b>② 연결 수</b> — <b>BT 1/3</b> = 연결된 PC 1대 / 최대 3대. 0/3이면 깜빡임(연결 대기)",
        "<b>③ 남은 시간</b> — 다음 Shift 입력까지 (분:초)",
        "<b>④ 현재 간격</b> — [1m] / [5m] / [8m]",
    ]
    for i, text in enumerate(legend):
        d.box(text, 440, oy + 10 + i * 48, 540, 40, CALLOUT)
    d.write()


def screen_states():
    d = Diagram("screen-states", 1240, 400)
    d.box("화면 상태별 예시", 20, 10, 400, 40, TITLE)
    y = 80
    oled(d, 20, y, remaining=0, top="BT 0/3", time_text="-:--", interval="[1m]", show_pie=False)
    oled(d, 420, y, remaining=0.45, top="BT 2/3", time_text="0:27", interval="[1m]")
    oled(d, 820, y, remaining=1.0, top="BT 2/3", time_text="1:00", interval="[1m]", inverted=True)
    caps = [
        ("<b>연결 대기</b><br>BT 0/3 깜빡임, 시간은 -:--<br>타이머 정지 상태", 20),
        ("<b>카운트다운 중 (PC 2대 연결)</b><br>파이가 시계방향으로 줄어듦", 420),
        ("<b>Shift 입력 순간</b><br>연결된 모든 PC에 동시 전송<br>화면 0.2초 반전 + LED 깜빡임", 820),
    ]
    for text, x in caps:
        d.box(text, x, y + 215, 360, 70, CALLOUT.replace("align=left", "align=center"))
    d.write()


def button_cycle():
    d = Diagram("button-cycle", 760, 420)
    d.box("BOOT 버튼으로 간격 바꾸기", 20, 10, 500, 40, TITLE)
    s = STATE + "fillColor=#dae8fc;strokeColor=#6c8ebf;fontSize=22;fontStyle=1;"
    a = d.box("1분<br><span style='font-size:13px;font-weight:normal'>[1m] · 기본값</span>", 300, 70, 160, 80, s)
    b = d.box("5분<br><span style='font-size:13px;font-weight:normal'>[5m]</span>", 520, 250, 160, 80, s)
    c = d.box("8분<br><span style='font-size:13px;font-weight:normal'>[8m]</span>", 80, 250, 160, 80, s)
    e = EDGE.replace("orthogonalEdgeStyle", "none") + "curved=1;strokeWidth=2;"
    d.edge(a, b, "누르기", e, points=[(600, 110)])
    d.edge(b, c, "누르기", e, points=[(380, 360)])
    d.edge(c, a, "누르기", e, points=[(160, 110)])
    d.box("• 짧게 눌렀다 <b>떼면</b> 다음 간격으로 바뀌고 타이머는 처음부터 시작<br>"
          "• 선택한 간격은 저장되어 전원을 껐다 켜도 유지", 250, 160, 300, 70, NOTE)
    d.write()


def pairing_steps():
    d = Diagram("pairing-steps", 1100, 300)
    d.box("처음 한 번만 하는 블루투스 페어링", 20, 10, 600, 40, TITLE)
    steps = [
        "<b>1. 전원 연결</b><br>USB-C로 PC나 충전기에 연결<br>화면에 BT 0/3 깜빡임",
        "<b>2. PC 블루투스 설정 열기</b><br>Windows: 설정 › Bluetooth 및 장치<br>macOS: 시스템 설정 › Bluetooth",
        "<b>3. PCCaffeine 선택</b><br>장치 목록에서 PCCaffeine 연결<br>(PIN 입력 없음)",
        "<b>4. 연결 확인</b><br>화면에 BT 1/3 표시<br>카운트다운 시작",
    ]
    prev = None
    for i, text in enumerate(steps):
        cur = d.box(text, 20 + i * 270, 90, 230, 120, APP if i == 3 else BOX + "fillColor=#ffffff;strokeColor=#6c8ebf;")
        if prev:
            d.edge(prev, cur, style=EDGE + "strokeWidth=2;")
        prev = cur
    d.box("한 번 페어링하면 이후에는 전원만 넣어도 자동으로 다시 연결됩니다. 다른 PC도 같은 방법으로 최대 3대까지 함께 연결할 수 있습니다.", 20, 230, 1000, 30, NOTE)
    d.write()


def board_overview():
    d = Diagram("board-overview", 900, 460)
    d.box("보드 각 부분", 20, 10, 400, 40, TITLE)
    d.box("", 250, 90, 260, 330, "rounded=1;html=1;arcSize=6;fillColor=#1b4f8a;strokeColor=#0d2b4d;strokeWidth=2;")
    usb = d.box("USB-C", 330, 395, 100, 40, "rounded=1;html=1;fillColor=#c0c0c0;strokeColor=#666666;" + FONT)
    scr = d.box("", 290, 120, 180, 100, OLED_BG)
    chip = d.box("ESP32-C3", 320, 250, 120, 70, "rounded=0;html=1;fillColor=#333333;fontColor=#ffffff;strokeColor=#111111;" + FONT)
    boot = d.box("BOOT", 270, 340, 60, 36, "rounded=1;html=1;fillColor=#eeeeee;strokeColor=#333333;" + FONT + "fontSize=11;")
    rst = d.box("RST", 430, 340, 60, 36, "rounded=1;html=1;fillColor=#eeeeee;strokeColor=#333333;" + FONT + "fontSize=11;")
    led = d.box("", 460, 255, 16, 16, "ellipse;html=1;fillColor=#3399ff;strokeColor=#003366;")
    callouts = [
        (scr, "0.42\" OLED (72×40)<br>동작 상태 표시", 560, 130),
        (led, "LED<br>Shift 입력 시 깜빡임", 560, 230),
        (rst, "RST 버튼<br>재시작", 560, 330),
        (boot, "BOOT 버튼<br>짧게: 간격 변경 · 5초: 페어링 초기화", 20, 330),
        (chip, "ESP32-C3<br>블루투스 LE 내장", 20, 250),
        (usb, "USB-C<br>전원 / 펌웨어 업로드", 20, 400),
    ]
    for target, text, x, y in callouts:
        lab = d.box(text, x, y, 210, 50, CALLOUT)
        d.edge(lab, target, style="endArrow=oval;endFill=1;html=1;strokeColor=#e51400;")
    d.box("※ 실제 보드의 부품 위치는 제조사마다 조금 다를 수 있습니다.", 20, 445, 600, 20, NOTE)
    d.height = 480
    d.write()


# ---------------------------------------------------------------- SADS diagrams
def multi_host():
    d = Diagram("multi-host", 1000, 400)
    d.box("여러 PC에 동시에 연결하기 (최대 3대)", 20, 10, 600, 40, TITLE)
    dev = d.box("", 60, 110, 72 * 3, 40 * 3, OLED_BG)
    txt = OLED_TXT + "fontSize=20;"
    d.box("BT 3/3", 60 + 41 * 3, 112, 31 * 3, 30, txt)
    d.box("2:14", 60 + 41 * 3, 145, 31 * 3, 40, txt + "fontSize=30;")
    d.box("[5m]", 60 + 41 * 3, 195, 31 * 3, 30, txt)
    d.box("", 60 + 3, 110 + 3, 108, 108, "shape=mxgraph.basic.pie;html=1;startAngle=0.55;endAngle=1;fillColor=#ffffff;strokeColor=none;")
    d.box("", 60 + 3, 110 + 3, 108, 108, "ellipse;html=1;fillColor=none;strokeColor=#ffffff;strokeWidth=3;")
    names = ["회사 PC (Windows)", "개인 노트북 (macOS)", "회의실 PC"]
    for i, n in enumerate(names):
        pc = d.box(f"<b>{n}</b>", 640, 80 + i * 90, 300, 60, BOX + "fillColor=#ffffff;strokeColor=#6c8ebf;")
        d.edge(dev, pc, "Shift" if i == 1 else "", EDGE.replace("orthogonalEdgeStyle", "none") + "strokeWidth=2;")
    d.box("• 각 PC에서 한 번씩 페어링하면 됩니다. 연결 중에도 다른 PC가 장치를 찾을 수 있습니다.<br>"
          "• Shift는 연결된 <b>모든 PC에 같은 순간</b> 전송됩니다. 타이머는 하나입니다.<br>"
          "• 3대가 모두 연결되면 더 이상 보이지 않습니다. 한 대가 끊기면 다시 보입니다.", 60, 300, 900, 70, NOTE)
    d.write()


def button_gestures():
    d = Diagram("button-gestures", 1000, 380)
    d.box("BOOT 버튼 사용법 (누르고 있는 시간)", 20, 10, 600, 40, TITLE)
    axis_y = 150
    d.box("", 80, axis_y, 840, 2, "line;html=1;strokeWidth=2;strokeColor=#333333;")
    for sec, x in ((0, 80), (1, 248), (5, 920)):
        d.box(f"{sec}초", x - 20, axis_y + 10, 40, 20, NOTE + "align=center;")
    d.box("<b>짧게 누르기</b><br>떼는 순간 간격 변경<br>1분 → 5분 → 8분", 80, 70, 168, 70, BOX + "fillColor=#dae8fc;strokeColor=#6c8ebf;fontSize=12;")
    d.box("<b>취소 구간</b><br>이때 떼면 아무 일도 없음<br>화면에 RST 4 → 3 → 2 → 1", 248, 70, 672, 70, BOX + "fillColor=#f5f5f5;strokeColor=#999999;fontSize=12;")
    d.box("<b>5초가 되는 순간 페어링 초기화</b><br>모든 PC 연결 끊김 · 저장된 페어링 삭제<br>화면 반전 후 BT 0/3 · 간격 설정은 유지",
          640, 200, 340, 80, BOX + "fillColor=#f8cecc;strokeColor=#b85450;fontSize=12;")
    d.box("", 914, axis_y - 6, 14, 14, "ellipse;html=1;fillColor=#b85450;strokeColor=none;")
    d.box("초기화 후에는 각 PC의 블루투스 설정에서 PCCaffeine을 <b>기기 삭제</b>한 뒤 다시 연결하세요.", 80, 300, 840, 30, NOTE)
    d.write()


def system_context():
    d = Diagram("system-context", 1040, 480)
    d.box("시스템 컨텍스트", 20, 10, 400, 40, TITLE)
    dev = d.box("<b>PCCaffeine KBD</b><br>ESP32-C3 + 0.42\" OLED<br>BLE HID 키보드", 380, 190, 220, 100, APP)
    pcs = [d.box(f"<b>Host PC {i + 1}</b><br>Windows / macOS / Linux", 760, 90 + i * 110, 240, 70, EXT) for i in range(3)]
    pwr = d.box("<b>USB 전원</b><br>PC 포트 또는 충전기", 380, 380, 220, 60, EXT)
    usr = d.box("<b>사용자</b>", 60, 205, 160, 70, EXT)
    for i, pc in enumerate(pcs):
        d.edge(dev, pc, "BLE HID · Left Shift" if i == 1 else "", EDGE.replace("orthogonalEdgeStyle", "none"))
    d.edge(pwr, dev, "5V 전원<br>(플래시·로그: USB-Serial-JTAG)")
    d.edge(usr, dev, "BOOT: 간격 변경 / 5초 초기화<br>OLED: 상태 확인")
    d.box("최대 3대 동시 연결 · 1/5/8분마다 모든 PC에 동시 전송<br>ESP32-C3에는 USB OTG가 없어 USB HID는 불가 → BLE만 사용", 380, 70, 340, 50, NOTE)
    d.write()


def hardware_block():
    d = Diagram("hardware-block", 1060, 560)
    d.box("하드웨어 블록도", 20, 10, 400, 40, TITLE)
    mcu = d.box("<b>ESP32-C3</b><br>RISC-V 160 MHz · 400 KB SRAM<br>4 MB 내장 플래시 (앱 + NVS)", 480, 110, 260, 280, HW)
    oled_ = d.box("<b>OLED SSD1306</b><br>72×40, I2C 0x3C", 40, 120, 200, 70, DRV)
    btn = d.box("<b>BOOT 버튼</b><br>active-low, 외부 풀업<br>부팅 strap 핀", 40, 210, 200, 80, DRV)
    led = d.box("<b>LED</b><br>active-low", 40, 320, 200, 60, DRV)
    usb = d.box("<b>USB-C</b><br>USB-Serial-JTAG<br>전원 · 플래시 · 로그", 490, 450, 240, 80, DRV)
    rf = d.box("<b>2.4 GHz 안테나</b><br>BLE 5", 820, 215, 200, 70, DRV)
    straight = EDGE.replace("orthogonalEdgeStyle", "none")
    both = straight + "startArrow=block;startFill=1;"
    d.edge(mcu, oled_, "I2C · SDA=GPIO5, SCL=GPIO6", both + "exitX=0;exitY=0.196;entryX=1;entryY=0.5;")
    d.edge(btn, mcu, "GPIO9 (INPUT_PULLUP)", straight + "exitX=1;exitY=0.5;entryX=0;entryY=0.5;")
    d.edge(mcu, led, "GPIO8 (OUTPUT)", straight + "exitX=0;exitY=0.857;entryX=1;entryY=0.5;")
    d.edge(usb, mcu, "GPIO18/19 (D-/D+)", both)
    d.edge(mcu, rf, "RF", both)
    d.write()


def software_architecture():
    d = Diagram("software-architecture", 1080, 720)
    d.box("소프트웨어 모듈 구조", 20, 10, 400, 40, TITLE)
    main = d.box("<b>main.cpp</b> (Application)<br>setup() / loop() 조립 · 이벤트 로깅", 380, 60, 300, 60, APP)
    core_g = d.box("Core (lib/core)<br><span style='font-weight:normal;font-size:11px'>순수 로직 · PC 단위 테스트</span>",
                   20, 160, 320, 520, GROUP + "strokeColor=#6c8ebf;")
    adp_g = d.box("Adapters (src/)<br><span style='font-weight:normal;font-size:11px'>하드웨어 의존</span>",
                  380, 160, 300, 330, GROUP + "strokeColor=#d6b656;")
    ext_g = d.box("Third-party", 740, 160, 320, 330, GROUP + "strokeColor=#999999;")
    rows = (220, 310, 400)
    ble = d.box("<b>ble_keyboard</b><br>HID · 최대 3대 · 광고 유지<br>tapLeftShift() · clearPairings()", 400, rows[0], 260, 70, DRV)
    st = d.box("<b>settings</b><br>NVS 간격 인덱스 load/save", 400, rows[1], 260, 70, DRV)
    scr = d.box("<b>screen</b><br>72×40 렌더링 (파이, BT n/3, RST n)", 400, rows[2], 260, 70, DRV)
    nim = d.box("NimBLE-Arduino 2.5.1", 760, rows[0] + 12, 280, 46, EXT)
    pref = d.box("Preferences (arduino-esp32 2.0.x)", 760, rows[1] + 12, 280, 46, EXT)
    u8 = d.box("U8g2 2.36.18 + Wire", 760, rows[2] + 12, 280, 46, EXT)
    hs = d.box("<b>HostSet</b> · <b>PendingLinks</b><br>페어링된 호스트(최대 3) · 30 s 미페어링", 40, rows[0], 280, 70, CORE)
    iv = d.box("<b>interval_cycle</b><br>1→5→8분, sanitize", 40, rows[1], 280, 70, CORE)
    vm = d.box("<b>view_math</b><br>파이 각도, M:SS, BT n/3", 40, rows[2], 280, 70, CORE)
    d.box("<b>Countdown</b><br>연결 중에만 진행, 1회 fire", 40, 490, 280, 50, CORE)
    d.box("<b>Debouncer</b> → <b>ButtonGesture</b><br>짧게 / 취소 / 5초 길게", 40, 550, 280, 56, CORE)
    d.box("main.cpp가 Countdown · Debouncer · ButtonGesture · interval_cycle을 직접 사용", 40, 615, 280, 50, NOTE)
    dep = EDGE.replace("orthogonalEdgeStyle", "none") + "dashed=1;endArrow=open;endFill=0;"
    d.edge(main, adp_g, style=dep)
    d.edge(main, core_g, "", EDGE + "dashed=1;endArrow=open;endFill=0;exitX=0;exitY=0.5;entryX=0.5;entryY=0;",
           points=[(180, 90)])
    for a, b in ((ble, nim), (st, pref), (scr, u8), (ble, hs), (st, iv), (scr, vm)):
        d.edge(a, b, style=dep)
    d.box("점선 화살표 = 의존(include) 방향. lib/core는 Arduino 헤더를 쓰지 않아 PC에서 테스트됩니다.", 380, 510, 680, 40, NOTE)
    d.write()


def state_machine():
    d = Diagram("state-machine", 1100, 560)
    d.box("동작 상태 머신 (연결된 호스트 수 n 기준)", 20, 10, 600, 40, TITLE)
    init = d.box("", 20, 111, 24, 24, "ellipse;html=1;fillColor=#000000;")
    wait = d.box("<b>Waiting</b> (n = 0)<br>광고 중 · 타이머 정지<br>화면: BT 0/3 깜빡임", 160, 80, 220, 86, STATE + "fillColor=#f5f5f5;strokeColor=#666666;")
    link = d.box("<b>Linked</b><br>GAP 연결됨<br>페어링·암호화 진행", 520, 80, 220, 86, STATE + "fillColor=#fff2cc;strokeColor=#d6b656;")
    count = d.box("<b>Counting</b> (n ≥ 1)<br>타이머 진행, n &lt; 3이면 광고 유지<br>화면: BT n/3 · M:SS", 520, 300, 220, 86, STATE + "fillColor=#dae8fc;strokeColor=#6c8ebf;")
    fire = d.box("<b>Fire</b><br>연결된 모든 PC에 Left Shift<br>LED + 화면 반전 200 ms", 860, 300, 220, 86, STATE + "fillColor=#d5e8d4;strokeColor=#82b366;")
    d.edge(init, wait, "부팅")
    d.edge(wait, link, "호스트 연결")
    d.edge(link, count, "암호화 완료 · n: 0→1<br>타이머 = full")
    d.edge(count, fire, "남은 시간 = 0")
    d.edge(fire, count, "타이머 = full 재시작", EDGE + "exitX=0.5;exitY=1;entryX=0.75;entryY=1;",
           points=[(970, 450), (685, 450)])
    d.edge(count, wait, "n: 1→0<br>(마지막 PC 끊김)", EDGE + "exitX=0;exitY=0.5;entryX=0.5;entryY=1;",
           points=[(270, 343)])
    d.edge(link, wait, "연결 끊김", EDGE + "exitX=0.5;exitY=0;entryX=0.5;entryY=0;",
           points=[(630, 50), (270, 50)])
    d.edge(count, count, "짧게: 다음 간격 · 저장 · 타이머 = full<br>PC 추가/해제(n ≥ 1 유지): 타이머 유지",
           EDGE + "exitX=0.25;exitY=1;entryX=0.4;entryY=1;", points=[(575, 420), (608, 420)])
    d.edge(wait, wait, "짧게: 다음 간격 저장", EDGE + "exitX=0;exitY=0.8;entryX=0.25;entryY=1;",
           points=[(130, 149), (130, 200), (215, 200)])
    d.box("<b>BOOT 5초 (어느 상태에서나)</b>: 광고 정지 → 모든 연결 끊기 → 본딩 전체 삭제 → 광고 재개 → Waiting", 20, 480, 900, 24, NOTE + "fontColor=#b85450;")
    d.box("부팅 시 NVS에서 마지막 간격을 읽어 Waiting으로 시작합니다.", 20, 510, 700, 24, NOTE)
    d.write()


def main_loop():
    d = Diagram("main-loop", 900, 1100)
    d.box("loop() 처리 흐름 (약 5 ms 주기)", 20, 10, 500, 40, TITLE)
    p = BOX + "fillColor=#ffffff;strokeColor=#333333;"
    start = d.box("loop() 시작<br>now = millis()", 330, 60, 200, 50, STATE + "fillColor=#f5f5f5;")
    b1 = d.box("버튼 제스처?<br>(Debouncer→Gesture)", 330, 140, 200, 80, DECISION)
    a1 = d.box("짧게: 다음 간격·NVS·타이머 리셋<br>5초: clearPairings()", 620, 145, 250, 70, p)
    b2 = d.box("호스트 0↔1 이상<br>바뀜?", 330, 250, 200, 80, DECISION)
    a2 = d.box("Countdown.setConnected()<br>[ble] 로그", 620, 260, 250, 60, p)
    b3 = d.box("Countdown.update()<br>== fired?", 330, 360, 200, 90, DECISION)
    a3 = d.box("tapLeftShift() → 모든 PC<br>LED ON · 반전 시작", 620, 375, 250, 60, p)
    b4 = d.box("반전 200 ms<br>지남?", 330, 480, 200, 80, DECISION)
    a4 = d.box("LED OFF · 반전 해제", 620, 495, 250, 50, p)
    b5 = d.box("100 ms 지남?", 330, 590, 200, 70, DECISION)
    a5 = d.box("screen::draw()", 620, 600, 250, 50, p)
    b6 = d.box("1 s 지남?", 330, 690, 200, 70, DECISION)
    a6 = d.box("ble_keyboard::maintain()<br>광고 꺼졌고 n &lt; 3이면 재개", 620, 695, 250, 60, p)
    b7 = d.box("10 s 지남?", 330, 790, 200, 70, DECISION)
    a7 = d.box("[stat] 상태 로그", 620, 800, 250, 50, p)
    end = d.box("delay(5)", 330, 890, 200, 50, STATE + "fillColor=#f5f5f5;")
    decisions = (b1, b2, b3, b4, b5, b6, b7)
    seq = [start, *decisions, end]
    for x, y in zip(seq, seq[1:]):
        d.edge(x, y, "아니오" if x in decisions else "")
    pairs = ((b1, a1, b2), (b2, a2, b3), (b3, a3, b4), (b4, a4, b5), (b5, a5, b6), (b6, a6, b7), (b7, a7, end))
    for dec, act, nxt in pairs:
        d.edge(dec, act, "예", EDGE + "exitX=1;exitY=0.5;entryX=0;entryY=0.5;")
        d.edge(act, nxt, style=EDGE + "dashed=1;exitX=0.5;exitY=1;entryX=1;entryY=0.5;" if nxt is end
               else EDGE + "dashed=1;exitX=0.5;exitY=1;entryX=0.75;entryY=0.25;")
    d.edge(end, start, "반복", points=[(250, 915), (250, 85)])
    d.write()


def fire_sequence():
    d = Diagram("fire-sequence", 1100, 520)
    d.box("키 전송 시퀀스", 20, 10, 400, 40, TITLE)
    names = ["loop()", "Countdown", "ble_keyboard", "NimBLE stack", "Host PC ×n"]
    xs = [60, 280, 500, 720, 940]
    life = "line;html=1;strokeColor=#999999;dashed=1;direction=south;"
    for n, x in zip(names, xs):
        d.box(f"<b>{n}</b>", x, 70, 140, 40, BOX + "fillColor=#dae8fc;strokeColor=#6c8ebf;")
        d.box("", x + 69, 110, 2, 380, life)
    msg = "endArrow=block;endFill=1;html=1;" + FONT + "fontSize=12;labelBackgroundColor=#ffffff;"
    ret = msg + "dashed=1;endArrow=open;endFill=0;"
    rows = [
        (0, 1, "update(now)", 150, msg), (1, 0, "true (만료, 타이머 재시작)", 190, ret),
        (0, 2, "tapLeftShift()", 230, msg), (2, 3, "notify([0x02,0,0,0,0,0,0,0])", 270, msg),
        (3, 4, "구독한 모든 PC: Left Shift 누름", 300, msg), (2, 2, "delay(50 ms)", 330, msg),
        (2, 3, "notify([0,0,0,0,0,0,0,0])", 370, msg), (3, 4, "구독한 모든 PC: 뗌", 400, msg),
        (0, 0, "LED ON · 화면 반전 200 ms", 450, msg),
    ]
    for a, b, label, y, st in rows:
        x1, x2 = xs[a] + 70, xs[b] + 70
        if a == b:
            d.box(label, x1 + 10, y - 12, 220, 24, NOTE + "fontColor=#333333;")
            continue
        s = d.box("", x1, y, 1, 1, "point;html=1;strokeColor=none;fillColor=none;")
        t = d.box("", x2, y, 1, 1, "point;html=1;strokeColor=none;fillColor=none;")
        d.edge(s, t, label, st)
    d.box("호스트는 Shift 입력을 사용자 활동으로 간주 → idle 타이머가 초기화되어 화면보호기가 켜지지 않음", 60, 480, 900, 30, NOTE)
    d.height = 540
    d.write()


if __name__ == "__main__":
    OUT.mkdir(parents=True, exist_ok=True)
    for fn in (screen_guide, screen_states, button_cycle, pairing_steps, board_overview,
               multi_host, button_gestures,
               system_context, hardware_block, software_architecture, state_machine,
               main_loop, fire_sequence):
        fn()
    print("\n".join(sorted(p.name for p in OUT.glob("*.drawio"))))
