#include "controller/stanley_controller.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace controller
{

double StanleyController::computeSteeringAngle(
  const std::vector<PathPoint> & path_body_frame,
  const VehicleState & state)
{
  if (path_body_frame.size() < 2) {
    return 0.0;
  }

  const PathPoint front_axle{params_.wheelbase_m, 0.0};

  // Find the path segment whose perpendicular projection is closest to the
  // front axle -- gives both the cross-track error and the local path
  // tangent (hence heading error) in a single pass over the path.
  double best_dist_sq = std::numeric_limits<double>::max();
  double best_cte = 0.0;
  double best_tangent_angle = 0.0;

  for (size_t i = 0; i + 1 < path_body_frame.size(); ++i) {
    const PathPoint & a = path_body_frame[i];
    const PathPoint & b = path_body_frame[i + 1];
    double seg_x = b.x - a.x;
    double seg_y = b.y - a.y;
    double seg_len_sq = seg_x * seg_x + seg_y * seg_y;
    if (seg_len_sq < 1e-9) {continue;}

    double t = ((front_axle.x - a.x) * seg_x + (front_axle.y - a.y) * seg_y) / seg_len_sq;
    t = std::clamp(t, 0.0, 1.0);
    double proj_x = a.x + t * seg_x;
    double proj_y = a.y + t * seg_y;
    double dx = front_axle.x - proj_x;
    double dy = front_axle.y - proj_y;
    double dist_sq = dx * dx + dy * dy;

    if (dist_sq < best_dist_sq) {
      best_dist_sq = dist_sq;
      // Signed perpendicular distance via the 2D cross product of the
      // segment direction with the vector to the front axle: positive
      // means the axle is to the left of the path's direction of travel,
      // so negate to get "+ = axle is to the right" (steer left/positive
      // to correct), matching the classical Stanley sign convention.
      double cross = seg_x * dy - seg_y * dx;
      best_cte = -cross / std::sqrt(seg_len_sq);
      best_tangent_angle = std::atan2(seg_y, seg_x);
    }
  }

  // Path is expressed in the vehicle body frame, so the vehicle's own
  // heading is 0 by construction -- the path's local tangent angle is
  // directly the heading error to correct.
  double heading_error = best_tangent_angle;
  last_cross_track_error_m_ = best_cte;
  last_heading_error_rad_ = heading_error;

  double v = std::max(std::abs(state.speed_mps), 0.0);
  double delta = heading_error +
    std::atan2(params_.cross_track_gain * best_cte, params_.softening_speed_mps + v);

  return std::clamp(delta, -params_.max_steering_angle_rad, params_.max_steering_angle_rad);
}

}  // namespace controller
