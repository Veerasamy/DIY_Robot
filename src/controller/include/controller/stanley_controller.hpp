// Stanley steering controller (Stanford DARPA Grand Challenge, Hoffmann
// et al. 2007), Section D reference implementation for A/B comparison
// against Pure Pursuit (see ARCHITECTURE.md section 4).
//
// Mathematical derivation:
//   The front axle sits at (wheelbase_m, 0) in the vehicle's rear-axle body
//   frame (vehicle heading defines the local +x axis, so any path tangent
//   angle measured in this frame *is* the heading error directly). Find the
//   path segment nearest the front axle, giving:
//       heading_error   = psi_path  (path tangent angle vs. vehicle heading)
//       cross_track_err = signed perpendicular distance from the front axle
//                         to that segment (+ = front axle is to the right of
//                         the path, i.e. a positive/left correction is needed)
//   Then:
//       delta = heading_error + atan2(k_e * cross_track_err, k_soft + v)
//   The atan2 term drives cross-track error to zero with a gain that
//   softens (via k_soft) at low speed to avoid a divide-by-zero/over-reaction
//   when nearly stationary.
#pragma once

#include "controller/steering_controller_interface.hpp"

namespace controller
{

struct StanleyParams
{
  double wheelbase_m{0.045};          // measured: 4.5cm wheelbase
  double cross_track_gain{1.0};      // k_e
  double softening_speed_mps{1.0};   // k_soft
  double max_steering_angle_rad{0.2812};  // measured: atan(1.3cm max deviation / 4.5cm wheelbase) = 16.1 deg
};

class StanleyController : public ISteeringController
{
public:
  explicit StanleyController(const StanleyParams & params) : params_(params) {}

  double computeSteeringAngle(
    const std::vector<PathPoint> & path_body_frame,
    const VehicleState & state) override;

  void setParams(const StanleyParams & params) {params_ = params;}
  const StanleyParams & params() const {return params_;}

  double lastCrossTrackErrorM() const {return last_cross_track_error_m_;}
  double lastHeadingErrorRad() const {return last_heading_error_rad_;}

private:
  StanleyParams params_;
  double last_cross_track_error_m_{0.0};
  double last_heading_error_rad_{0.0};
};

}  // namespace controller
