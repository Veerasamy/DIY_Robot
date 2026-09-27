// Loads a pre-computed global racing line (map-frame waypoints, optionally
// with a per-waypoint target speed) from a YAML file. Pure data/algorithm
// class, no ROS dependency (unit-testable in isolation, matching the
// VfhPlanner/PurePursuitController convention elsewhere in this workspace).
//
// This package does not compute an optimal racing line from scratch --
// that requires an offline optimizer (e.g. minimum-curvature or minimum-time
// solved against a track boundary from spatial mapping) run once per course
// ahead of the competition, which is out of scope for an onboard ROS2 node.
// `config/raceline.yaml` ships a placeholder straight-line stand-in; replace
// it with the actual course's optimized waypoints before racing (see the
// package README for the recommended offline workflow).
#pragma once

#include <string>
#include <vector>

namespace planner
{

struct Waypoint
{
  double x{0.0};  // map frame, meters
  double y{0.0};
  double target_speed_mps{0.0};  // 0 = no override, use the controller's own speed profile
};

class RacelineLoader
{
public:
  // Returns false (and logs nothing itself -- caller decides how to report)
  // if the file doesn't exist or has no "raceline" sequence.
  bool loadFromYaml(const std::string & path);

  const std::vector<Waypoint> & waypoints() const {return waypoints_;}

private:
  std::vector<Waypoint> waypoints_;
};

}  // namespace planner
