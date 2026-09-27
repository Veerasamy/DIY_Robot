# autonomous_rc — SCRT10 Autonomous RC Race Car Stack

Production ROS2 Jazzy (C++17 + Python) workspace for an autonomous SCRT10 4WD RC
race car running on a Jetson Orin Nano with a ZED 2i stereo camera, RPLiDAR, and
a quadrature magnetic wheel encoder. Targets Ubuntu 24.04.4 LTS.

This workspace was generated from `CameraModulePrompt.md`. All packages
documented in [ARCHITECTURE.md](ARCHITECTURE.md) are now implemented,
including the ones originally deferred to a later phase (`lidar_interface`,
`encoder_interface`, `planner`, `recovery_manager`, `diagnostics`, and the
Stanley/MPC steering controllers).

## Packages in this drop

| Package | Command | Purpose |
|---|---|---|
| `safety_manager` | *(mandatory, from E-Stop eml)* | ESP32 hardware E-Stop bridge + vehicle state machine (NORMAL→…→EMERGENCY_STOP) |
| `zed_interface` | 2 | ZED 2i depth, IMU, RGB and visual-odometry ROS2 topics |
| `lane_detection` | 1 | CUDA-accelerated OpenCV lane/road-edge detection, centerline generation |
| `sensor_fusion` | 4 | EKF fusing ZED VO + ZED IMU + wheel encoder |
| `controller` | 3 | Pure Pursuit / Stanley / MPC steering controllers (`controller_type` param) tuned for SCRT10 wheelbase |
| `obstacle_detection` | 5 | ZED depth + RPLiDAR fused VFH local obstacle avoidance |
| `lidar_interface` | — | Thin wrapper/config around the vendored `rplidar_ros` driver, publishes cleaned `/scan` |
| `encoder_interface` | — | libgpiod GPIO quadrature wheel-tick counter, publishes `/encoder/ticks` + `/encoder/velocity_mps` |
| `planner` | — | Global racing-line loader + local/global path arbitration above `obstacle_detection` |
| `recovery_manager` | — | DWA-based low-speed recovery motion for `safety_manager`'s `REVERSE`/`RECOVER` states |
| `diagnostics` | — | Topic-staleness watchdog + Jetson GPU/CPU health (`/diagnostics`), rosbag recording, GPU profiling scripts |

## Prerequisites

- Ubuntu 24.04.4 LTS
- ROS2 Jazzy Jalisco (`/opt/ros/jazzy`)
- ZED SDK 4.x + CUDA 12.x (for `zed_interface`, `lane_detection` GPU path)
- OpenCV 4.x (`sudo apt install libopencv-dev`)
- Eigen3 (`sudo apt install libeigen3-dev`)
- `ackermann_msgs`, `nav_msgs`, `sensor_msgs`, `tf2_ros`, `cv_bridge`, `diagnostic_msgs`, `yaml-cpp`
- `libgpiod-dev` (for `encoder_interface`'s GPIO backend — builds without it, falls back to zero output)
- `ros-jazzy-rplidar-ros` (vendored driver for `lidar_interface`, not built from this workspace)
- `pyserial` (only needed if you use the Python diagnostics tools, not for building)

## Building

A smart Python build wrapper (`build.py`) is provided instead of raw `colcon`
invocations — it validates the environment, supports incremental/clean/debug
builds and package selection.

```bash
# from the autonomous_rc/ directory
python3 build.py                      # build everything, Release, symlink-install
python3 build.py --packages controller sensor_fusion
python3 build.py --clean --debug
python3 build.py --check-env          # only validate toolchain, don't build
```

Under the hood this is equivalent to:

```bash
colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release \
  --event-handlers console_direct+
source install/setup.bash
```

## Running

```bash
source install/setup.bash
ros2 launch safety_manager safety_manager.launch.py      # bring up first, always
ros2 launch lidar_interface lidar_interface.launch.py
ros2 launch encoder_interface encoder_interface.launch.py
ros2 launch zed_interface zed_interface.launch.py
ros2 launch sensor_fusion sensor_fusion.launch.py
ros2 launch lane_detection lane_detection.launch.py
ros2 launch obstacle_detection obstacle_detection.launch.py
ros2 launch planner planner.launch.py
ros2 launch recovery_manager recovery_manager.launch.py
ros2 launch controller controller.launch.py
ros2 launch diagnostics diagnostics.launch.py
```

Or bring up the whole stack (bar `safety_manager`, launched first internally with a
stagger) in one shot: `ros2 launch autonomous_rc bringup.launch.py mode:=speed`
(`launch/bringup.launch.py`, from the workspace `install` tree).

`safety_manager` must always be launched first: `controller` only commands motion
in `NORMAL`/`AVOID`, and `recovery_manager` exclusively owns `REVERSE`/`RECOVER` —
see `controller`'s and `recovery_manager`'s READMEs for why they never fight over
`/cmd_ackermann`.

## E-Stop requirements captured from competition brief

Extracted from `Fw_ DIY Robotics Challenge - Course E-Stop Description.eml`:

- Course E-Stop circuit exposes an RJ45 socket per vehicle; **does not** supply
  voltage — the team radio must source 3–12V on pins 1‑4 and read the relay
  state back on pins 5‑8 (circuit **open** = pressed, **closed** = released).
- The team radio must relay the E-Stop state (or a periodic heartbeat) to the
  robot; the robot must come to a stop **within 1 second** of the E-Stop being
  pressed.
- POC validated ESP32 + LoRa radio link at ~300 ft — used here as the
  robot-side radio device driving `safety_manager`.
- Robot onboard electrical systems must not exceed 50V (informs ESC/BEC choice,
  not a software concern but documented for the safety review).
- Radio link must sustain ≥300 ft range for the Oct 9 final safety review.

See `src/safety_manager/firmware/esp32_estop/` for the firmware implementing the
radio→relay→UART bridge and the 1-second hard stop guarantee.

## Report

See [REPORT.md](REPORT.md) for the end-to-end summary of everything generated
in this pass, open items, and recommended next steps.
