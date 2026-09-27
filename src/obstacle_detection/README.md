# obstacle_detection — VFH

Fuses ZED depth (projected to 2D) and RPLiDAR `/scan` into a single obstacle
point set, runs Vector Field Histogram (VFH) local avoidance, and publishes a
steering bias plus safety-state requests (Command 5).

## Why VFH over DWA/A* for this layer

See [ARCHITECTURE.md](../../ARCHITECTURE.md#5-obstacle-avoidance-algorithm-comparison-section-f).
VFH's O(sectors) cost keeps this node comfortably inside a 20Hz cycle budget
on the Orin Nano even with both a full LiDAR scan and a subsampled depth
image as input, which DWA's per-cycle trajectory rollout and A*'s grid search
cannot guarantee at the same rate.

## Depth-to-2D projection caveat

`depthImageToPoints()` uses a simplified pinhole + planar-tilt model
(`camera_mount_height_m`, `camera_tilt_rad`) rather than a full tf2-based
extrinsic transform. This is sufficient for coarse ground-plane rejection and
VFH sector binning, but if you see phantom obstacles or missed low obstacles,
first re-measure `camera_mount_height_m`/`camera_tilt_rad`, and consider
upgrading to a proper `base_link -> zed2i_camera_link` static transform with
`tf2_ros::Buffer::transform()` per-point for the final competition build.

## Safety-state requests

Publishes `/safety/state_request` (`std_msgs/UInt8`) consumed by
`safety_manager`:
- `2` (collision_imminent → BRAKE) when the required braking distance at the
  current EKF speed exceeds the measured forward clearance
- `1` (obstacle_close → AVOID) when VFH found no valley wide enough to pass
- `0` (clear) otherwise

## Topics

- Subscribes: `/scan`, `/zed/depth/image`, `/ekf/odom`
- Publishes: `/obstacle/avoidance_cmd` (`geometry_msgs/Twist`: `angular.z` =
  steering bias rad, `linear.x` = 0/1 go-scale), `/obstacle/costmap`
  (`nav_msgs/OccupancyGrid`), `/safety/state_request`
