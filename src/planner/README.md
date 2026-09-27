# planner

Global racing-line loader plus a local/global path arbitration layer above
`obstacle_detection` (ARCHITECTURE.md section 7). Not wired into the default
bringup by default — see "Integrating with `controller`" below.

## What it does

- **`RacelineLoader`** reads a pre-computed course racing line (map-frame
  waypoints + optional per-waypoint target speed) from a YAML file.
  Computing that racing line (minimum-curvature or minimum-time optimization
  against the course boundary) is an **offline** step done once per course,
  not an onboard ROS2 responsibility — this package only loads and uses the
  result. `config/raceline.yaml` ships a 20m placeholder straight line;
  replace it before racing.
- **`local_planner_node`** transforms the loaded raceline into the vehicle
  body frame around the current `/ekf/odom` pose, and arbitrates it against
  `lane_detection`'s `/lane/centerline`:
  - **Speed Race Mode** (`race_mode: speed`): lane centerline is primary
    (ground-truth observed corridor); the raceline nudges the path toward
    the apex-hugging line within it (`blend_weight`).
  - **Obstacle Race Mode** (`race_mode: obstacle`): raceline is primary
    (there may be no lane markings at all on this course); centerline is
    only a fallback.
  - If `obstacle_detection` reports the path blocked (`/obstacle/avoidance_cmd`
    `linear.x == 0`), the arbitrated path is marked invalid and this node
    also requests `/safety/state_request = 1` (AVOID).

## Integrating with `controller`

`controller`'s `pure_pursuit_node` subscribes directly to `/lane/centerline`
today (see ARCHITECTURE.md's topic contract) — that wiring is left unchanged
by default so the existing tested pipeline keeps working. To have the
controller track the arbitrated path instead, remap at launch:

```bash
ros2 launch controller controller.launch.py --ros-args -r /lane/centerline:=/planner/local_path
```

## Topics

- Subscribes: `/ekf/odom`, `/lane/centerline`, `/obstacle/avoidance_cmd`
- Publishes: `/planner/local_path` (`nav_msgs/Path`, body frame,
  `base_link`), `/planner/global_path` (`nav_msgs/Path`, `map` frame,
  transient-local for rviz), `/safety/state_request` (`std_msgs/UInt8`)
