#include "sensor_fusion/ekf_node.hpp"

#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Quaternion.h>

namespace sensor_fusion
{

EkfNode::EkfNode()
: Node("sensor_fusion_ekf")
{
  odom_frame_id_ = declare_parameter<std::string>("odom_frame_id", "odom");
  base_frame_id_ = declare_parameter<std::string>("base_frame_id", "base_link");
  encoder_variance_ = declare_parameter<double>("encoder_variance", 0.01);
  imu_yaw_rate_variance_ = declare_parameter<double>("imu_yaw_rate_variance", 0.0009);
  publish_tf_ = declare_parameter<bool>("publish_tf", true);

  double q_pos = declare_parameter<double>("process_noise_pos", 0.01);
  double q_theta = declare_parameter<double>("process_noise_theta", 0.005);
  double q_v = declare_parameter<double>("process_noise_v", 0.2);
  double q_omega = declare_parameter<double>("process_noise_omega", 0.1);
  ekf_.setProcessNoise(q_pos, q_theta, q_v, q_omega);

  vo_sub_ = create_subscription<nav_msgs::msg::Odometry>(
    "/zed/odom/vo", rclcpp::QoS(10),
    std::bind(&EkfNode::onVisualOdometry, this, std::placeholders::_1));
  imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
    "/zed/imu/data", rclcpp::SensorDataQoS(),
    std::bind(&EkfNode::onImu, this, std::placeholders::_1));
  encoder_sub_ = create_subscription<std_msgs::msg::Float32>(
    "/encoder/velocity_mps", rclcpp::QoS(10),
    std::bind(&EkfNode::onEncoderSpeed, this, std::placeholders::_1));

  odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("/ekf/odom", rclcpp::QoS(10));
  if (publish_tf_) {
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);
  }

  RCLCPP_INFO(get_logger(), "sensor_fusion EKF started (state: x,y,theta,v,omega)");
}

void EkfNode::predictToNow()
{
  const rclcpp::Time t_now = now();
  if (!initialized_) {
    last_predict_time_ = t_now;
    initialized_ = true;
    return;
  }
  double dt = (t_now - last_predict_time_).seconds();
  ekf_.predict(dt);
  last_predict_time_ = t_now;
}

void EkfNode::onVisualOdometry(const nav_msgs::msg::Odometry::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(ekf_mutex_);
  predictToNow();

  tf2::Quaternion q(
    msg->pose.pose.orientation.x, msg->pose.pose.orientation.y,
    msg->pose.pose.orientation.z, msg->pose.pose.orientation.w);
  double roll, pitch, yaw;
  tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);

  Eigen::Vector3d z(msg->pose.pose.position.x, msg->pose.pose.position.y, yaw);
  Eigen::Matrix3d R;
  R.setZero();
  R(0, 0) = msg->pose.covariance[0] > 0 ? msg->pose.covariance[0] : 0.01;
  R(1, 1) = msg->pose.covariance[7] > 0 ? msg->pose.covariance[7] : 0.01;
  R(2, 2) = msg->pose.covariance[35] > 0 ? msg->pose.covariance[35] : 0.02;

  ekf_.updateVisualOdometry(z, R);
  publishState(msg->header.stamp);
}

void EkfNode::onImu(const sensor_msgs::msg::Imu::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(ekf_mutex_);
  predictToNow();
  ekf_.updateImuYawRate(msg->angular_velocity.z, imu_yaw_rate_variance_);
  publishState(msg->header.stamp);
}

void EkfNode::onEncoderSpeed(const std_msgs::msg::Float32::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(ekf_mutex_);
  predictToNow();
  ekf_.updateEncoderSpeed(msg->data, encoder_variance_);
  publishState(now());
}

void EkfNode::publishState(const rclcpp::Time & stamp)
{
  const auto & x = ekf_.state();
  const auto & P = ekf_.covariance();

  nav_msgs::msg::Odometry odom;
  odom.header.stamp = stamp;
  odom.header.frame_id = odom_frame_id_;
  odom.child_frame_id = base_frame_id_;
  odom.pose.pose.position.x = x(0);
  odom.pose.pose.position.y = x(1);

  tf2::Quaternion q;
  q.setRPY(0, 0, x(2));
  odom.pose.pose.orientation.x = q.x();
  odom.pose.pose.orientation.y = q.y();
  odom.pose.pose.orientation.z = q.z();
  odom.pose.pose.orientation.w = q.w();

  odom.twist.twist.linear.x = x(3);
  odom.twist.twist.angular.z = x(4);

  for (int i = 0; i < 36; ++i) {odom.pose.covariance[i] = 0.0;}
  odom.pose.covariance[0] = P(0, 0);
  odom.pose.covariance[7] = P(1, 1);
  odom.pose.covariance[35] = P(2, 2);

  odom_pub_->publish(odom);

  if (publish_tf_) {
    geometry_msgs::msg::TransformStamped tf_msg;
    tf_msg.header.stamp = stamp;
    tf_msg.header.frame_id = odom_frame_id_;
    tf_msg.child_frame_id = base_frame_id_;
    tf_msg.transform.translation.x = x(0);
    tf_msg.transform.translation.y = x(1);
    tf_msg.transform.rotation = odom.pose.pose.orientation;
    tf_broadcaster_->sendTransform(tf_msg);
  }
}

}  // namespace sensor_fusion
