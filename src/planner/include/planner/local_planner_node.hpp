// ROS2 node wiring for the planner package: transforms the loaded global
// raceline into the vehicle body frame around the current EKF pose, feeds
// it plus /lane/centerline into PathArbitrator, and publishes the result.
#pragma once

#include <mutex>

#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "nav_msgs/msg/path.hpp"
#include "planner/path_arbitrator.hpp"
#include "planner/raceline_loader.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/u_int8.hpp"

namespace planner
{

class LocalPlannerNode : public rclcpp::Node
{
public:
  LocalPlannerNode();

private:
  void onOdom(const nav_msgs::msg::Odometry::ConstSharedPtr & msg);
  void onCenterline(const nav_msgs::msg::Path::ConstSharedPtr & msg);
  void onObstacleAvoidance(const geometry_msgs::msg::Twist::ConstSharedPtr & msg);
  void onControlTimer();

  void publishGlobalPath();
  std::vector<PathPoint2D> globalWindowInBodyFrame() const;

  RacelineLoader raceline_;
  PathArbitrator arbitrator_;

  // mutable: locked from globalWindowInBodyFrame(), which is const.
  mutable std::mutex data_mutex_;
  double pose_x_{0.0};
  double pose_y_{0.0};
  double pose_theta_{0.0};
  std::vector<PathPoint2D> lane_centerline_body_;
  bool obstacle_path_blocked_{false};

  std::string race_mode_{"speed"};
  double lookahead_window_m_{15.0};
  double blend_weight_{0.3};

  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr centerline_sub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr obstacle_sub_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr local_path_pub_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr global_path_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr state_request_pub_;
  rclcpp::TimerBase::SharedPtr control_timer_;
};

}  // namespace planner
