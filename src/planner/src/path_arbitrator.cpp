#include "planner/path_arbitrator.hpp"

#include <algorithm>

namespace planner
{

ArbitrationResult PathArbitrator::arbitrate(const ArbitrationInput & in) const
{
  ArbitrationResult result;

  bool have_lane = !in.lane_centerline_body.empty();
  bool have_global = !in.global_path_body.empty();

  if (in.race_mode == "speed") {
    if (have_lane && have_global) {
      // Blend index-by-index over the common prefix (both sequences are
      // already ordered nearest-to-farthest in the body frame) -- a
      // lightweight approximation to a proper arclength resample, adequate
      // given both paths cover the same short lookahead horizon.
      size_t n = std::min(in.lane_centerline_body.size(), in.global_path_body.size());
      result.local_path_body.reserve(n);
      double w = std::clamp(in.blend_weight, 0.0, 1.0);
      for (size_t i = 0; i < n; ++i) {
        const auto & lane = in.lane_centerline_body[i];
        const auto & race = in.global_path_body[i];
        result.local_path_body.push_back(
          PathPoint2D{
            lane.x * (1.0 - w) + race.x * w,
            lane.y * (1.0 - w) + race.y * w});
      }
      // Append any remaining lane points beyond the raceline window so the
      // path doesn't truncate short.
      for (size_t i = n; i < in.lane_centerline_body.size(); ++i) {
        result.local_path_body.push_back(in.lane_centerline_body[i]);
      }
      result.source = "blended";
    } else if (have_lane) {
      result.local_path_body = in.lane_centerline_body;
      result.source = "lane_centerline";
    } else if (have_global) {
      result.local_path_body = in.global_path_body;
      result.source = "global_raceline";
    }
  } else {  // "obstacle" mode
    if (have_global) {
      result.local_path_body = in.global_path_body;
      result.source = "global_raceline";
    } else if (have_lane) {
      result.local_path_body = in.lane_centerline_body;
      result.source = "lane_centerline";
    }
  }

  result.path_valid = !in.obstacle_path_blocked && !result.local_path_body.empty();
  return result;
}

}  // namespace planner
