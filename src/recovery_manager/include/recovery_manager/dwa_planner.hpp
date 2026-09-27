// Dynamic Window Approach (DWA) planner for low-speed recovery maneuvers
// (ARCHITECTURE.md section 5: DWA recommended for `recovery_manager`'s
// REVERSE/RECOVER states, vs. VFH for the high-speed reactive layer in
// `obstacle_detection`). Pure algorithm class, no ROS dependency, operating
// on a local occupancy grid already expressed in the vehicle body frame
// (matches `obstacle_detection`'s `/obstacle/costmap`: x=forward, y=left,
// re-centered on the vehicle every cycle from live sensor data).
//
// Algorithm:
//   1. Dynamic window: bound (v, omega) to what's reachable within one
//      control cycle given max acceleration/yaw-acceleration limits and the
//      configured absolute (v, omega) caps.
//   2. Sample a v_samples x omega_samples grid within that window.
//   3. Forward-simulate each (v, omega) as a constant-command unicycle
//      trajectory for `predict_steps` steps of `dt_sec`.
//   4. Score each trajectory: clearance to the nearest occupied costmap cell
//      (found via a bounded local window search around every simulated
//      point -- not a full distance transform, kept cheap for this node's
//      relatively low control rate), heading alignment to a caller-supplied
//      goal heading, and progress (|v| magnitude).
//   5. Reject any trajectory whose clearance is inside the vehicle's own
//      radius + margin outright; return the best-scoring survivor.
#pragma once

#include <cstdint>
#include <vector>

namespace recovery_manager
{

// Snapshot of a nav_msgs/OccupancyGrid already in the vehicle body frame,
// re-centered on the vehicle every cycle (as obstacle_detection publishes
// it), so no extra pose-tracking is needed here.
struct OccupancyGridView
{
  std::vector<int8_t> data;  // row-major, 0=free, 100=occupied, -1=unknown
  int width{0};
  int height{0};
  double resolution{0.1};
  double origin_x{0.0};  // body-frame x of grid cell (0,0), meters
  double origin_y{0.0};  // body-frame y of grid cell (0,0), meters
};

struct DwaParams
{
  double max_speed_mps{0.6};
  double min_speed_mps{-0.6};   // negative = reverse-only window when max==0
  double max_accel_mps2{1.0};
  double max_yaw_rate_rad_s{1.5};
  double max_yaw_accel_rad_s2{2.5};
  double wheelbase_m{0.045};    // measured: 4.5cm wheelbase
  double dt_sec{0.1};
  int predict_steps{6};
  double vehicle_radius_m{0.3}; // inflation radius for collision checking -- measured car ~530x273mm circumscribes to ~0.3m
  double collision_margin_m{0.1};
  int v_samples{7};
  int omega_samples{9};
  int clearance_search_cells{3};  // bounded window radius, in cells, for the clearance search
  double heading_weight{1.0};
  double clearance_weight{1.5};
  double velocity_weight{0.5};
  double max_steering_angle_rad{0.2812};  // measured: atan(1.3cm max deviation / 4.5cm wheelbase) = 16.1 deg
};

struct DwaCommand
{
  bool trajectory_found{false};
  double speed_mps{0.0};
  double steering_angle_rad{0.0};
  double min_clearance_m{0.0};
};

class DwaPlanner
{
public:
  explicit DwaPlanner(const DwaParams & params) : params_(params) {}

  // goal_heading_rad: desired trajectory end-heading in the body frame
  // (0 = straight ahead), used purely as a scoring bias toward re-aligning
  // with e.g. the lane centerline during RECOVER.
  DwaCommand plan(
    double current_speed_mps, double current_yaw_rate_rad_s,
    const OccupancyGridView & grid, double goal_heading_rad) const;

  // Minimum obstacle clearance in the forward hemisphere (x > 0) within the
  // grid, independent of any candidate trajectory -- used by the node to
  // decide when it's safe to end a REVERSE maneuver (see recovery_manager's
  // README for why this is checked against live sensor data each cycle
  // rather than a fixed timeout alone).
  double forwardClearanceM(const OccupancyGridView & grid) const;

  void setParams(const DwaParams & params) {params_ = params;}
  const DwaParams & params() const {return params_;}

private:
  double trajectoryClearanceM(
    const std::vector<std::pair<double, double>> & trajectory_xy,
    const OccupancyGridView & grid) const;

  DwaParams params_;
};

}  // namespace recovery_manager
