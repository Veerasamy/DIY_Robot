#pragma once

#include <mutex>
#include <vector>

#include "ackermann_msgs/msg/ackermann_drive_stamped.hpp"
#include "controller/controller_factory.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "nav_msgs/msg/path.hpp"
#include "rclcpp/rclcpp.hpp"
#include "safety_manager/msg/vehicle_state.hpp"
#include "std_msgs/msg/float32.hpp"

namespace controller
{

class PurePursuitNode : public rclcpp::Node
{
public:
  PurePursuitNode();

private:
  void onCenterline(const nav_msgs::msg::Path::ConstSharedPtr & msg);
  void onCurvature(const std_msgs::msg::Float32::ConstSharedPtr & msg);
  void onOdom(const nav_msgs::msg::Odometry::ConstSharedPtr & msg);
  void onVehicleState(const safety_manager::msg::VehicleState::ConstSharedPtr & msg);
  void onObstacleAvoidance(const geometry_msgs::msg::Twist::ConstSharedPtr & msg);
  void onControlTimer();

  double speedProfile(double curvature_radius_m) const;

  // Swappable via the `controller_type` parameter (pure_pursuit|stanley|mpc)
  // -- see controller_factory.hpp. Steering-angle clamping is done at the
  // node level (max_steering_angle_rad_) since not every ISteeringController
  // implementation exposes the same params() struct.
  std::unique_ptr<ISteeringController> steering_controller_;
  double max_steering_angle_rad_{0.2812};  // measured: atan(1.3cm max deviation / 4.5cm wheelbase) = 16.1 deg

  // Steering slew-rate limit (Section E): bounds how fast the *combined*
  // steering command (base controller + obstacle bias) can change per
  // control cycle, regardless of source -- without this, a VFH valley
  // switch or a noisy centerline jump can command a near-instant angle
  // change that exceeds the tires' available lateral grip and skids.
  double max_steering_rate_rad_s_{3.0};  // placeholder -- MEASURE the Savox 640's actual slew rate
  double last_steering_rad_{0.0};
  rclcpp::Time last_control_time_;

  std::mutex data_mutex_;
  std::vector<PathPoint> path_;
  double curvature_radius_m_{1e6};
  double current_speed_mps_{0.0};
  double obstacle_steering_bias_rad_{0.0};
  // obstacle_avoidance_node's requested speed multiplier (1.0 = no
  // restriction .. 0.0 = full stop), continuous with proximity -- see
  // ObstacleAvoidanceNode::onControlTimer.
  double obstacle_speed_scale_{1.0};
  uint8_t vehicle_state_{0};  // safety_manager::msg::VehicleState::NORMAL

  double speed_p_gain_{0.0};
  double prev_speed_error_{0.0};

  // Speed profile tuning (Section E / K)
  double max_lateral_accel_mps2_{4.0};  // tune on-track; start conservative
  double v_min_mps_{0.5};
  double v_max_mps_{8.0};

  rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr centerline_sub_;
  rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr curvature_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<safety_manager::msg::VehicleState>::SharedPtr vehicle_state_sub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr obstacle_sub_;
  rclcpp::Publisher<ackermann_msgs::msg::AckermannDriveStamped>::SharedPtr cmd_pub_;
  rclcpp::TimerBase::SharedPtr control_timer_;
};

}  // namespace controller
