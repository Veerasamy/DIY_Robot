// Pure Pursuit steering controller.
//
// Mathematical derivation:
//   Given a lookahead point (x_t, y_t) in the vehicle's rear-axle frame at
//   distance Ld = sqrt(x_t^2 + y_t^2), the arc that passes through the
//   origin and the lookahead point with the vehicle's current heading as
//   tangent has curvature:
//       kappa = 2*y_t / Ld^2
//   For a bicycle model with wheelbase L, the required front-wheel steering
//   angle to track that arc is:
//       delta = atan(kappa * L) = atan2(2*L*y_t, Ld^2)
//   which is algebraically identical to the more common form
//       delta = atan2(2*L*sin(alpha), Ld),  alpha = atan2(y_t, x_t)
//   since y_t = Ld*sin(alpha) and Ld^2 = Ld*Ld.
//
// Dynamic lookahead: Ld = clamp(k_ld * v + Ld_min, Ld_min, Ld_max). Faster
// speeds need a longer lookahead to remain stable (too-short Ld at speed
// causes steering oscillation); slower speeds need a shorter Ld to actually
// cut corners tightly instead of understeering wide.
#pragma once

#include "controller/steering_controller_interface.hpp"

namespace controller
{

struct PurePursuitParams
{
  double wheelbase_m{0.045};  // measured: 4.5cm wheelbase
  double lookahead_min_m{0.08};
  double lookahead_max_m{0.25};
  double lookahead_speed_gain{0.02125};  // Ld = gain * v + min -- spans min..max across v_min_mps..v_max_mps
  double max_steering_angle_rad{0.2812};  // measured: atan(1.3cm max deviation / 4.5cm wheelbase) = 16.1 deg
};

class PurePursuitController : public ISteeringController
{
public:
  explicit PurePursuitController(const PurePursuitParams & params) : params_(params) {}

  double computeSteeringAngle(
    const std::vector<PathPoint> & path_body_frame,
    const VehicleState & state) override;

  void setParams(const PurePursuitParams & params) {params_ = params;}
  const PurePursuitParams & params() const {return params_;}

  double lastLookaheadDistance() const {return last_lookahead_m_;}

private:
  PathPoint findLookaheadPoint(
    const std::vector<PathPoint> & path_body_frame, double lookahead_m) const;

  PurePursuitParams params_;
  double last_lookahead_m_{0.0};
};

}  // namespace controller
