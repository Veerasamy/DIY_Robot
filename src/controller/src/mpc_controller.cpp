#include "controller/mpc_controller.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace controller
{

void MpcController::setParams(const MpcParams & params)
{
  params_ = params;
  warm_start_.assign(static_cast<size_t>(params_.horizon_steps), 0.0);
}

MpcController::TrackingError MpcController::nearestPathError(
  const std::vector<PathPoint> & path_body_frame, const PathPoint & position) const
{
  TrackingError err;
  if (path_body_frame.size() < 2) {return err;}

  double best_dist_sq = std::numeric_limits<double>::max();
  for (size_t i = 0; i + 1 < path_body_frame.size(); ++i) {
    const PathPoint & a = path_body_frame[i];
    const PathPoint & b = path_body_frame[i + 1];
    double seg_x = b.x - a.x;
    double seg_y = b.y - a.y;
    double seg_len_sq = seg_x * seg_x + seg_y * seg_y;
    if (seg_len_sq < 1e-9) {continue;}

    double t = ((position.x - a.x) * seg_x + (position.y - a.y) * seg_y) / seg_len_sq;
    t = std::clamp(t, 0.0, 1.0);
    double proj_x = a.x + t * seg_x;
    double proj_y = a.y + t * seg_y;
    double dx = position.x - proj_x;
    double dy = position.y - proj_y;
    double dist_sq = dx * dx + dy * dy;

    if (dist_sq < best_dist_sq) {
      best_dist_sq = dist_sq;
      double cross = seg_x * dy - seg_y * dx;
      err.cross_track_m = -cross / std::sqrt(seg_len_sq);
      err.heading_rad = std::atan2(seg_y, seg_x);
    }
  }
  return err;
}

double MpcController::simulateCost(
  const std::vector<PathPoint> & path_body_frame,
  const std::vector<double> & steering_sequence,
  double speed_mps) const
{
  double x = 0.0, y = 0.0, theta = 0.0;
  double prev_delta = 0.0;
  double cost = 0.0;

  for (int k = 0; k < params_.horizon_steps; ++k) {
    double delta = steering_sequence[static_cast<size_t>(k)];

    x += speed_mps * std::cos(theta) * params_.dt_sec;
    y += speed_mps * std::sin(theta) * params_.dt_sec;
    theta += (speed_mps / params_.wheelbase_m) * std::tan(delta) * params_.dt_sec;

    TrackingError err = nearestPathError(path_body_frame, PathPoint{x, y});
    double heading_err = err.heading_rad - theta;

    cost += params_.q_cross_track * err.cross_track_m * err.cross_track_m;
    cost += params_.q_heading * heading_err * heading_err;
    cost += params_.r_steering * delta * delta;
    double ddelta = delta - prev_delta;
    cost += params_.r_steering_rate * ddelta * ddelta;

    prev_delta = delta;
  }
  return cost;
}

double MpcController::computeSteeringAngle(
  const std::vector<PathPoint> & path_body_frame,
  const VehicleState & state)
{
  if (path_body_frame.size() < 2) {
    return 0.0;
  }

  // Warm-start from the previous solution shifted by one step (classic
  // receding-horizon trick -- the tail is a good guess for the new horizon).
  std::vector<double> solution = warm_start_;
  if (solution.size() > 1) {
    std::rotate(solution.begin(), solution.begin() + 1, solution.end());
  }

  double speed_mps = std::max(state.speed_mps, 0.5);  // avoid a degenerate zero-speed horizon
  double step = params_.initial_search_step_rad;
  double best_cost = simulateCost(path_body_frame, solution, speed_mps);

  // Hooke-Jeeves coordinate pattern search: perturb each decision variable
  // by +-step, keep any improving move, and halve the step once a full
  // sweep yields no improvement -- converges to a local optimum of the
  // (locally near-convex) tracking cost in a handful of sweeps, with no
  // QP solver dependency.
  for (int iter = 0; iter < params_.optimization_iterations && step > params_.min_search_step_rad;
    ++iter)
  {
    bool improved = false;
    for (size_t k = 0; k < solution.size(); ++k) {
      for (double sign : {1.0, -1.0}) {
        std::vector<double> candidate = solution;
        candidate[k] = std::clamp(
          candidate[k] + sign * step,
          -params_.max_steering_angle_rad, params_.max_steering_angle_rad);
        double cost = simulateCost(path_body_frame, candidate, speed_mps);
        if (cost < best_cost) {
          best_cost = cost;
          solution = candidate;
          improved = true;
        }
      }
    }
    if (!improved) {
      step *= 0.5;
    }
  }

  warm_start_ = solution;
  return std::clamp(
    solution.front(), -params_.max_steering_angle_rad, params_.max_steering_angle_rad);
}

}  // namespace controller
