#pragma once

#include <mutex>

#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_fusion/ekf.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "std_msgs/msg/float32.hpp"
#include "tf2_ros/transform_broadcaster.h"

namespace sensor_fusion
{

class EkfNode : public rclcpp::Node
{
public:
  EkfNode();

private:
  void onVisualOdometry(const nav_msgs::msg::Odometry::ConstSharedPtr & msg);
  void onImu(const sensor_msgs::msg::Imu::ConstSharedPtr & msg);
  void onEncoderSpeed(const std_msgs::msg::Float32::ConstSharedPtr & msg);
  void predictToNow();
  void publishState(const rclcpp::Time & stamp);

  std::mutex ekf_mutex_;
  EKF ekf_;
  rclcpp::Time last_predict_time_;
  bool initialized_{false};

  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr vo_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
  rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr encoder_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  std::string odom_frame_id_;
  std::string base_frame_id_;
  double encoder_variance_{0.01};
  double imu_yaw_rate_variance_{0.0009};  // ~1.7 deg/s std dev, typical MEMS gyro
  bool publish_tf_{true};
};

}  // namespace sensor_fusion
