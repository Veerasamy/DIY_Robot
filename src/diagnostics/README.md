# diagnostics

Rosbag/health dashboards + a GPU/CPU profiling harness (ARCHITECTURE.md
section 7), the last of the previously-documented-but-not-coded packages.

## `system_health_node`

Publishes `diagnostic_msgs/DiagnosticArray` on `/diagnostics` at
`publish_rate_hz` (default 2Hz):

- **Topic watchdog** — `/safety/vehicle_state`, `/safety/estop_status`,
  `/ekf/odom`, `/cmd_ackermann`: `OK` while receiving, `ERROR` once
  `stale_after_sec` has elapsed since the last message, `WARN` if never
  received. View with `rqt_runtime_monitor` / `rqt_robot_monitor`.
- **Jetson GPU/CPU/RAM** — shells out to `tegrastats` (there is no
  `nvidia-smi` on Jetson's integrated GPU) every `gpu_cpu_sample_rate_hz`
  and regex-parses `RAM used/total`, `GR3D_FREQ` (GPU %) and the per-core
  `CPU [..%@freq,...]` block into an average. Reports `WARN` rather than
  failing if `tegrastats` isn't found (e.g. running this off-Jetson).

## Scripts (installed alongside the node, run with `ros2 run diagnostics <script>`)

- **`record_bag.py`** — wraps `ros2 bag record` for every topic in
  ARCHITECTURE.md's topic contract table (override with `--topics`).
- **`clear_estop.py`** — publishes the single explicit
  `operator_clear_requested` (`/safety/state_request = 9`) that is the
  *only* way `safety_manager`'s state machine exits `EMERGENCY_STOP` (by
  design — see `vehicle_state_machine.hpp`). Run manually, only after
  confirming the course is actually clear.
- **`gpu_profile.py`** — logs `tegrastats` samples to CSV for a fixed
  duration. Not a TensorRT layer profiler (no TensorRT model exists in this
  repo yet) — this is the entry point for that once one is added; for now
  it answers "do I have GPU/RAM headroom for NEURAL depth + lane_detection's
  CUDA path at the same time?".

## Topics

- Subscribes: `/safety/vehicle_state`, `/safety/estop_status`, `/ekf/odom`,
  `/cmd_ackermann`
- Publishes: `/diagnostics` (`diagnostic_msgs/DiagnosticArray`)
