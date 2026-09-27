#include "recovery_manager/recovery_manager_node.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace recovery_manager
{

RecoveryManagerNode::RecoveryManagerNode()
: Node("recovery_manager")
{
  DwaParams reverse_params;
  reverse_params.wheelbase_m = declare_parameter<double>("wheelbase_m", 0.045);
  reverse_params.max_steering_angle_rad = declare_parameter<double>("max_steering_angle_rad", 0.2812);
  reverse_params.max_speed_mps = 0.0;
  reverse_params.min_speed_mps = -std::abs(declare_parameter<double>("reverse_speed_mps", 0.5));
  reverse_params.max_accel_mps2 = declare_parameter<double>("max_accel_mps2", 1.0);
  reverse_params.max_yaw_rate_rad_s = declare_parameter<double>("max_yaw_rate_rad_s", 1.5);
  reverse_params.max_yaw_accel_rad_s2 = declare_parameter<double>("max_yaw_accel_rad_s2", 2.5);
  reverse_params.vehicle_radius_m = declare_parameter<double>("vehicle_radius_m", 0.3);
  reverse_params.collision_margin_m = declare_parameter<double>("collision_margin_m", 0.1);
  reverse_planner_ = std::make_unique<DwaPlanner>(reverse_params);

  DwaParams recover_params = reverse_params;
  recover_params.min_speed_mps = 0.0;
  recover_params.max_speed_mps = std::abs(declare_parameter<double>("recover_creep_speed_mps", 0.4));
  recover_planner_ = std::make_unique<DwaPlanner>(recover_params);

  reverse_min_duration_sec_ = declare_parameter<double>("reverse_min_duration_sec", 1.0);
  reverse_clear_distance_m_ = declare_parameter<double>("reverse_clear_distance_m", 1.0);
  recovered_heading_tol_rad_ = declare_parameter<double>("recovered_heading_tol_rad", 0.15);

  double control_rate_hz = declare_parameter<double>("control_rate_hz", 20.0);

  vehicle_state_sub_ = create_subscription<safety_manager::msg::VehicleState>(
    "/safety/vehicle_state", rclcpp::QoS(10),
    std::bind(&RecoveryManagerNode::onVehicleState, this, std::placeholders::_1));
  costmap_sub_ = create_subscription<nav_msgs::msg::OccupancyGrid>(
    "/obstacle/costmap", rclcpp::QoS(1),
    std::bind(&RecoveryManagerNode::onCostmap, this, std::placeholders::_1));
  odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
    "/ekf/odom", rclcpp::QoS(10),
    std::bind(&RecoveryManagerNode::onOdom, this, std::placeholders::_1));
  centerline_sub_ = create_subscription<nav_msgs::msg::Path>(
    "/lane/centerline", rclcpp::QoS(10),
    std::bind(&RecoveryManagerNode::onCenterline, this, std::placeholders::_1));

  cmd_pub_ = create_publisher<ackermann_msgs::msg::AckermannDriveStamped>(
    "/cmd_ackermann", rclcpp::QoS(10));
  state_request_pub_ = create_publisher<std_msgs::msg::UInt8>(
    "/safety/state_request", rclcpp::QoS(10));

  state_entered_time_ = now();
  control_timer_ = create_wall_timer(
    std::chrono::duration<double>(1.0 / control_rate_hz),
    std::bind(&RecoveryManagerNode::onControlTimer, this));

  RCLCPP_INFO(get_logger(), "recovery_manager started (DWA-based REVERSE/RECOVER)");
}

void RecoveryManagerNode::onVehicleState(const safety_manager::msg::VehicleState::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  prev_vehicle_state_ = vehicle_state_;
  vehicle_state_ = msg->state;
  if (vehicle_state_ != prev_vehicle_state_) {
    state_entered_time_ = now();
  }
}

void RecoveryManagerNode::onCostmap(const nav_msgs::msg::OccupancyGrid::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  grid_.data = msg->data;
  grid_.width = static_cast<int>(msg->info.width);
  grid_.height = static_cast<int>(msg->info.height);
  grid_.resolution = msg->info.resolution;
  grid_.origin_x = msg->info.origin.position.x;
  grid_.origin_y = msg->info.origin.position.y;
  grid_received_ = true;
}

void RecoveryManagerNode::onOdom(const nav_msgs::msg::Odometry::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  current_speed_mps_ = msg->twist.twist.linear.x;
  current_yaw_rate_rad_s_ = msg->twist.twist.angular.z;
}

void RecoveryManagerNode::onCenterline(const nav_msgs::msg::Path::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  centerline_body_.clear();
  centerline_body_.reserve(msg->poses.size());
  for (const auto & pose : msg->poses) {
    centerline_body_.emplace_back(pose.pose.position.x, pose.pose.position.y);
  }
}

double RecoveryManagerNode::centerlineHeadingErrorRad() const
{
  // Same nearest-segment-tangent projection as controller/stanley_controller
  // (duplicated rather than shared across packages to keep recovery_manager
  // dependency-free of `controller`) -- vehicle heading is 0 by construction
  // in the body frame, so the nearest segment's tangent angle is directly
  // the heading error to correct back onto the lane centerline.
  if (centerline_body_.size() < 2) {return 0.0;}

  double best_dist_sq = std::numeric_limits<double>::max();
  double best_tangent = 0.0;
  for (size_t i = 0; i + 1 < centerline_body_.size(); ++i) {
    const auto & a = centerline_body_[i];
    const auto & b = centerline_body_[i + 1];
    double seg_x = b.first - a.first;
    double seg_y = b.second - a.second;
    double seg_len_sq = seg_x * seg_x + seg_y * seg_y;
    if (seg_len_sq < 1e-9) {continue;}

    double t = std::clamp((-a.first * seg_x + -a.second * seg_y) / seg_len_sq, 0.0, 1.0);
    double proj_x = a.first + t * seg_x;
    double proj_y = a.second + t * seg_y;
    double dist_sq = proj_x * proj_x + proj_y * proj_y;
    if (dist_sq < best_dist_sq) {
      best_dist_sq = dist_sq;
      best_tangent = std::atan2(seg_y, seg_x);
    }
  }
  return best_tangent;
}

void RecoveryManagerNode::publishCommand(double speed_mps, double steering_angle_rad)
{
  ackermann_msgs::msg::AckermannDriveStamped cmd;
  cmd.header.stamp = now();
  cmd.header.frame_id = "base_link";
  cmd.drive.speed = speed_mps;
  cmd.drive.steering_angle = steering_angle_rad;
  cmd_pub_->publish(cmd);
}

void RecoveryManagerNode::publishStop()
{
  publishCommand(0.0, 0.0);
}

void RecoveryManagerNode::onControlTimer()
{
  uint8_t state;
  OccupancyGridView grid;
  bool grid_received;
  double speed, yaw_rate;
  double heading_error;
  double elapsed_sec;
  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    state = vehicle_state_;
    grid = grid_;
    grid_received = grid_received_;
    speed = current_speed_mps_;
    yaw_rate = current_yaw_rate_rad_s_;
    heading_error = centerlineHeadingErrorRad();
    elapsed_sec = (now() - state_entered_time_).seconds();
  }

  constexpr uint8_t kReverse = safety_manager::msg::VehicleState::REVERSE;
  constexpr uint8_t kRecover = safety_manager::msg::VehicleState::RECOVER;

  if (state != kReverse && state != kRecover) {
    return;  // not our turn -- controller or safety_manager owns actuation
  }
  if (!grid_received) {
    publishStop();  // no obstacle picture yet, hold position rather than guess
    return;
  }

  if (state == kReverse) {
    // Straight-back bias (goal_heading=0): the vehicle just braked because
    // something is ahead, so back away roughly along the reverse of the
    // approach line by default; DWA still steers off dead-straight if the
    // scored trajectories show more clearance doing so.
    DwaCommand cmd = reverse_planner_->plan(speed, yaw_rate, grid, 0.0);
    if (!cmd.trajectory_found) {
      publishStop();
      return;
    }
    publishCommand(cmd.speed_mps, cmd.steering_angle_rad);

    double clearance_ahead = reverse_planner_->forwardClearanceM(grid);
    if (elapsed_sec >= reverse_min_duration_sec_ && clearance_ahead >= reverse_clear_distance_m_) {
      std_msgs::msg::UInt8 req;
      req.data = 3;  // path_clear -> safety_manager transitions REVERSE -> RECOVER
      state_request_pub_->publish(req);
    }
  } else {  // kRecover
    DwaCommand cmd = recover_planner_->plan(speed, yaw_rate, grid, heading_error);
    if (!cmd.trajectory_found) {
      publishStop();
      return;
    }
    publishCommand(cmd.speed_mps, cmd.steering_angle_rad);

    if (std::abs(heading_error) <= recovered_heading_tol_rad_) {
      std_msgs::msg::UInt8 req;
      req.data = 4;  // recovered -> safety_manager transitions RECOVER -> NORMAL
      state_request_pub_->publish(req);
    }
  }
}

}  // namespace recovery_manager
