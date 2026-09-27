// Abstract steering-controller interface (Section D). Allows Pure Pursuit /
// Stanley / MPC to be swapped without touching pure_pursuit_node.cpp — only
// this interface and the concrete implementation change.
#pragma once

#include <vector>

namespace controller
{

// A point on the reference path, in the vehicle body frame:
// x = forward (m), y = lateral, + = left (REP-103 convention).
struct PathPoint
{
  double x{0.0};
  double y{0.0};
};

struct VehicleState
{
  double speed_mps{0.0};
};

class ISteeringController
{
public:
  virtual ~ISteeringController() = default;

  // Returns the desired front-wheel steering angle in radians
  // (+ = left, Ackermann convention), given the reference path in the
  // vehicle body frame and the current vehicle state.
  virtual double computeSteeringAngle(
    const std::vector<PathPoint> & path_body_frame,
    const VehicleState & state) = 0;
};

}  // namespace controller
