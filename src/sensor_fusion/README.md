# sensor_fusion

EKF fusing ZED Visual Odometry, ZED IMU (yaw rate) and wheel encoder speed
into `/ekf/odom` (Command 4).

## State vector

`x = [px, py, theta, v, omega]ᵀ` — see full derivation in
[ARCHITECTURE.md](../../ARCHITECTURE.md#6-sensor-fusion-ekf-design-section-b).

## Why a sequential (not stacked) EKF

IMU (~400 Hz), VO (~30–60 Hz) and encoder (~50–100 Hz) all arrive at
different, jittery rates. Rather than resampling everything onto a fixed
fusion period, each sensor callback:

1. Computes `dt` since the last `predict()` and calls `EKF::predict(dt)`.
2. Immediately applies its own `updateX()`.
3. Publishes the updated `/ekf/odom`.

This means the filter's output is always as fresh as the last sensor
message, which matters at race speed where a 480Hz->50Hz downsample would
throw away IMU information between control cycles.

## Parameters

| Name | Default | Notes |
|---|---|---|
| `process_noise_pos` | 0.01 | m²/s, position random-walk |
| `process_noise_theta` | 0.005 | rad²/s |
| `process_noise_v` | 0.2 | large: model has no acceleration input |
| `process_noise_omega` | 0.1 | large: model has no angular-accel input |
| `encoder_variance` | 0.01 | (m/s)², bench-measure from tick jitter |
| `imu_yaw_rate_variance` | 0.0009 | (rad/s)², from ZED IMU datasheet |

## Topics

- Subscribes: `/zed/odom/vo`, `/zed/imu/data`, `/encoder/velocity_mps`
  (`std_msgs/Float32`, published by the future `encoder_interface` package)
- Publishes: `/ekf/odom` (`nav_msgs/Odometry`), `odom -> base_link` TF
