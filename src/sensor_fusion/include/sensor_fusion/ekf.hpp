// Extended Kalman Filter fusing ZED visual odometry, ZED IMU (yaw rate) and
// wheel encoder velocity. Pure math class, no ROS dependency (unit
// testable). See ARCHITECTURE.md section 6 for the full derivation.
//
// State vector: x = [px, py, theta, v, omega]^T
//   px, py  - position in the odom frame (m)
//   theta   - heading (rad, wrapped to [-pi, pi])
//   v       - body-frame forward speed (m/s)
//   omega   - yaw rate (rad/s)
//
// Motion model (unicycle/bicycle-equivalent since v, omega are already the
// track-level linear/angular rates):
//   px'    = px + v*cos(theta)*dt
//   py'    = py + v*sin(theta)*dt
//   theta' = theta + omega*dt
//   v'     = v
//   omega' = omega
#pragma once

#include <Eigen/Dense>

namespace sensor_fusion
{

class EKF
{
public:
  static constexpr int kStateDim = 5;
  using StateVec = Eigen::Matrix<double, kStateDim, 1>;
  using StateCov = Eigen::Matrix<double, kStateDim, kStateDim>;

  EKF();

  void reset(const StateVec & x0, const StateCov & p0);

  // Predicts the state forward by dt seconds using the motion model above.
  // Safe to call with dt <= 0 (no-op) to simplify callers that compute dt
  // from wall-clock timestamps.
  void predict(double dt);

  // z = [px, py, theta] from ZED visual odometry. R is the 3x3 measurement
  // noise covariance (from the ZED SDK's pose confidence, see zed_interface).
  void updateVisualOdometry(const Eigen::Vector3d & z, const Eigen::Matrix3d & R);

  // z = yaw rate (rad/s) from the ZED IMU gyro-z.
  void updateImuYawRate(double z, double r_variance);

  // z = forward speed (m/s) from the wheel encoder tick-delta/dt.
  void updateEncoderSpeed(double z, double r_variance);

  const StateVec & state() const {return x_;}
  const StateCov & covariance() const {return P_;}

  // Process noise, tunable at runtime (exposed as ROS parameters by ekf_node).
  void setProcessNoise(double q_pos, double q_theta, double q_v, double q_omega);

private:
  static double wrapAngle(double a);

  StateVec x_;
  StateCov P_;
  StateCov Q_;  // process noise, diagonal
};

}  // namespace sensor_fusion
