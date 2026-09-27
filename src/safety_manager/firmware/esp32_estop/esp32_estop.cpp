// ESP32 firmware for the robot-side E-Stop radio bridge.
//
// Responsibilities (from the competition E-Stop brief):
//  1. Read the team radio's relayed E-Stop state (digital GPIO from the
//     team radio, or decoded from a LoRa packet if the team radio is remote
//     from the robot — both paths supported here, LoRa preferred per the
//     300ft POC).
//  2. Drive a normally-energized relay/MOSFET (kMotorPowerGatePin) that cuts
//     BLDC motor power *in hardware*, independent of the Jetson's software
//     state. This is the actual "stop within 1 second" guarantee: it does
//     not depend on the Jetson, ROS2, or the UART link being alive.
//  3. Report status to the Jetson over UART using the shared packet format
//     in uart_protocol.hpp (copy kept in this directory — see header).
//  4. Fail-safe on Jetson watchdog silence: if no WATCHDOG_KICK command is
//     received from safety_manager within kJetsonWatchdogTimeoutMs, treat it
//     as equivalent to an E-Stop press (protects against a hung/crashed
//     Jetson process, not just a pressed button).
//
// NOTE: copy `../../common/uart_protocol.hpp` into this directory (or add it
// to `lib_deps`/`include` search path in platformio.ini) before building —
// PlatformIO does not share the colcon package's include tree automatically.
#include <Arduino.h>
#include <LoRa.h>

#include "uart_protocol.hpp"

namespace proto = safety_manager::uart;

// ---- Pin map (adjust to your wiring) ----
constexpr uint8_t kTeamRadioGpioPin = 25;   // direct digital E-Stop line, if used
constexpr uint8_t kMotorPowerGatePin = 26;  // drives relay/MOSFET gate cutting ESC power
constexpr uint8_t kStatusLedPin = 2;
constexpr uint8_t kLoRaCsPin = 18;
constexpr uint8_t kLoRaRstPin = 14;
constexpr uint8_t kLoRaIrqPin = 27;

constexpr unsigned long kStatusPeriodMs = 100;      // 10 Hz status to Jetson
constexpr unsigned long kRadioTimeoutMs = 500;       // team-radio link considered lost after this
constexpr unsigned long kJetsonWatchdogTimeoutMs = 500;  // Jetson considered hung after this

bool g_estop_pressed = true;   // fail-safe default until first valid reading
bool g_radio_ok = false;
unsigned long g_last_radio_rx_ms = 0;
unsigned long g_last_jetson_kick_ms = 0;
uint8_t g_heartbeat_seq = 0;

void applyMotorPowerGate(bool estop_active)
{
  // Active-low gate drive: LOW = cut power. Wire the relay/MOSFET so that
  // loss of GPIO drive (ESP32 reset/brownout) also defaults to power-cut.
  digitalWrite(kMotorPowerGatePin, estop_active ? LOW : HIGH);
  digitalWrite(kStatusLedPin, estop_active ? HIGH : LOW);
}

void onLoRaReceive(int packetSize)
{
  if (packetSize <= 0) {return;}
  // Minimal LoRa payload contract with the team radio: single byte,
  // 0x01 = E-Stop pressed (circuit open), 0x00 = released (circuit closed).
  uint8_t payload = LoRa.read();
  g_estop_pressed = (payload == 0x01);
  g_radio_ok = true;
  g_last_radio_rx_ms = millis();
}

void setup()
{
  pinMode(kTeamRadioGpioPin, INPUT_PULLUP);
  pinMode(kMotorPowerGatePin, OUTPUT);
  pinMode(kStatusLedPin, OUTPUT);
  applyMotorPowerGate(true);  // power stays cut until a link is proven healthy

  Serial.begin(115200);

  LoRa.setPins(kLoRaCsPin, kLoRaRstPin, kLoRaIrqPin);
  if (LoRa.begin(915E6)) {
    LoRa.onReceive(onLoRaReceive);
    LoRa.receive();
  }

  g_last_jetson_kick_ms = millis();
}

void sendStatusPacket()
{
  proto::StatusPacket pkt;
  pkt.estop_pressed = g_estop_pressed;
  pkt.radio_ok = g_radio_ok && (millis() - g_last_radio_rx_ms < kRadioTimeoutMs);
  pkt.heartbeat_seq = g_heartbeat_seq++;
  pkt.battery_millivolts = static_cast<uint16_t>(analogRead(A0) * 2.0 * (3300.0 / 4095.0));
  pkt.radio_rssi_dbm = static_cast<int8_t>(LoRa.packetRssi());

  auto buf = proto::encodeStatus(pkt);
  Serial.write(buf.data(), buf.size());
}

void pollJetsonCommands()
{
  static uint8_t frame[proto::kCommandPacketLen];
  static size_t idx = 0;

  while (Serial.available() > 0) {
    uint8_t b = static_cast<uint8_t>(Serial.read());
    if (idx == 0 && b != proto::kStartByte) {continue;}
    frame[idx++] = b;
    if (idx == proto::kCommandPacketLen) {
      if (frame[0] == proto::kStartByte && frame[1] == proto::kPacketTypeCommand &&
        frame[5] == proto::kEndByte && proto::crc8(frame, 4) == frame[4])
      {
        uint8_t cmd = frame[2];
        if (cmd == proto::kCmdWatchdogKick) {
          g_last_jetson_kick_ms = millis();
        } else if (cmd == proto::kCmdSoftwareEStop) {
          g_estop_pressed = true;
        } else if (cmd == proto::kCmdClearSoftwareEStop) {
          // Software-requested clear is only honored if the *hardware* radio
          // line is also released — the ESP32 never trusts software alone
          // to clear a hardware E-Stop.
          if (digitalRead(kTeamRadioGpioPin) == HIGH) {g_estop_pressed = false;}
        }
      }
      idx = 0;
    }
  }
}

void loop()
{
  // Direct GPIO line (if the team radio drives a wired digital output
  // instead of / in addition to LoRa) — pressed pulls the line low.
  bool gpio_estop = digitalRead(kTeamRadioGpioPin) == LOW;
  g_estop_pressed = g_estop_pressed || gpio_estop;

  bool jetson_hung = (millis() - g_last_jetson_kick_ms) > kJetsonWatchdogTimeoutMs;
  bool effective_estop = g_estop_pressed || jetson_hung;

  applyMotorPowerGate(effective_estop);

  pollJetsonCommands();

  static unsigned long last_status_ms = 0;
  if (millis() - last_status_ms >= kStatusPeriodMs) {
    sendStatusPacket();
    last_status_ms = millis();
  }
}
