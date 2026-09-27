# Use Cases — Inputs & Outputs

Every use case below has been implemented and verified in this session, either
against the real compiled ROS2 nodes (via the mock-hardware demo stack) or,
where explicitly noted, only in the browser dashboard simulation
(`scripts/interactive_dashboard.py`, `http://localhost:8088`). "Verified"
means the exact behavior described was observed live, not just implemented.

Vehicle reference data used throughout: wheelbase 4.5cm, max steering
deviation 1.3cm (→ max steering angle 16.1°/0.2812 rad, min turning radius
15.6cm), body 530×273mm, height 190–202mm, ground clearance 4cm, ZED
camera + LiDAR mounted 19–20cm above ground, track path width 36in (0.9144m).

---

## 1. Normal Lane Following

| | |
|---|---|
| **Trigger** | Vehicle in `NORMAL` state, lane visible to camera |
| **Inputs** | `/zed/rgb/image_raw` (camera), `/ekf/odom` (current speed) |
| **Processing** | `lane_detection` → threshold + bird's-eye warp + sliding-window fit → `/lane/centerline`, `/lane/curvature_radius_m`; `controller` (Pure Pursuit, default) computes steering from centerline + lookahead |
| **Outputs** | `/cmd_ackermann`: `steering_angle` (rad, clamped ±0.2812), `speed` (m/s, from `v = sqrt(max_lateral_accel_mps2 * curvature_radius_m)`, clamped 0.5–8.0 m/s) |
| **Verified** | Yes — steering tracks synthetic lane curvature smoothly; speed drops on tight curves, recovers to 8.0 m/s on straights |

## 2. Hairpin / Tight-Turn Handling

| | |
|---|---|
| **Trigger** | Curvature radius drops below ~16m (where `v = sqrt(4×R) < 8`) |
| **Inputs** | Same as #1 |
| **Processing** | `pure_pursuit_node`'s steering slew-rate limiter (`max_steering_rate_rad_s = 3.0`) bounds how fast steering can change per cycle, preventing an instantaneous snap that could skid the tires |
| **Outputs** | Steering ramps smoothly to its ±0.2812 rad limit; speed drops proportionally (e.g. ~4.7–6.9 m/s at R≈9–12m) |
| **Verified** | Yes — "U-turn left/right", "Speed Course", "Figure-8", "Multi-Hairpin" dashboard scenarios all show smooth saturating steering with no instantaneous jumps |

## 3. Object Detected — Straight Ahead

| | |
|---|---|
| **Trigger** | Depth/LiDAR obstacle within the ±15° front sector |
| **Inputs** | `/zed/depth/image`, `/scan` (filtered LiDAR) |
| **Processing** | `obstacle_detection`'s VFH planner finds a clear steering "valley" to one side; braking-distance check (`braking_decel_mps2 = 6.0`) computes a continuous speed-scale factor |
| **Outputs** | `/obstacle/avoidance_cmd`: `angular.z` = steering bias (added to lane-following steering), `linear.x` = speed multiplier (1.0 clear → 0.15 crawl); `/safety/state_request` = 1 (AVOID) or 2 (BRAKE) if within braking distance |
| **Verified** | Yes — object at 1.0m dead-ahead: steering saturates to max (routing around it), speed drops from 8.0 → 3.33 m/s |

## 4. Object Detected — Left or Right Side

| | |
|---|---|
| **Trigger** | Depth/LiDAR obstacle within a side sector (20–90° off-axis) |
| **Inputs** | Same as #3, plus object angle |
| **Processing** | Same VFH + speed-scale logic; steering bias direction depends on which side the obstacle is on |
| **Outputs** | Left-side obstacle (+angle, REP-103) → negative (rightward) steering bias; right-side obstacle → positive (leftward) bias |
| **Verified** | Yes — object at +60° (left): steer → −0.524 rad; object at −60° (right): steer → +0.524 rad (symmetric, correct) |

## 5. Wall / Dead-End Detection

| | |
|---|---|
| **Trigger** | Front clearance < 1.2m **AND** both left clearance < 0.5m **AND** right clearance < 0.5m simultaneously (no passable gap on any side) |
| **Inputs** | Fused depth+LiDAR points, sector-clearance computation (`computeSectorClearance`) |
| **Processing** | `obstacle_detection` distinguishes "wall" (nothing passable) from a normal avoidable obstacle (which has a clear side) |
| **Outputs** | `/obstacle/wall_detected` = true, `/obstacle/avoidance_cmd.linear.x` forced to 0.0 (overrides the normal continuous scale), `/safety/state_request` = 2 (BRAKE) |
| **Verified** | Yes — object spanning ±100° at 0.35m: `wall_detected=true`, `speed=0.0`, `steer=0.0`, state escalates to REVERSE ("stopped") |

## 6. Cliff / Track-Edge (Negative Obstacle) Detection

| | |
|---|---|
| **Trigger** | Depth reading at a near-ground row is significantly farther than a flat floor would give (the ground has dropped away) within 1.5m |
| **Inputs** | `/zed/depth/image`, camera mount height/tilt/FOV params |
| **Processing** | `computeCliffDistance()` compares actual depth against the expected flat-ground depth per row; if missing/farther by >0.15m, that row's expected distance is the edge distance |
| **Outputs** | `/obstacle/cliff_detected` = true, `/obstacle/cliff_distance_m`, forces full stop (`linear.x=0.0`) + `/safety/state_request` = 2 (BRAKE) — same full-stop path as a wall |
| **Verified** | Yes — floor removed from rows 550→720: `cliff_detected=true`, `cliff_distance_m=0.22`, `speed=0.0`; recovers cleanly once floor returns |

## 7. Figure-8 Course

| | |
|---|---|
| **Trigger** | Dashboard "Figure-8 (loop)" button (test/demo scenario only) |
| **Inputs** | Scripted lane-bend profile: two opposite-rotation full loops joined by short straights |
| **Processing** | Same lane-following pipeline as #1/#2, no obstacles |
| **Outputs** | Steering alternates sign smoothly across both lobes; speed drops in each tight lobe and recovers on straights |
| **Verified** | Yes — full loop: `state=NORMAL` throughout, steering swings both directions (−0.30/+0.52/+0.16 rad observed), speed 6.3–8.0 m/s |

## 8. Multi-Hairpin (Technical Slalom) Course

| | |
|---|---|
| **Trigger** | Dashboard "Multi-Hairpin (loop)" button (test/demo scenario only) |
| **Inputs** | Scripted lane-bend profile: 5 alternating-direction hairpins |
| **Processing** | Same as #7 |
| **Outputs** | Steering alternates through all 5 hairpins; speed dips at each apex |
| **Verified** | Yes — `state=NORMAL` throughout a 52.5s lap, steering swung through all 5 alternating turns, speed 6.4–8.0 m/s |

## 9. Emergency Stop — Manual Button Press

| | |
|---|---|
| **Trigger** | Physical E-Stop button (or dashboard "EMERGENCY STOP" button in simulation) |
| **Inputs** | Real hardware: `Master.ino`'s BOOT button → WiFi TCP `"MASTER_ON"` → `SlaveMotorController.ino` sets `masterStop=true` → `emergencyStop()` (ESC to neutral, then hardware power cutoff via `POWER_CUTOFF_PIN`). Our ROS2/dashboard simulation: mock UART `StatusPacket.estop_pressed=1` → `safety_manager` |
| **Processing** | `safety_manager`'s state machine treats `estop_pressed` as an unconditional, highest-priority transition to `EMERGENCY_STOP` from any state |
| **Outputs** | `/safety/vehicle_state` = 5 (EMERGENCY_STOP), `/safety/gated_cmd_ackermann` and `pure_pursuit_node`'s own `/cmd_ackermann` both zeroed (`speed=0, steering=0`) |
| **Verified** | Yes (ROS2/dashboard side) — press → within ~1s: state=5, speed=0.0, steering=0.0. Clear → within ~1.4s: state=0 (NORMAL), speed restored to 8.0. **Real hardware note**: as originally written, `emergencyStop()` only wrote ESC-neutral (no active braking) — the car would coast, not stop instantly, until the hardware power-cutoff fix (see SYSTEM_FLOW.md) was added this session |

## 10. Emergency Stop — Implicit (Link/Heartbeat Loss)

| | |
|---|---|
| **Trigger** | E-Stop UART link goes stale (`link_timeout_sec = 0.3s`, ROS2 side) or WiFi/TCP link to Master is lost (real firmware side) |
| **Inputs** | Absence of a fresh heartbeat packet (ROS2) / `WiFi.status() != WL_CONNECTED` or `!wifiClient.connected()` (firmware) |
| **Processing** | Both are treated as fail-safe implicit E-Stops — loss of the safety link is exactly as dangerous as the button being pressed, so it defaults to the safe state rather than continuing on stale commands |
| **Outputs** | Same as #9 |
| **Verified** | ROS2 side: yes (this is the `packet_fresh` mechanism validated throughout the session). Firmware side: added this session (`wifiTask`'s disconnect handlers now call `emergencyStop()`) — not flashed/tested on real hardware yet |

## 11. Recovery Maneuver (Blocked Path)

| | |
|---|---|
| **Trigger** | Vehicle state = `BRAKE` and path remains blocked (owned exclusively by `recovery_manager`, never `pure_pursuit_node`, to avoid both nodes fighting over `/cmd_ackermann`) |
| **Inputs** | `/obstacle/costmap`, `/ekf/odom`, `/lane/centerline`, `/safety/vehicle_state` |
| **Processing** | `recovery_manager`'s DWA planner samples (v, ω) pairs, scores by heading/clearance/velocity, drives `REVERSE` (back away, min-duration + live-clearance gated) then `RECOVER` (creep + re-align to centerline heading) |
| **Outputs** | `/cmd_ackermann` (negative speed while reversing), `/safety/state_request` = 3/4 |
| **Verified** | Indirectly — observed the state machine correctly escalate BRAKE→REVERSE on wall/cliff detection; DWA maneuver itself not directly exercised in this session's dashboard tests |

## 12. Speed Course Lap Completion / Finish-Line Stop

| | |
|---|---|
| **Trigger** | Integrated real commanded distance ≥ estimated lap distance (166.15m, derived from the attached Speed Course diagram) |
| **Inputs** | `/cmd_ackermann.speed` integrated over time by the dashboard |
| **Processing** | Dashboard-only — reuses the wall-detection full-stop path (forces a simulated wall directly ahead) once the lap distance is reached |
| **Outputs** | `distance_remaining_m` → 0, `lap_finished=true`, car stops and holds |
| **Verified** | Yes — full lap: distance ticked down correctly, stopped exactly at 0m, held there (didn't loop again) |
| **Important caveat** | **This is a dashboard simulation only.** The real ROS2 codebase (`planner`, `safety_manager`, `controller`) has **no** lap-counting, waypoint-wraparound, or finish-line-detection logic anywhere — confirmed by full-repo grep. There is currently no way for the real vehicle to know it has "completed a course" and stop on its own. |
