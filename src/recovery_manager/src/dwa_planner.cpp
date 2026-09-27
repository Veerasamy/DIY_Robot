#include "recovery_manager/dwa_planner.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace recovery_manager
{

namespace
{
// Returns -1 if (x, y) in the body frame falls outside the grid.
int cellIndex(const OccupancyGridView & grid, double x, double y)
{
  int gx = static_cast<int>((x - grid.origin_x) / grid.resolution);
  int gy = static_cast<int>((y - grid.origin_y) / grid.resolution);
  if (gx < 0 || gx >= grid.width || gy < 0 || gy >= grid.height) {return -1;}
  return gy * grid.width + gx;
}
}  // namespace

double DwaPlanner::trajectoryClearanceM(
  const std::vector<std::pair<double, double>> & trajectory_xy,
  const OccupancyGridView & grid) const
{
  // Sentinel "plenty clear" value when no occupied cell is found in the
  // bounded search window around any trajectory point -- this planner
  // intentionally does not compute a full distance transform (too costly
  // for what is otherwise a lightweight recovery-only node).
  double sentinel_clear_m =
    (params_.clearance_search_cells + 1) * grid.resolution;
  double min_clearance = sentinel_clear_m;

  for (const auto & point : trajectory_xy) {
    int idx = cellIndex(grid, point.first, point.second);
    if (idx < 0) {continue;}  // outside the local grid == assumed clear

    int gx = idx % grid.width;
    int gy = idx / grid.width;

    for (int dy = -params_.clearance_search_cells; dy <= params_.clearance_search_cells; ++dy) {
      for (int dx = -params_.clearance_search_cells; dx <= params_.clearance_search_cells; ++dx) {
        int nx = gx + dx;
        int ny = gy + dy;
        if (nx < 0 || nx >= grid.width || ny < 0 || ny >= grid.height) {continue;}
        int8_t val = grid.data[static_cast<size_t>(ny * grid.width + nx)];
        if (val < 100) {continue;}  // free or unknown, not an obstacle cell

        double cell_dist = std::hypot(dx, dy) * grid.resolution;
        min_clearance = std::min(min_clearance, cell_dist);
      }
    }
  }
  return min_clearance;
}

double DwaPlanner::forwardClearanceM(const OccupancyGridView & grid) const
{
  double min_clearance = std::numeric_limits<double>::max();
  bool found_any_obstacle = false;

  for (int gy = 0; gy < grid.height; ++gy) {
    for (int gx = 0; gx < grid.width; ++gx) {
      int8_t val = grid.data[static_cast<size_t>(gy * grid.width + gx)];
      if (val < 100) {continue;}

      double x = grid.origin_x + (gx + 0.5) * grid.resolution;
      double y = grid.origin_y + (gy + 0.5) * grid.resolution;
      if (x <= 0.0) {continue;}  // only the forward hemisphere matters here

      found_any_obstacle = true;
      min_clearance = std::min(min_clearance, std::hypot(x, y));
    }
  }
  if (!found_any_obstacle) {
    return static_cast<double>(std::max(grid.width, grid.height)) * grid.resolution;
  }
  return min_clearance;
}

DwaCommand DwaPlanner::plan(
  double current_speed_mps, double current_yaw_rate_rad_s,
  const OccupancyGridView & grid, double goal_heading_rad) const
{
  DwaCommand best;
  double best_score = -std::numeric_limits<double>::max();

  double window_dt = params_.dt_sec * params_.predict_steps;
  double v_lo = std::clamp(
    current_speed_mps - params_.max_accel_mps2 * window_dt,
    params_.min_speed_mps, params_.max_speed_mps);
  double v_hi = std::clamp(
    current_speed_mps + params_.max_accel_mps2 * window_dt,
    params_.min_speed_mps, params_.max_speed_mps);
  double w_lo = std::clamp(
    current_yaw_rate_rad_s - params_.max_yaw_accel_rad_s2 * window_dt,
    -params_.max_yaw_rate_rad_s, params_.max_yaw_rate_rad_s);
  double w_hi = std::clamp(
    current_yaw_rate_rad_s + params_.max_yaw_accel_rad_s2 * window_dt,
    -params_.max_yaw_rate_rad_s, params_.max_yaw_rate_rad_s);

  int v_n = std::max(params_.v_samples, 1);
  int w_n = std::max(params_.omega_samples, 1);
  double collision_threshold_m = params_.vehicle_radius_m + params_.collision_margin_m;
  double speed_norm = std::max(std::abs(params_.max_speed_mps), std::abs(params_.min_speed_mps));
  speed_norm = std::max(speed_norm, 1e-3);

  for (int i = 0; i < v_n; ++i) {
    double v = v_n == 1 ? v_lo : v_lo + (v_hi - v_lo) * i / (v_n - 1);
    for (int j = 0; j < w_n; ++j) {
      double w = w_n == 1 ? w_lo : w_lo + (w_hi - w_lo) * j / (w_n - 1);

      std::vector<std::pair<double, double>> trajectory;
      trajectory.reserve(static_cast<size_t>(params_.predict_steps));
      double x = 0.0, y = 0.0, theta = 0.0;
      for (int k = 0; k < params_.predict_steps; ++k) {
        x += v * std::cos(theta) * params_.dt_sec;
        y += v * std::sin(theta) * params_.dt_sec;
        theta += w * params_.dt_sec;
        trajectory.emplace_back(x, y);
      }

      double clearance = trajectoryClearanceM(trajectory, grid);
      if (clearance < collision_threshold_m) {continue;}  // reject: collision

      double heading_error = std::atan2(std::sin(theta - goal_heading_rad), std::cos(theta - goal_heading_rad));
      double heading_score = 1.0 - std::abs(heading_error) / M_PI;
      double clearance_score = std::min(clearance / (collision_threshold_m * 3.0), 1.0);
      double velocity_score = std::abs(v) / speed_norm;

      double score = params_.heading_weight * heading_score +
        params_.clearance_weight * clearance_score +
        params_.velocity_weight * velocity_score;

      if (score > best_score) {
        best_score = score;
        best.trajectory_found = true;
        best.speed_mps = v;
        best.min_clearance_m = clearance;
        // Bicycle-model inverse kinematics: delta = atan2(omega*L, v). Guard
        // the near-zero-speed singularity (unicycle omega has no bicycle
        // equivalent at v=0) by falling back to the max steering angle in
        // the commanded turn direction.
        if (std::abs(v) < 1e-3) {
          best.steering_angle_rad = w >= 0.0 ?
            params_.max_steering_angle_rad : -params_.max_steering_angle_rad;
        } else {
          best.steering_angle_rad = std::clamp(
            std::atan2(w * params_.wheelbase_m, v),
            -params_.max_steering_angle_rad, params_.max_steering_angle_rad);
        }
      }
    }
  }

  return best;
}

}  // namespace recovery_manager
