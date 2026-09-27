# Decoded status of the physical E-Stop radio link, as reported by the
# ESP32 over UART. Published at the UART packet arrival rate (>= 5 Hz).

builtin_interfaces/Time stamp

# True when the course E-Stop relay is OPEN (button pressed) as read back
# on RJ45 pins 5-8 by the team radio and relayed to the robot.
bool estop_pressed

# True while heartbeat packets are arriving from the ESP32 within the
# configured timeout. False is treated as an implicit E-Stop (fail-safe).
bool link_ok

# True while the team-radio <-> robot-radio RF link itself is healthy
# (distinct from the Jetson<->ESP32 UART link captured by link_ok).
bool radio_ok

uint8 heartbeat_seq
uint16 battery_millivolts
int8 radio_rssi_dbm
