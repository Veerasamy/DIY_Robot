#include "obstacle_detection/obstacle_avoidance_node.hpp"

#if __has_include(<cv_bridge/cv_bridge.hpp>)
#include <cv_bridge/cv_bridge.hpp>  // Jazzy+
#else
#include <cv_bridge/cv_bridge.h>  // Humble
#endif

#include <algorithm>
#include <cmath>
#include <limits>

namespace obstacle_detection
{

ObstacleAvoidanceNode::ObstacleAvoidanceNode()
: Node("obstacle_avoidance")
{
  VfhParams params;
  params.fov_half_angle_rad = declare_parameter<double>("fov_half_angle_rad", 1.5708);
  params.sector_resolution_rad = declare_parameter<double>("sector_resolution_rad", 0.0349);
  params.smoothing_kernel_width = declare_parameter<int>("smoothing_kernel_width", 5);
  params.max_range_m = declare_parameter<double>("max_range_m", 6.0);
  params.obstacle_threshold = declare_parameter<double>("obstacle_threshold", 1.5);
  params.vehicle_half_width_m = declare_parameter<double>("vehicle_half_width_m", 0.1365);
  params.safety_margin_m = declare_parameter<double>("safety_margin_m", 0.15);
  planner_ = std::make_unique<VfhPlanner>(params);

  camera_hfov_rad_ = declare_parameter<double>("camera_hfov_rad", 1.361);
  camera_vfov_rad_ = declare_parameter<double>("camera_vfov_rad", 1.204);
  camera_mount_height_m_ = declare_parameter<double>("camera_mount_height_m", 0.195);
  camera_tilt_rad_ = declare_parameter<double>("camera_tilt_rad", 0.0);
  ground_clearance_min_m_ = declare_parameter<double>("ground_clearance_min_m", 0.03);
  obstacle_height_max_m_ = declare_parameter<double>("obstacle_height_max_m", 0.25);
  depth_sample_stride_px_ = declare_parameter<int>("depth_sample_stride_px", 8);

  braking_decel_mps2_ = declare_parameter<double>("braking_decel_mps2", 6.0);
  braking_margin_m_ = declare_parameter<double>("braking_margin_m", 0.3);

  wall_check_distance_m_ = declare_parameter<double>("wall_check_distance_m", 1.2);
  wall_side_clear_min_m_ = declare_parameter<double>("wall_side_clear_min_m", 0.5);
  wall_front_half_angle_rad_ = declare_parameter<double>("wall_front_half_angle_rad", 0.26);
  wall_side_min_angle_rad_ = declare_parameter<double>("wall_side_min_angle_rad", 0.35);
  wall_side_max_angle_rad_ = declare_parameter<double>("wall_side_max_angle_rad", 1.5708);

  cliff_drop_max_m_ = declare_parameter<double>("cliff_drop_max_m", 0.15);
  cliff_check_distance_m_ = declare_parameter<double>("cliff_check_distance_m", 1.5);

  double control_rate_hz = declare_parameter<double>("control_rate_hz", 20.0);

  scan_sub_ = create_subscription<sensor_msgs::msg::LaserScan>(
    "/scan", rclcpp::SensorDataQoS(),
    std::bind(&ObstacleAvoidanceNode::onLaserScan, this, std::placeholders::_1));
  depth_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "/zed/depth/image", rclcpp::SensorDataQoS(),
    std::bind(&ObstacleAvoidanceNode::onDepthImage, this, std::placeholders::_1));
  odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
    "/ekf/odom", rclcpp::QoS(10),
    std::bind(&ObstacleAvoidanceNode::onOdom, this, std::placeholders::_1));

  avoidance_cmd_pub_ = create_publisher<geometry_msgs::msg::Twist>(
    "/obstacle/avoidance_cmd", rclcpp::QoS(10));
  costmap_pub_ = create_publisher<nav_msgs::msg::OccupancyGrid>(
    "/obstacle/costmap", rclcpp::QoS(1));
  state_request_pub_ = create_publisher<std_msgs::msg::UInt8>(
    "/safety/state_request", rclcpp::QoS(10));
  wall_detected_pub_ = create_publisher<std_msgs::msg::Bool>("/obstacle/wall_detected", rclcpp::QoS(10));
  front_clearance_pub_ = create_publisher<std_msgs::msg::Float32>(
    "/obstacle/front_clearance_m", rclcpp::QoS(10));
  left_clearance_pub_ = create_publisher<std_msgs::msg::Float32>(
    "/obstacle/left_clearance_m", rclcpp::QoS(10));
  right_clearance_pub_ = create_publisher<std_msgs::msg::Float32>(
    "/obstacle/right_clearance_m", rclcpp::QoS(10));
  cliff_detected_pub_ = create_publisher<std_msgs::msg::Bool>("/obstacle/cliff_detected", rclcpp::QoS(10));
  cliff_distance_pub_ = create_publisher<std_msgs::msg::Float32>(
    "/obstacle/cliff_distance_m", rclcpp::QoS(10));

  control_timer_ = create_wall_timer(
    std::chrono::duration<double>(1.0 / control_rate_hz),
    std::bind(&ObstacleAvoidanceNode::onControlTimer, this));

  RCLCPP_INFO(get_logger(), "obstacle_detection (VFH) started");
}

void ObstacleAvoidanceNode::onLaserScan(const sensor_msgs::msg::LaserScan::ConstSharedPtr & msg)
{
  std::vector<ObstaclePoint> points;
  points.reserve(msg->ranges.size());
  for (size_t i = 0; i < msg->ranges.size(); ++i) {
    float r = msg->ranges[i];
    if (!std::isfinite(r) || r < msg->range_min || r > msg->range_max) {continue;}
    double angle = msg->angle_min + static_cast<double>(i) * msg->angle_increment;
    points.push_back(ObstaclePoint{angle, r});
  }
  std::lock_guard<std::mutex> lock(data_mutex_);
  lidar_points_ = std::move(points);
}

std::vector<ObstaclePoint> ObstacleAvoidanceNode::depthImageToPoints(
  const sensor_msgs::msg::Image & msg) const
{
  cv_bridge::CvImageConstPtr cv_ptr;
  try {
    cv_ptr = cv_bridge::toCvCopy(msg, "32FC1");
  } catch (const cv_bridge::Exception & e) {
    return {};
  }
  const cv::Mat & depth = cv_ptr->image;
  const int w = depth.cols;
  const int h = depth.rows;

  const double fx = (w / 2.0) / std::tan(camera_hfov_rad_ / 2.0);
  const double fy = (h / 2.0) / std::tan(camera_vfov_rad_ / 2.0);
  const double cx = w / 2.0;
  const double cy = h / 2.0;
  const double ct = std::cos(camera_tilt_rad_);
  const double st = std::sin(camera_tilt_rad_);

  std::vector<ObstaclePoint> points;
  for (int v = 0; v < h; v += std::max(1, depth_sample_stride_px_)) {
    // Track the single closest valid obstacle per column band to keep the
    // point count (and therefore the VFH histogram build cost) bounded.
    for (int u = 0; u < w; u += std::max(1, depth_sample_stride_px_)) {
      float d = depth.at<float>(v, u);
      if (!std::isfinite(d) || d <= 0.0f || d > static_cast<float>(planner_->params().max_range_m)) {
        continue;
      }

      double x_cam = (u - cx) * d / fx;  // right
      double y_cam = (v - cy) * d / fy;  // down
      double z_cam = d;                  // forward (optical axis)

      // Rotate by mount tilt (pitch about camera's right axis) and shift by
      // mount height to get an approximate ground-relative height and
      // forward distance in the vehicle body frame.
      double forward_x = z_cam * ct - y_cam * st;
      double height_above_ground = camera_mount_height_m_ - (y_cam * ct + z_cam * st);
      double lateral_y = -x_cam;

      if (height_above_ground < ground_clearance_min_m_ ||
        height_above_ground > obstacle_height_max_m_)
      {
        continue;  // ground plane or overhead clutter
      }

      double range = std::hypot(forward_x, lateral_y);
      double angle = std::atan2(lateral_y, forward_x);
      points.push_back(ObstaclePoint{angle, range});
    }
  }
  return points;
}

double ObstacleAvoidanceNode::computeCliffDistance(const cv::Mat & depth) const
{
  const int w = depth.cols;
  const int h = depth.rows;
  const double fy = (h / 2.0) / std::tan(camera_vfov_rad_ / 2.0);
  const double cy = h / 2.0;
  const double ct = std::cos(camera_tilt_rad_);
  const double st = std::sin(camera_tilt_rad_);
  const double max_range = planner_->params().max_range_m;
  const int row_stride = std::max(1, depth_sample_stride_px_);
  const int col_stride = std::max(1, depth_sample_stride_px_) * 4;  // cheaper: this is a 2nd full pass

  double cliff_range_m = std::numeric_limits<double>::infinity();
  for (int v = static_cast<int>(cy) + row_stride; v < h; v += row_stride) {
    // Expected depth if flat ground continues to this row (tilt/mount-height
    // corrected pinhole projection, height_above_ground == 0 solved for d).
    double denom = (v - cy) * ct - fy * st;
    if (denom <= 1e-6) {continue;}
    double d_expected = camera_mount_height_m_ * fy / denom;
    if (!(d_expected > 0.0) || d_expected > max_range) {continue;}  // beyond meaningful range anyway

    for (int u = 0; u < w; u += col_stride) {
      float d = depth.at<float>(v, u);
      bool missing_or_far = !std::isfinite(d) || d <= 0.0f || d > d_expected + cliff_drop_max_m_;
      if (missing_or_far) {
        cliff_range_m = std::min(cliff_range_m, d_expected);
        break;
      }
    }
  }
  return cliff_range_m;
}

void ObstacleAvoidanceNode::onDepthImage(const sensor_msgs::msg::Image::ConstSharedPtr & msg)
{
  auto points = depthImageToPoints(*msg);

  cv_bridge::CvImageConstPtr cv_ptr;
  double cliff_m = std::numeric_limits<double>::infinity();
  try {
    cv_ptr = cv_bridge::toCvCopy(*msg, "32FC1");
    cliff_m = computeCliffDistance(cv_ptr->image);
  } catch (const cv_bridge::Exception &) {
  }

  std::lock_guard<std::mutex> lock(data_mutex_);
  depth_points_ = std::move(points);
  last_cliff_distance_m_ = cliff_m;
}

void ObstacleAvoidanceNode::onOdom(const nav_msgs::msg::Odometry::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  current_speed_mps_ = msg->twist.twist.linear.x;
}

void ObstacleAvoidanceNode::publishCostmap(
  const std::vector<ObstaclePoint> & points, const rclcpp::Time & stamp)
{
  constexpr double kResolution = 0.1;
  constexpr int kGridSize = 100;  // 10m x 10m local costmap centered on vehicle

  nav_msgs::msg::OccupancyGrid grid;
  grid.header.stamp = stamp;
  grid.header.frame_id = "base_link";
  grid.info.resolution = static_cast<float>(kResolution);
  grid.info.width = kGridSize;
  grid.info.height = kGridSize;
  grid.info.origin.position.x = -kGridSize * kResolution / 2.0;
  grid.info.origin.position.y = -kGridSize * kResolution / 2.0;
  grid.data.assign(static_cast<size_t>(kGridSize * kGridSize), 0);

  for (const auto & p : points) {
    double x = p.range_m * std::cos(p.angle_rad);
    double y = p.range_m * std::sin(p.angle_rad);
    int gx = static_cast<int>((x - grid.info.origin.position.x) / kResolution);
    int gy = static_cast<int>((y - grid.info.origin.position.y) / kResolution);
    if (gx >= 0 && gx < kGridSize && gy >= 0 && gy < kGridSize) {
      grid.data[static_cast<size_t>(gy * kGridSize + gx)] = 100;
    }
  }
  costmap_pub_->publish(grid);
}

ObstacleAvoidanceNode::SectorClearance ObstacleAvoidanceNode::computeSectorClearance(
  const std::vector<ObstaclePoint> & points) const
{
  // REP-103: angle=0 is straight ahead, +angle is left, -angle is right.
  SectorClearance clearance{planner_->params().max_range_m, planner_->params().max_range_m,
    planner_->params().max_range_m};
  for (const auto & p : points) {
    double a = std::abs(p.angle_rad);
    if (a <= wall_front_half_angle_rad_) {
      clearance.front_m = std::min(clearance.front_m, p.range_m);
    }
    if (p.angle_rad >= wall_side_min_angle_rad_ && p.angle_rad <= wall_side_max_angle_rad_) {
      clearance.left_m = std::min(clearance.left_m, p.range_m);
    }
    if (-p.angle_rad >= wall_side_min_angle_rad_ && -p.angle_rad <= wall_side_max_angle_rad_) {
      clearance.right_m = std::min(clearance.right_m, p.range_m);
    }
  }
  return clearance;
}

void ObstacleAvoidanceNode::onControlTimer()
{
  std::vector<ObstaclePoint> fused;
  double current_speed;
  double cliff_distance_m;
  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    fused = lidar_points_;
    fused.insert(fused.end(), depth_points_.begin(), depth_points_.end());
    current_speed = current_speed_mps_;
    cliff_distance_m = last_cliff_distance_m_;
  }

  VfhResult result = planner_->plan(fused);

  // Time-To-Collision style braking distance check: if the required braking
  // distance at the current speed exceeds the measured clearance (plus
  // margin), request BRAKE regardless of whether a VFH valley was found —
  // a valley to the side doesn't help if we're about to rear-end something
  // dead ahead before the steering command can take effect.
  double braking_distance = (current_speed * current_speed) / (2.0 * braking_decel_mps2_);
  bool collision_imminent = result.min_clearance_m < (braking_distance + braking_margin_m_);

  // Continuous speed scale (1.0 = no restriction .. ~0.15 = crawl) so an
  // object anywhere in view actually slows the car down, not just biases
  // steering -- ramps down starting well before the hard-brake threshold
  // above so the slowdown is smooth rather than an abrupt cliff to zero.
  double ease_start_m = braking_distance + braking_margin_m_ * 5.0;
  double speed_scale = 1.0;
  if (result.min_clearance_m < ease_start_m) {
    double span = std::max(0.1, ease_start_m - braking_margin_m_);
    speed_scale = std::clamp((result.min_clearance_m - braking_margin_m_) / span, 0.15, 1.0);
  }

  // Wall detection: an object dead ahead within wall_check_distance_m only
  // matters if BOTH side sectors are also too tight to slip through -- a
  // clear side means it's a normal avoidable obstacle, not a wall.
  SectorClearance clearance = computeSectorClearance(fused);
  bool wall_detected =
    clearance.front_m < wall_check_distance_m_ &&
    clearance.left_m < wall_side_clear_min_m_ &&
    clearance.right_m < wall_side_clear_min_m_;

  // Cliff / negative-obstacle: a track edge or drop-off is exactly as
  // dangerous as a wall -- treat it the same way (full stop), it just comes
  // from the ground-plane side of the depth analysis instead of the
  // above-ground side.
  bool cliff_detected = cliff_distance_m < cliff_check_distance_m_;

  geometry_msgs::msg::Twist cmd;
  cmd.angular.z = result.steering_bias_rad;
  cmd.linear.x = (wall_detected || cliff_detected) ? 0.0 : speed_scale;
  avoidance_cmd_pub_->publish(cmd);

  publishCostmap(fused, now());

  std_msgs::msg::Bool wall_msg;
  wall_msg.data = wall_detected;
  wall_detected_pub_->publish(wall_msg);
  std_msgs::msg::Float32 front_msg, left_msg, right_msg;
  front_msg.data = static_cast<float>(clearance.front_m);
  left_msg.data = static_cast<float>(clearance.left_m);
  right_msg.data = static_cast<float>(clearance.right_m);
  front_clearance_pub_->publish(front_msg);
  left_clearance_pub_->publish(left_msg);
  right_clearance_pub_->publish(right_msg);

  std_msgs::msg::Bool cliff_msg;
  cliff_msg.data = cliff_detected;
  cliff_detected_pub_->publish(cliff_msg);
  std_msgs::msg::Float32 cliff_dist_msg;
  cliff_dist_msg.data = static_cast<float>(
    std::min(cliff_distance_m, static_cast<double>(planner_->params().max_range_m) * 2.0));
  cliff_distance_pub_->publish(cliff_dist_msg);

  std_msgs::msg::UInt8 state_req;
  if (wall_detected || cliff_detected || collision_imminent) {
    state_req.data = 2;  // wall / cliff / collision_imminent -> BRAKE
  } else if (result.path_blocked) {
    state_req.data = 1;  // obstacle_close -> AVOID
  } else {
    state_req.data = 0;  // clear
  }
  state_request_pub_->publish(state_req);
}

}  // namespace obstacle_detection
