# safety_manager

Hardware E-Stop bridge and vehicle safety state machine. **Always launch this
node first** — every actuation node gates on `/safety/vehicle_state` and
`/safety/estop_status`.

## Why this design

- The competition brief only guarantees the **radio link** (team radio ⇄
  robot). Everything after that (parsing, state machine, actuation gating) is
  our responsibility, so the guarantee "stop within 1 second" is implemented
  with two independent layers:
  1. **Hardware layer (ESP32 firmware):** the ESP32 drives the ESC power gate
     directly from the decoded E-Stop line. This works even if the Jetson is
     powered off, crashed, or the UART cable is unplugged (fail-open wiring).
  2. **Software layer (this ROS2 node):** treats *loss* of the UART heartbeat
     (`link_timeout_sec`, default 0.3s) as an implicit E-Stop, and gates
     `/cmd_ackermann` before it reaches any downstream consumer.
- `VehicleStateMachine` implements Section G's state diagram
  (`NORMAL → AVOID → BRAKE → REVERSE → RECOVER → NORMAL`, with
  `EMERGENCY_STOP` pre-emptable from any state and requiring an explicit
  operator clear to exit).
- The Jetson also sends periodic `WATCHDOG_KICK` commands to the ESP32; if the
  ESP32 stops receiving them (Jetson hang/crash) it fails safe on its own,
  independent of ROS2.

## Topics

- Publishes: `/safety/estop_status` (`safety_manager/msg/EStopStatus`),
  `/safety/vehicle_state` (`safety_manager/msg/VehicleState`),
  `/safety/gated_cmd_ackermann` (`ackermann_msgs/AckermannDriveStamped`)
- Subscribes: `/cmd_ackermann` (from `controller` or `recovery_manager`,
  whichever owns the current vehicle state — they never publish at the same
  time, see both packages' READMEs),
  `/safety/state_request` (`std_msgs/UInt8`, from `obstacle_detection`/
  `planner` [0=clear, 1=obstacle_close, 2=collision_imminent],
  `recovery_manager` [3=path_clear, 4=recovered], `diagnostics`'
  `clear_estop.py` [9=operator_clear_requested])

## Parameters

| Name | Default | Notes |
|---|---|---|
| `serial_device` | `/dev/ttyUSB_estop` | udev-rule this to a stable name, not `/dev/ttyUSB0` |
| `serial_baud` | 115200 | must match `platformio.ini` |
| `link_timeout_sec` | 0.3 | keep well under the 1.0s competition budget |
| `control_period_sec` | 0.02 | 50 Hz state machine / gating loop |
| `watchdog_kick_period_sec` | 0.1 | must be < ESP32's `kJetsonWatchdogTimeoutMs` |

## Firmware

See `firmware/esp32_estop/` (PlatformIO project). Flash with:

```bash
cd firmware/esp32_estop
pio run -t upload
```
