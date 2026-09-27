// Lightweight Model Predictive Control steering controller (Section D,
// future work per ARCHITECTURE.md section 4 -- now implemented).
//
// Rather than depending on an external QP solver (kept out of this repo's
// build per the "standard toolchain only" packaging constraint -- see
// ApplicationGeneration.md), this solves the receding-horizon problem with
// derivative-free pattern search (Hooke-Jeeves) directly against the true
// nonlinear kinematic bicycle model, instead of linearizing and dispatching
// to a QP. This is adequate at the SCRT10's speed/curvature envelope and the
// Orin Nano's compute budget (default horizon_steps=8 @ dt=0.05s = 0.4s
// lookahead) and keeps the class swap-in compatible with
// `ISteeringController`.
//
// Model (per step k, constant measured speed v held over the horizon):
//   x_{k+1}     = x_k + v*cos(theta_k)*dt
//   y_{k+1}     = y_k + v*sin(theta_k)*dt
//   theta_{k+1} = theta_k + (v/L)*tan(delta_k)*dt
// Cost (minimized over the steering sequence delta_0..delta_{N-1}):
//   J = sum_k [ q_cte*cte_k^2 + q_heading*heading_k^2 + r_delta*delta_k^2
//               + r_ddelta*(delta_k - delta_{k-1})^2 ]
// where cte_k/heading_k are the simulated position's cross-track/heading
// error against the nearest reference path segment (same projection as
// `StanleyController`).
//
// If a production build has QP/solver headroom, swap this class for a
// linearized MPC dispatched to OSQP/qpOASES without touching the node --
// that is exactly the point of `ISteeringController`.
#pragma once

#include <cstddef>
#include <vector>

#include "controller/steering_controller_interface.hpp"

namespace controller
{

struct MpcParams
{
  double wheelbase_m{0.045};       // measured: 4.5cm wheelbase
  int horizon_steps{8};
  double dt_sec{0.05};
  double q_cross_track{5.0};
  double q_heading{3.0};
  double r_steering{0.5};
  double r_steering_rate{1.0};
  int optimization_iterations{15};
  double initial_search_step_rad{0.12};
  double min_search_step_rad{0.005};
  double max_steering_angle_rad{0.2812};  // measured: atan(1.3cm max deviation / 4.5cm wheelbase) = 16.1 deg
};

class MpcController : public ISteeringController
{
public:
  explicit MpcController(const MpcParams & params)
  : params_(params), warm_start_(static_cast<size_t>(params.horizon_steps), 0.0) {}

  double computeSteeringAngle(
    const std::vector<PathPoint> & path_body_frame,
    const VehicleState & state) override;

  void setParams(const MpcParams & params);
  const MpcParams & params() const {return params_;}

private:
  struct TrackingError
  {
    double cross_track_m{0.0};
    double heading_rad{0.0};
  };

  TrackingError nearestPathError(
    const std::vector<PathPoint> & path_body_frame, const PathPoint & position) const;

  double simulateCost(
    const std::vector<PathPoint> & path_body_frame,
    const std::vector<double> & steering_sequence,
    double speed_mps) const;

  MpcParams params_;
  std::vector<double> warm_start_;
};

}  // namespace controller
