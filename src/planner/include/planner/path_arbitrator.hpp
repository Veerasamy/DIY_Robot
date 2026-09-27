// Local/global path arbitration layer (ARCHITECTURE.md section 7:
// "global racing-line planner + local planner arbitration layer above
// obstacle_detection"). Pure algorithm class, no ROS dependency.
//
// Speed Race Mode: `lane_detection`'s centerline is ground-truth for the
// drivable corridor (it directly observes the painted lane/road edges), so
// it is the primary source; the global raceline only nudges the path
// laterally toward the apex-hugging line within that corridor
// (`blend_weight`, 0 = pure lane centerline, 1 = pure raceline).
//
// Obstacle Race Mode: there may be no lane markings at all, so the global
// raceline (pre-planned for the known course) is primary, with the lane
// centerline (if lane_detection still finds anything) used as the fallback.
//
// In both modes, `obstacle_path_blocked` (from obstacle_detection's VFH
// result) invalidates the arbitrated path outright rather than trying to
// route around it here -- steering avoidance itself is `obstacle_detection`
// and `recovery_manager`'s job; this layer only decides *which* path source
// to hand to the controller and whether it's currently trustworthy.
#pragma once

#include <string>
#include <vector>

namespace planner
{

struct PathPoint2D
{
  double x{0.0};  // vehicle body frame: forward
  double y{0.0};  // left
};

struct ArbitrationInput
{
  std::vector<PathPoint2D> lane_centerline_body;  // from /lane/centerline, body frame
  std::vector<PathPoint2D> global_path_body;       // raceline window, transformed to body frame
  bool obstacle_path_blocked{false};               // from /obstacle/avoidance_cmd linear.x==0
  std::string race_mode{"speed"};                   // "speed" | "obstacle"
  double blend_weight{0.3};                         // 0=pure centerline .. 1=pure raceline (speed mode only)
};

struct ArbitrationResult
{
  std::vector<PathPoint2D> local_path_body;
  bool path_valid{true};
  std::string source;  // "lane_centerline" | "global_raceline" | "blended", for diagnostics
};

class PathArbitrator
{
public:
  ArbitrationResult arbitrate(const ArbitrationInput & in) const;
};

}  // namespace planner
