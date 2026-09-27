# zed_interface

ZED 2i stereo camera driver publishing depth, RGB, IMU and visual odometry.

## Feature toggles (Section A of the design brief)

| Param | Speed mode | Obstacle mode | Why |
|---|---|---|---|
| `depth_mode` | `PERFORMANCE` | `NEURAL` | PERFORMANCE is the lowest-latency stereo matcher; NEURAL is far more accurate at range but costs ~2-3x GPU and only matters when planning around obstacles, not lane-following |
| `enable_point_cloud` | `false` | `true` | Point cloud publishing roughly doubles PCIe/USB bandwidth and GPU retrieve time — not needed for the depth-image-only lane/edge cueing used at speed |
| `camera_fps` | 60 | 30 | Higher FPS directly reduces steering-loop latency; obstacle mode trades FPS for NEURAL depth quality |
| `enable_ground_plane` | `false` | `true` | Ground-plane removal only matters for cleaning up the obstacle occupancy grid |

Switch modes with `ros2 launch zed_interface zed_interface.launch.py mode:=obstacle`.

## Topics published

- `/zed/rgb/image_raw` (`sensor_msgs/Image`, bgr8)
- `/zed/depth/image` (`sensor_msgs/Image`, 32FC1, meters)
- `/zed/depth/points` (`sensor_msgs/PointCloud2`, obstacle mode only)
- `/zed/imu/data` (`sensor_msgs/Imu`, ~400 Hz sensor-only rate, decoupled from grab loop)
- `/zed/odom/vo` (`nav_msgs/Odometry`, only published while `POSITIONAL_TRACKING_STATE::OK`)
- `odom -> zed2i_camera_link` TF

## Design notes

- Runs the blocking `sl::Camera::grab()` loop on its own thread so ROS2
  executor timer jitter never delays a frame grab (Section J: Jetson
  performance).
- Uses `slMatToCvMat()` to wrap the ZED SDK's CPU buffer directly as a
  `cv::Mat` **without copying**, then `cv_bridge` copies once into the
  ROS2 message — the only copy that's unavoidable without switching to
  `image_transport`'s zero-copy intra-process publishing (future work).
- Positional-tracking `enable_area_memory` is left `false`: area memory /
  loop-closure re-localization is a spatial-mapping/SLAM feature we
  intentionally don't need for a single competition lap and it costs both
  GPU and growing RAM over a run.
