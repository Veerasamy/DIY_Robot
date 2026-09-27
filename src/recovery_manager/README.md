# recovery_manager — DWA

DWA-based low-speed recovery motion generator (ARCHITECTURE.md section 5
recommended DWA for this role, vs. VFH for `obstacle_detection`'s high-speed
reactive layer). Exclusively drives `/cmd_ackermann` while
`safety_manager`'s vehicle state machine is in `REVERSE` or `RECOVER` —
`controller`'s `pure_pursuit_node` only commands motion in `NORMAL`/`AVOID`,
so the two nodes never publish conflicting commands (see both packages'
READMEs).

## State machine handoff

See `safety_manager/include/safety_manager/vehicle_state_machine.hpp` for
the full diagram. This package's role starts once `BRAKE` transitions to
`REVERSE` (vehicle has stopped) and ends when it reports `recovered` back to
`safety_manager`, returning control to `NORMAL`:

1. **REVERSE** — DWA samples a negative-speed-only dynamic window, biased
   toward straight-back motion (`goal_heading=0`) but free to steer off-axis
   if that scores higher clearance. Exits (`/safety/state_request = 3`,
   `path_clear`) once **both** `reverse_min_duration_sec` has elapsed *and*
   the live costmap's forward clearance exceeds `reverse_clear_distance_m` —
   the duration alone is not trusted since the obstacle picture is refreshed
   from live sensor data every cycle (obstacle_detection's costmap is
   re-centered on the vehicle each cycle, so backing away genuinely
   increases measured forward clearance).
2. **RECOVER** — DWA samples a small positive-creep-speed window with full
   yaw range, biased toward the heading error needed to re-align with
   `/lane/centerline` (0 if no centerline is available — obstacle courses may
   have none). Exits (`/safety/state_request = 4`, `recovered`) once that
   heading error is within `recovered_heading_tol_rad`.

If no `/obstacle/costmap` has been received yet, or DWA finds no
collision-free trajectory in the sampled window, this node holds position
(zero speed) rather than guessing — a stuck-and-safe failure mode instead of
a stuck-and-moving one.

## Tuning

- `reverse_speed_mps` / `recover_creep_speed_mps` — keep both low; this is a
  low-speed maneuvering mode by design, not a racing speed.
- `vehicle_radius_m` / `collision_margin_m` — the effective SCRT10 footprint
  DWA won't let any sampled trajectory violate; measure the chassis width
  plus a safety pad.
- `reverse_clear_distance_m` — how much forward clearance is "enough" to
  stop reversing; too small risks re-triggering `BRAKE` immediately on
  return to `NORMAL`.

## Topics

- Subscribes: `/safety/vehicle_state`, `/obstacle/costmap`, `/ekf/odom`,
  `/lane/centerline`
- Publishes: `/cmd_ackermann` (`ackermann_msgs/AckermannDriveStamped`, only
  while active), `/safety/state_request` (`std_msgs/UInt8`)
