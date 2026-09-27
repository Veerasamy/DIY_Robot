#include "sensor_fusion/ekf.hpp"

#include <cmath>

namespace sensor_fusion
{

EKF::EKF()
{
  x_.setZero();
  P_ = StateCov::Identity() * 1.0;
  Q_ = StateCov::Identity();
  setProcessNoise(0.01, 0.005, 0.2, 0.1);
}

void EKF::reset(const StateVec & x0, const StateCov & p0)
{
  x_ = x0;
  P_ = p0;
}

void EKF::setProcessNoise(double q_pos, double q_theta, double q_v, double q_omega)
{
  Q_.setZero();
  Q_(0, 0) = q_pos;
  Q_(1, 1) = q_pos;
  Q_(2, 2) = q_theta;
  Q_(3, 3) = q_v;      // larger uncertainty: no acceleration input in the model
  Q_(4, 4) = q_omega;
}

double EKF::wrapAngle(double a)
{
  while (a > M_PI) {a -= 2.0 * M_PI;}
  while (a < -M_PI) {a += 2.0 * M_PI;}
  return a;
}

void EKF::predict(double dt)
{
  if (dt <= 0.0) {return;}

  const double theta = x_(2);
  const double v = x_(3);
  const double omega = x_(4);
  const double c = std::cos(theta);
  const double s = std::sin(theta);

  StateVec x_pred = x_;
  x_pred(0) = x_(0) + v * c * dt;
  x_pred(1) = x_(1) + v * s * dt;
  x_pred(2) = wrapAngle(x_(2) + omega * dt);
  x_pred(3) = v;
  x_pred(4) = omega;

  // Jacobian of the motion model w.r.t. the state, evaluated at the prior.
  StateCov F = StateCov::Identity();
  F(0, 2) = -v * s * dt;
  F(0, 3) = c * dt;
  F(1, 2) = v * c * dt;
  F(1, 3) = s * dt;
  F(2, 4) = dt;

  x_ = x_pred;
  P_ = F * P_ * F.transpose() + Q_ * dt;
}

void EKF::updateVisualOdometry(const Eigen::Vector3d & z, const Eigen::Matrix3d & R)
{
  Eigen::Matrix<double, 3, kStateDim> H = Eigen::Matrix<double, 3, kStateDim>::Zero();
  H(0, 0) = 1.0;
  H(1, 1) = 1.0;
  H(2, 2) = 1.0;

  Eigen::Vector3d y;
  y(0) = z(0) - x_(0);
  y(1) = z(1) - x_(1);
  y(2) = wrapAngle(z(2) - x_(2));

  Eigen::Matrix3d S = H * P_ * H.transpose() + R;
  Eigen::Matrix<double, kStateDim, 3> K = P_ * H.transpose() * S.inverse();

  x_ = x_ + K * y;
  x_(2) = wrapAngle(x_(2));
  P_ = (StateCov::Identity() - K * H) * P_;
}

void EKF::updateImuYawRate(double z, double r_variance)
{
  Eigen::Matrix<double, 1, kStateDim> H = Eigen::Matrix<double, 1, kStateDim>::Zero();
  H(0, 4) = 1.0;

  double y = z - x_(4);
  double S = (H * P_ * H.transpose())(0, 0) + r_variance;
  Eigen::Matrix<double, kStateDim, 1> K = P_ * H.transpose() / S;

  x_ = x_ + K * y;
  x_(2) = wrapAngle(x_(2));
  P_ = (StateCov::Identity() - K * H) * P_;
}

void EKF::updateEncoderSpeed(double z, double r_variance)
{
  Eigen::Matrix<double, 1, kStateDim> H = Eigen::Matrix<double, 1, kStateDim>::Zero();
  H(0, 3) = 1.0;

  double y = z - x_(3);
  double S = (H * P_ * H.transpose())(0, 0) + r_variance;
  Eigen::Matrix<double, kStateDim, 1> K = P_ * H.transpose() / S;

  x_ = x_ + K * y;
  x_(2) = wrapAngle(x_(2));
  P_ = (StateCov::Identity() - K * H) * P_;
}

}  // namespace sensor_fusion
