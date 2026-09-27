// Vector Field Histogram (VFH) local obstacle avoidance planner.
// Pure algorithm class (no ROS dependency), operating on obstacle points
// already expressed in the vehicle body frame (x=forward, y=left, meters).
//
// Algorithm (classic VFH, Borenstein & Koren 1991, simplified for a single
// reactive layer — see ARCHITECTURE.md section 5 for the VFH/DWA/A*
// tradeoff that justifies this choice):
//   1. Bin obstacle points into angular sectors around the vehicle.
//   2. Accumulate a magnitude per sector: closer obstacles contribute more
//      (weighted by (max_range - range)), points are naturally down-weighted
//      with distance so far-away noise doesn't block a sector.
//   3. Smooth the histogram with a moving-average kernel to merge isolated
//      spikes into contiguous "blocked" regions (avoids threading the
//      needle between two returns off the same physical obstacle).
//   4. Threshold into a binary histogram; find contiguous free-sector runs
//      ("valleys").
//   5. A valley is only a valid candidate if it is wide enough for the
//      vehicle (width + safety margin) to actually pass through at its
//      nearest obstacle range — prevents picking a valley that looks open
//      angularly but is geometrically too narrow.
//   6. Steer toward the valid valley closest to the desired goal direction.
#pragma once

#include <vector>

namespace obstacle_detection
{

struct ObstaclePoint
{
  double angle_rad{0.0};  // 0 = straight ahead, + = left (REP-103)
  double range_m{0.0};
};

struct VfhParams
{
  double fov_half_angle_rad{1.5708};  // +-90 deg field considered
  double sector_resolution_rad{0.0349};  // ~2 degrees
  int smoothing_kernel_width{5};
  double max_range_m{6.0};
  double obstacle_threshold{1.5};  // histogram magnitude above this = blocked
  double vehicle_half_width_m{0.1365};  // measured: 273mm wide car -> 13.65cm half-width
  double safety_margin_m{0.15};
  double goal_direction_rad{0.0};  // straight ahead by default; planner can override
};

struct VfhResult
{
  bool path_blocked{true};
  double steering_bias_rad{0.0};
  double min_clearance_m{0.0};
  std::vector<float> histogram;  // for costmap/debug visualization
  double sector_resolution_rad{0.0};
  double fov_half_angle_rad{0.0};
};

class VfhPlanner
{
public:
  explicit VfhPlanner(const VfhParams & params) : params_(params) {}

  VfhResult plan(const std::vector<ObstaclePoint> & points) const;

  void setParams(const VfhParams & params) {params_ = params;}
  const VfhParams & params() const {return params_;}

private:
  std::vector<float> buildHistogram(const std::vector<ObstaclePoint> & points, int n_sectors) const;
  std::vector<float> smooth(const std::vector<float> & histogram) const;

  VfhParams params_;
};

}  // namespace obstacle_detection
