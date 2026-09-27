#include "controller/pure_pursuit_controller.hpp"

#include <algorithm>
#include <cmath>

namespace controller
{

PathPoint PurePursuitController::findLookaheadPoint(
  const std::vector<PathPoint> & path_body_frame, double lookahead_m) const
{
  // Path points are already in the vehicle body frame (forward=x, left=y),
  // ordered from nearest to farthest, as produced by lane_detection's
  // centerline or the planner's local path. Find the first point at or
  // beyond the lookahead distance; linearly interpolate between the
  // straddling points for a smoother target than a nearest-point search.
  PathPoint prev{0.0, 0.0};
  double prev_dist = 0.0;

  for (const auto & pt : path_body_frame) {
    double dist = std::hypot(pt.x, pt.y);
    if (dist >= lookahead_m) {
      if (dist - prev_dist < 1e-6) {return pt;}
      double t = (lookahead_m - prev_dist) / (dist - prev_dist);
      t = std::clamp(t, 0.0, 1.0);
      return PathPoint{
        prev.x + t * (pt.x - prev.x),
        prev.y + t * (pt.y - prev.y)};
    }
    prev = pt;
    prev_dist = dist;
  }

  // Path shorter than the lookahead distance: aim at the farthest point.
  return path_body_frame.empty() ? PathPoint{lookahead_m, 0.0} : path_body_frame.back();
}

double PurePursuitController::computeSteeringAngle(
  const std::vector<PathPoint> & path_body_frame,
  const VehicleState & state)
{
  if (path_body_frame.empty()) {
    return 0.0;
  }

  double lookahead_m = std::clamp(
    params_.lookahead_speed_gain * std::abs(state.speed_mps) + params_.lookahead_min_m,
    params_.lookahead_min_m, params_.lookahead_max_m);
  last_lookahead_m_ = lookahead_m;

  PathPoint target = findLookaheadPoint(path_body_frame, lookahead_m);
  double ld = std::max(std::hypot(target.x, target.y), 1e-3);

  double alpha = std::atan2(target.y, target.x);
  double delta = std::atan2(2.0 * params_.wheelbase_m * std::sin(alpha), ld);

  return std::clamp(delta, -params_.max_steering_angle_rad, params_.max_steering_angle_rad);
}

}  // namespace controller
