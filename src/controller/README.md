# controller — Pure Pursuit / Stanley / MPC

Implements Pure Pursuit, Stanley and a lightweight MPC steering controller
(Command 3) behind a single `ISteeringController` interface, selected at
runtime via the `controller_type` parameter (`pure_pursuit` | `stanley` |
`mpc`), plus a curvature-adaptive speed profile shared by all three.

## SCRT10 wheelbase — action required

`wheelbase_m` defaults to **0.26 m** as a placeholder typical of a 1/10-scale
4WD short-course-truck chassis. **Measure your actual SCRT10's front-to-rear
axle center distance and set `wheelbase_m` accordingly** — Pure Pursuit's
steering command is directly proportional to this value
(`delta = atan2(2*L*sin(alpha), Ld)`), so an incorrect wheelbase produces a
consistent under/over-steer bias at every curvature.

## Tuning `lookahead_*`

- `lookahead_min_m` / `lookahead_max_m` bound the lookahead distance `Ld`.
- `lookahead_speed_gain` scales `Ld` with speed: `Ld = gain*v + min`.
- Symptoms and fixes:
  - **Oscillating/zig-zagging steering** → `Ld` too short for the current
    speed → increase `lookahead_speed_gain` or `lookahead_min_m`.
  - **Cutting corners / wide understeer through turns** → `Ld` too long →
    decrease `lookahead_speed_gain` or `lookahead_max_m`.

## Pure Pursuit vs Stanley vs MPC

See [ARCHITECTURE.md](../../ARCHITECTURE.md#4-controller-comparison-pure-pursuit-vs-stanley-vs-mpc).
Pure Pursuit is the default (`controller_type: pure_pursuit`) for its O(n)
lookup cost and single well-understood tuning knob (`Ld`), at the cost of
some corner-cutting at high curvature versus Stanley/MPC — acceptable for
Speed Race Mode where straight-line speed dominates lap time more than
perfect apex tracking.

- `controller_type: stanley` — front-axle heading+cross-track controller
  (`stanley_cross_track_gain` = `k_e`, `stanley_softening_speed_mps` =
  `k_soft`). Tracks the path more tightly than Pure Pursuit at the cost of
  being twitchier at low speed / on noisy centerlines — increase `k_soft` if
  it oscillates near a stop.
- `controller_type: mpc` — receding-horizon controller solved by
  derivative-free pattern search over the true nonlinear bicycle model (no
  external QP solver dependency, see `mpc_controller.hpp` for the full
  derivation). `mpc_horizon_steps`/`mpc_dt_sec` set the lookahead window;
  `mpc_q_cross_track`/`mpc_q_heading` trade off path-tracking tightness vs.
  `mpc_r_steering`/`mpc_r_steering_rate` smoothness. Most expensive of the
  three per cycle — profile on the Orin Nano before racing with it.

All three share `wheelbase_m` and `max_steering_angle_rad`.

## REVERSE/RECOVER handoff

This node only commands motion in `NORMAL`/`AVOID` vehicle states; the
`REVERSE`/`RECOVER` states are exclusively driven by the `recovery_manager`
package's DWA-based low-speed maneuver so the two nodes never publish
conflicting `/cmd_ackermann` commands.

## Steering slew-rate limit

`max_steering_rate_rad_s` (default 3.0 rad/s, a placeholder -- measure the
Savox 640's actual slew rate) bounds how fast the *final combined* steering
command (base controller output + obstacle-avoidance bias) can change per
control cycle. Without this, a VFH valley switch, a noisy centerline point,
or a controller swap could command a near-instant angle change that exceeds
the tires' available lateral grip and skids the vehicle instead of turning
it. The limiter re-anchors to 0 whenever motion is halted (`BRAKE`/E-Stop/no
path), so resuming motion isn't rate-limited against a stale angle.

## Topics

- Subscribes: `/lane/centerline`, `/lane/curvature_radius_m`, `/ekf/odom`,
  `/safety/vehicle_state`, `/obstacle/avoidance_cmd`
- Publishes: `/cmd_ackermann` (`ackermann_msgs/AckermannDriveStamped`) — note
  this is gated a second time by `safety_manager` before reaching the ESP32.
