#include "obstacle_detection/vfh_planner.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace obstacle_detection
{

std::vector<float> VfhPlanner::buildHistogram(
  const std::vector<ObstaclePoint> & points, int n_sectors) const
{
  std::vector<float> histogram(static_cast<size_t>(n_sectors), 0.0f);
  const double res = params_.sector_resolution_rad;

  for (const auto & p : points) {
    if (p.range_m <= 0.0 || p.range_m > params_.max_range_m) {continue;}
    if (std::abs(p.angle_rad) > params_.fov_half_angle_rad) {continue;}

    int sector = static_cast<int>(
      std::round((p.angle_rad + params_.fov_half_angle_rad) / res));
    sector = std::clamp(sector, 0, n_sectors - 1);

    // Closer obstacles contribute quadratically more magnitude — matches
    // classic VFH's m = c^2*(a - b*d) with c=1 (single-return certainty).
    double weight = (params_.max_range_m - p.range_m) / params_.max_range_m;
    float magnitude = static_cast<float>(weight * weight * 4.0);
    histogram[static_cast<size_t>(sector)] =
      std::max(histogram[static_cast<size_t>(sector)], magnitude);
  }
  return histogram;
}

std::vector<float> VfhPlanner::smooth(const std::vector<float> & histogram) const
{
  const int k = std::max(1, params_.smoothing_kernel_width);
  const int half = k / 2;
  std::vector<float> out(histogram.size(), 0.0f);

  for (size_t i = 0; i < histogram.size(); ++i) {
    float sum = 0.0f;
    int count = 0;
    for (int j = -half; j <= half; ++j) {
      int idx = static_cast<int>(i) + j;
      if (idx >= 0 && idx < static_cast<int>(histogram.size())) {
        sum += histogram[static_cast<size_t>(idx)];
        ++count;
      }
    }
    out[i] = count > 0 ? sum / static_cast<float>(count) : 0.0f;
  }
  return out;
}

VfhResult VfhPlanner::plan(const std::vector<ObstaclePoint> & points) const
{
  VfhResult result;
  result.sector_resolution_rad = params_.sector_resolution_rad;
  result.fov_half_angle_rad = params_.fov_half_angle_rad;

  const int n_sectors = std::max(
    3, static_cast<int>(std::round(
      (2.0 * params_.fov_half_angle_rad) / params_.sector_resolution_rad)) + 1);

  std::vector<float> raw = buildHistogram(points, n_sectors);
  std::vector<float> hist = smooth(raw);
  result.histogram = hist;

  // Track minimum clearance across the whole FOV for TTC/braking upstream.
  double min_clearance = params_.max_range_m;
  for (const auto & p : points) {
    if (std::abs(p.angle_rad) < 0.35 && p.range_m > 0.0) {  // ~20 deg forward cone
      min_clearance = std::min(min_clearance, p.range_m);
    }
  }
  result.min_clearance_m = min_clearance;

  auto sector_angle = [&](int idx) {
    return -params_.fov_half_angle_rad + idx * params_.sector_resolution_rad;
  };
  auto min_range_in_sector = [&](int idx) {
    double min_r = params_.max_range_m;
    double center = sector_angle(idx);
    for (const auto & p : points) {
      if (std::abs(p.angle_rad - center) <= params_.sector_resolution_rad * 0.5) {
        min_r = std::min(min_r, p.range_m);
      }
    }
    return min_r;
  };

  // Find contiguous free-sector runs ("valleys") in the binary histogram.
  struct Valley {int start; int end;};
  std::vector<Valley> valleys;
  int run_start = -1;
  for (int i = 0; i < n_sectors; ++i) {
    bool free = hist[static_cast<size_t>(i)] <= params_.obstacle_threshold;
    if (free && run_start < 0) {
      run_start = i;
    } else if (!free && run_start >= 0) {
      valleys.push_back({run_start, i - 1});
      run_start = -1;
    }
  }
  if (run_start >= 0) {valleys.push_back({run_start, n_sectors - 1});}

  double best_score = std::numeric_limits<double>::max();
  bool found_valid = false;
  double best_angle = 0.0;

  for (const auto & v : valleys) {
    double width_rad = (v.end - v.start + 1) * params_.sector_resolution_rad;
    double nearest_obstacle_range = params_.max_range_m;
    for (int i = v.start; i <= v.end; ++i) {
      nearest_obstacle_range = std::min(nearest_obstacle_range, min_range_in_sector(i));
    }

    double required_half_angle = std::atan2(
      params_.vehicle_half_width_m + params_.safety_margin_m,
      std::max(nearest_obstacle_range, 0.05));
    bool wide_enough = width_rad >= 2.0 * required_half_angle;
    if (!wide_enough) {continue;}

    double center_angle = 0.5 * (sector_angle(v.start) + sector_angle(v.end));
    double score = std::abs(center_angle - params_.goal_direction_rad);
    if (score < best_score) {
      best_score = score;
      best_angle = center_angle;
      found_valid = true;
    }
  }

  result.path_blocked = !found_valid;
  result.steering_bias_rad = found_valid ? best_angle : 0.0;
  return result;
}

}  // namespace obstacle_detection
