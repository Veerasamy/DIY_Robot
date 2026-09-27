# lidar_interface

Thin wrapper/config around the community `rplidar_ros` driver (per
ARCHITECTURE.md section 7 — this package deliberately does not reimplement
the RPLiDAR USB/serial protocol). Republishes a cleaned `/scan` that
`obstacle_detection`'s VFH planner subscribes to.

## Install the vendored driver

Not built from this workspace — install from rosdep/apt or clone into
`src/` alongside this package:

```bash
sudo apt install ros-jazzy-rplidar-ros
# or: git clone https://github.com/Slamtec/rplidar_ros.git -b ros2 src/rplidar_ros
```

Confirm the exact executable name for your installed version
(`ros2 pkg executables rplidar_ros`) — `launch/lidar_interface.launch.py`
assumes `rplidar_node`; adjust if your version differs.

## What this package actually does

`scan_filter_node` subscribes to the driver's raw scan (remapped to
`/lidar/scan_raw`) and republishes `/scan` with:

- Range values outside `[range_min_m, range_max_m]` forced to `+inf` (REP-117
  invalid-but-in-range convention) rather than trusting the driver's raw
  clamp behavior.
- An optional angular **blind-spot mask** (`blind_spot_start_rad`/
  `blind_spot_end_rad`) for the sector occluded by the SCRT10's own mounting
  bracket/chassis — measure this on the assembled vehicle; every RC chassis
  mount blocks a different sector.
- Optional `frame_id_override` if the driver's default doesn't match your
  URDF/TF tree.

## Action required before racing

- **`serial_port`** (`config/rplidar.yaml`) — set up a persistent udev rule
  keyed on the RPLiDAR's USB `idVendor`/`idProduct` (e.g. `/dev/rplidar`)
  rather than the raw `/dev/ttyUSBx`, which is not stable across reboots or
  when other USB serial devices (ESP32) are also attached.
- **`serial_baudrate`** — 115200 for RPLiDAR A1/A2, 256000 for A3/S1; set to
  match your exact model.
- **`blind_spot_start_rad`/`blind_spot_end_rad`** — measure the occluded
  sector on the assembled chassis.

## Topics

- Subscribes (internally, from the driver): `/lidar/scan_raw`
- Publishes: `/scan` (`sensor_msgs/LaserScan`), consumed by
  `obstacle_detection`
