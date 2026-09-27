// ROS2 node wiring: runs the DWA planner exclusively during safety_manager's
// REVERSE and RECOVER vehicle states, and reports back to safety_manager via
// /safety/state_request (3=path_clear, 4=recovered) once each maneuver's
// exit condition is met -- see safety_manager's vehicle_state_machine.hpp
// for the full state diagram this drives.
#pragma once

#include <mutex>
#include <utility>
#include <vector>

#include "ackermann_msgs/msg/ackermann_drive_stamped.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "nav_msgs/msg/path.hpp"
#include "rclcpp/rclcpp.hpp"
#include "recovery_manager/dwa_planner.hpp"
#include "safety_manager/msg/vehicle_state.hpp"
#include "std_msgs/msg/u_int8.hpp"

namespace recovery_manager
{

class RecoveryManagerNode : public rclcpp::Node
{
public:
  RecoveryManagerNode();

private:
  void onVehicleState(const safety_manager::msg::VehicleState::ConstSharedPtr & msg);
  void onCostmap(const nav_msgs::msg::OccupancyGrid::ConstSharedPtr & msg);
  void onOdom(const nav_msgs::msg::Odometry::ConstSharedPtr & msg);
  void onCenterline(const nav_msgs::msg::Path::ConstSharedPtr & msg);
  void onControlTimer();

  double centerlineHeadingErrorRad() const;
  void publishCommand(double speed_mps, double steering_angle_rad);
  void publishStop();

  std::unique_ptr<DwaPlanner> reverse_planner_;   // narrow negative-only speed window
  std::unique_ptr<DwaPlanner> recover_planner_;   // narrow positive-creep + full yaw window

  std::mutex data_mutex_;
  uint8_t vehicle_state_{0};
  uint8_t prev_vehicle_state_{0};
  rclcpp::Time state_entered_time_;
  OccupancyGridView grid_;
  bool grid_received_{false};
  double current_speed_mps_{0.0};
  double current_yaw_rate_rad_s_{0.0};
  std::vector<std::pair<double, double>> centerline_body_;  // (x, y), nearest-to-farthest

  double reverse_min_duration_sec_{1.0};
  double reverse_clear_distance_m_{1.0};
  double recovered_heading_tol_rad_{0.15};

  rclcpp::Subscription<safety_manager::msg::VehicleState>::SharedPtr vehicle_state_sub_;
  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr costmap_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr centerline_sub_;
  rclcpp::Publisher<ackermann_msgs::msg::AckermannDriveStamped>::SharedPtr cmd_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr state_request_pub_;
  rclcpp::TimerBase::SharedPtr control_timer_;
};

}  // namespace recovery_manager
