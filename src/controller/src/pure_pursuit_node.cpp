#include "controller/pure_pursuit_node.hpp"

#include <algorithm>
#include <cmath>

namespace controller
{

PurePursuitNode::PurePursuitNode()
: Node("pure_pursuit_controller")
{
  // SCRT10 wheelbase: default is a placeholder for a typical 1/10-scale
  // 4WD chassis — MEASURE the actual center-to-center distance between the
  // front and rear axles on your SCRT10 and set this parameter accordingly
  // before racing. See package README for the calibration procedure. Shared
  // across all three controller implementations below.
  double wheelbase_m = declare_parameter<double>("wheelbase_m", 0.045);
  max_steering_angle_rad_ = declare_parameter<double>("max_steering_angle_rad", 0.2812);

  ControllerFactoryParams factory_params;
  factory_params.type = declare_parameter<std::string>("controller_type", "pure_pursuit");

  factory_params.pure_pursuit.wheelbase_m = wheelbase_m;
  factory_params.pure_pursuit.lookahead_min_m = declare_parameter<double>("lookahead_min_m", 0.08);
  factory_params.pure_pursuit.lookahead_max_m = declare_parameter<double>("lookahead_max_m", 0.25);
  factory_params.pure_pursuit.lookahead_speed_gain =
    declare_parameter<double>("lookahead_speed_gain", 0.02125);
  factory_params.pure_pursuit.max_steering_angle_rad = max_steering_angle_rad_;

  factory_params.stanley.wheelbase_m = wheelbase_m;
  factory_params.stanley.cross_track_gain = declare_parameter<double>("stanley_cross_track_gain", 1.0);
  factory_params.stanley.softening_speed_mps =
    declare_parameter<double>("stanley_softening_speed_mps", 1.0);
  factory_params.stanley.max_steering_angle_rad = max_steering_angle_rad_;

  factory_params.mpc.wheelbase_m = wheelbase_m;
  factory_params.mpc.horizon_steps = declare_parameter<int>("mpc_horizon_steps", 8);
  factory_params.mpc.dt_sec = declare_parameter<double>("mpc_dt_sec", 0.05);
  factory_params.mpc.q_cross_track = declare_parameter<double>("mpc_q_cross_track", 5.0);
  factory_params.mpc.q_heading = declare_parameter<double>("mpc_q_heading", 3.0);
  factory_params.mpc.r_steering = declare_parameter<double>("mpc_r_steering", 0.5);
  factory_params.mpc.r_steering_rate = declare_parameter<double>("mpc_r_steering_rate", 1.0);
  factory_params.mpc.optimization_iterations =
    declare_parameter<int>("mpc_optimization_iterations", 15);
  factory_params.mpc.initial_search_step_rad =
    declare_parameter<double>("mpc_initial_search_step_rad", 0.12);
  factory_params.mpc.min_search_step_rad = declare_parameter<double>("mpc_min_search_step_rad", 0.005);
  factory_params.mpc.max_steering_angle_rad = max_steering_angle_rad_;

  steering_controller_ = createSteeringController(factory_params);

  max_lateral_accel_mps2_ = declare_parameter<double>("max_lateral_accel_mps2", 4.0);
  v_min_mps_ = declare_parameter<double>("v_min_mps", 0.5);
  v_max_mps_ = declare_parameter<double>("v_max_mps", 8.0);
  speed_p_gain_ = declare_parameter<double>("speed_p_gain", 1.5);
  max_steering_rate_rad_s_ = declare_parameter<double>("max_steering_rate_rad_s", 3.0);

  double control_rate_hz = declare_parameter<double>("control_rate_hz", 50.0);

  centerline_sub_ = create_subscription<nav_msgs::msg::Path>(
    "/lane/centerline", rclcpp::QoS(10),
    std::bind(&PurePursuitNode::onCenterline, this, std::placeholders::_1));
  curvature_sub_ = create_subscription<std_msgs::msg::Float32>(
    "/lane/curvature_radius_m", rclcpp::QoS(10),
    std::bind(&PurePursuitNode::onCurvature, this, std::placeholders::_1));
  odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
    "/ekf/odom", rclcpp::QoS(10),
    std::bind(&PurePursuitNode::onOdom, this, std::placeholders::_1));
  vehicle_state_sub_ = create_subscription<safety_manager::msg::VehicleState>(
    "/safety/vehicle_state", rclcpp::QoS(10),
    std::bind(&PurePursuitNode::onVehicleState, this, std::placeholders::_1));
  obstacle_sub_ = create_subscription<geometry_msgs::msg::Twist>(
    "/obstacle/avoidance_cmd", rclcpp::QoS(10),
    std::bind(&PurePursuitNode::onObstacleAvoidance, this, std::placeholders::_1));

  cmd_pub_ = create_publisher<ackermann_msgs::msg::AckermannDriveStamped>(
    "/cmd_ackermann", rclcpp::QoS(10));

  control_timer_ = create_wall_timer(
    std::chrono::duration<double>(1.0 / control_rate_hz),
    std::bind(&PurePursuitNode::onControlTimer, this));

  last_control_time_ = now();

  RCLCPP_INFO(
    get_logger(), "controller started, type=%s, wheelbase=%.3fm",
    factory_params.type.c_str(), wheelbase_m);
}

void PurePursuitNode::onCenterline(const nav_msgs::msg::Path::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  path_.clear();
  path_.reserve(msg->poses.size());
  for (const auto & pose : msg->poses) {
    path_.push_back(PathPoint{pose.pose.position.x, pose.pose.position.y});
  }
}

void PurePursuitNode::onCurvature(const std_msgs::msg::Float32::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  curvature_radius_m_ = msg->data;
}

void PurePursuitNode::onOdom(const nav_msgs::msg::Odometry::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  current_speed_mps_ = msg->twist.twist.linear.x;
}

void PurePursuitNode::onVehicleState(const safety_manager::msg::VehicleState::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  vehicle_state_ = msg->state;
}

void PurePursuitNode::onObstacleAvoidance(const geometry_msgs::msg::Twist::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  // angular.z carries a steering bias (rad) requested by the VFH avoidance
  // node; additive with the Pure Pursuit output, clamped downstream.
  obstacle_steering_bias_rad_ = msg->angular.z;
  // linear.x carries a continuous speed multiplier (1.0=clear .. 0.0=stop)
  // based on measured clearance -- an object being present must actually
  // slow the car down, not just bias the steering.
  obstacle_speed_scale_ = msg->linear.x;
}

double PurePursuitNode::speedProfile(double curvature_radius_m) const
{
  // v_max_curve = sqrt(a_lat_max * R): the classic max-speed-through-a-turn
  // bound from the tire's available lateral acceleration budget.
  double r = std::max(curvature_radius_m, 0.1);
  double v_curve = std::sqrt(max_lateral_accel_mps2_ * r);
  return std::clamp(v_curve, v_min_mps_, v_max_mps_);
}

void PurePursuitNode::onControlTimer()
{
  std::vector<PathPoint> path_copy;
  double curvature_radius_m, current_speed_mps, obstacle_bias, obstacle_speed_scale;
  uint8_t vehicle_state;
  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    path_copy = path_;
    curvature_radius_m = curvature_radius_m_;
    current_speed_mps = current_speed_mps_;
    obstacle_bias = obstacle_steering_bias_rad_;
    obstacle_speed_scale = obstacle_speed_scale_;
    vehicle_state = vehicle_state_;
  }

  ackermann_msgs::msg::AckermannDriveStamped cmd;
  cmd.header.stamp = now();
  cmd.header.frame_id = "base_link";

  // NORMAL(0)/AVOID(1) are the only states permitted to command forward
  // motion from this node; REVERSE(3)/RECOVER(4) are exclusively owned by
  // `recovery_manager`'s DWA-based low-speed maneuver so the two nodes never
  // fight over /cmd_ackermann (see recovery_manager's README). safety_manager
  // still gates on /cmd_ackermann as a second independent layer (defense in
  // depth) even if this check is bypassed.
  bool motion_allowed = vehicle_state == 0 || vehicle_state == 1;

  if (!motion_allowed || path_copy.empty()) {
    cmd.drive.speed = 0.0;
    cmd.drive.steering_angle = 0.0;
    last_steering_rad_ = 0.0;  // re-anchor the slew limiter so resuming motion isn't rate-limited from a stale angle
    last_control_time_ = now();
    cmd_pub_->publish(cmd);
    return;
  }

  VehicleState state{current_speed_mps};
  double steering = steering_controller_->computeSteeringAngle(path_copy, state);
  steering = std::clamp(
    steering + obstacle_bias, -max_steering_angle_rad_, max_steering_angle_rad_);

  // Slew-rate limit: cap how far the steering angle can move this cycle so a
  // sudden upstream change (VFH switching valleys, a noisy centerline point,
  // a controller swap) can't command a step change the tires can't track
  // without skidding -- the servo/wheels turn smoothly toward the target
  // instead of snapping to it.
  rclcpp::Time t_now = now();
  double dt = (t_now - last_control_time_).seconds();
  last_control_time_ = t_now;
  if (dt > 0.0) {
    double max_delta = max_steering_rate_rad_s_ * dt;
    steering = last_steering_rad_ + std::clamp(steering - last_steering_rad_, -max_delta, max_delta);
  }
  last_steering_rad_ = steering;

  double target_speed = speedProfile(curvature_radius_m) * std::clamp(obstacle_speed_scale, 0.0, 1.0);

  // Light feed-forward + proportional feedback on the speed error; ESC-level
  // throttle mapping/braking logic lives in the hardware interface layer,
  // this only produces the requested speed/acceleration.
  double speed_error = target_speed - current_speed_mps;
  double accel_cmd = speed_p_gain_ * speed_error;
  prev_speed_error_ = speed_error;

  cmd.drive.steering_angle = steering;
  cmd.drive.speed = target_speed;
  cmd.drive.acceleration = accel_cmd;

  cmd_pub_->publish(cmd);
}

}  // namespace controller
