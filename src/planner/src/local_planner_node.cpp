#include "planner/local_planner_node.hpp"

#include <algorithm>
#include <cmath>

#include "geometry_msgs/msg/pose_stamped.hpp"

namespace planner
{

LocalPlannerNode::LocalPlannerNode()
: Node("planner")
{
  std::string raceline_path = declare_parameter<std::string>("raceline_yaml_path", "");
  race_mode_ = declare_parameter<std::string>("race_mode", "speed");
  lookahead_window_m_ = declare_parameter<double>("lookahead_window_m", 15.0);
  blend_weight_ = declare_parameter<double>("blend_weight", 0.3);
  double control_rate_hz = declare_parameter<double>("control_rate_hz", 20.0);

  if (raceline_path.empty() || !raceline_.loadFromYaml(raceline_path)) {
    RCLCPP_WARN(
      get_logger(),
      "planner: no valid raceline loaded from '%s' -- running on lane_detection's "
      "centerline alone until a course raceline is supplied", raceline_path.c_str());
  } else {
    RCLCPP_INFO(
      get_logger(), "planner: loaded %zu raceline waypoints from '%s'",
      raceline_.waypoints().size(), raceline_path.c_str());
  }

  odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
    "/ekf/odom", rclcpp::QoS(10),
    std::bind(&LocalPlannerNode::onOdom, this, std::placeholders::_1));
  centerline_sub_ = create_subscription<nav_msgs::msg::Path>(
    "/lane/centerline", rclcpp::QoS(10),
    std::bind(&LocalPlannerNode::onCenterline, this, std::placeholders::_1));
  obstacle_sub_ = create_subscription<geometry_msgs::msg::Twist>(
    "/obstacle/avoidance_cmd", rclcpp::QoS(10),
    std::bind(&LocalPlannerNode::onObstacleAvoidance, this, std::placeholders::_1));

  local_path_pub_ = create_publisher<nav_msgs::msg::Path>("/planner/local_path", rclcpp::QoS(10));
  rclcpp::QoS global_qos(1);
  global_qos.transient_local();  // late-joining rviz subs still get the last published raceline
  global_path_pub_ = create_publisher<nav_msgs::msg::Path>("/planner/global_path", global_qos);
  state_request_pub_ = create_publisher<std_msgs::msg::UInt8>("/safety/state_request", rclcpp::QoS(10));

  publishGlobalPath();

  control_timer_ = create_wall_timer(
    std::chrono::duration<double>(1.0 / control_rate_hz),
    std::bind(&LocalPlannerNode::onControlTimer, this));

  RCLCPP_INFO(get_logger(), "planner started, race_mode=%s", race_mode_.c_str());
}

void LocalPlannerNode::onOdom(const nav_msgs::msg::Odometry::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  pose_x_ = msg->pose.pose.position.x;
  pose_y_ = msg->pose.pose.position.y;
  // Pure-yaw quaternion assumption (2D ground vehicle, qx=qy=0): theta =
  // 2*atan2(qz, qw). Consistent with how sensor_fusion's EKF builds the
  // orientation quaternion from its scalar heading state.
  const auto & q = msg->pose.pose.orientation;
  pose_theta_ = 2.0 * std::atan2(q.z, q.w);
}

void LocalPlannerNode::onCenterline(const nav_msgs::msg::Path::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  lane_centerline_body_.clear();
  lane_centerline_body_.reserve(msg->poses.size());
  for (const auto & pose : msg->poses) {
    lane_centerline_body_.push_back(PathPoint2D{pose.pose.position.x, pose.pose.position.y});
  }
}

void LocalPlannerNode::onObstacleAvoidance(const geometry_msgs::msg::Twist::ConstSharedPtr & msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  obstacle_path_blocked_ = msg->linear.x <= 0.0;
}

std::vector<PathPoint2D> LocalPlannerNode::globalWindowInBodyFrame() const
{
  std::vector<PathPoint2D> out;
  const auto & waypoints = raceline_.waypoints();
  if (waypoints.empty()) {return out;}

  double px, py, theta;
  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    px = pose_x_;
    py = pose_y_;
    theta = pose_theta_;
  }
  double ct = std::cos(theta);
  double st = std::sin(theta);

  struct Candidate {double dist; PathPoint2D pt;};
  std::vector<Candidate> ahead;
  ahead.reserve(waypoints.size());

  for (const auto & wp : waypoints) {
    double dx = wp.x - px;
    double dy = wp.y - py;
    // World -> body frame rotation (inverse of the vehicle's world pose).
    double body_x = dx * ct + dy * st;
    double body_y = -dx * st + dy * ct;
    if (body_x < 0.0 || body_x > lookahead_window_m_) {continue;}
    ahead.push_back(Candidate{body_x, PathPoint2D{body_x, body_y}});
  }

  std::sort(ahead.begin(), ahead.end(), [](const Candidate & a, const Candidate & b) {
    return a.dist < b.dist;
  });

  out.reserve(ahead.size());
  for (const auto & c : ahead) {out.push_back(c.pt);}
  return out;
}

void LocalPlannerNode::onControlTimer()
{
  ArbitrationInput input;
  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    input.lane_centerline_body = lane_centerline_body_;
    input.obstacle_path_blocked = obstacle_path_blocked_;
  }
  input.global_path_body = globalWindowInBodyFrame();
  input.race_mode = race_mode_;
  input.blend_weight = blend_weight_;

  ArbitrationResult result = arbitrator_.arbitrate(input);

  nav_msgs::msg::Path path_msg;
  path_msg.header.stamp = now();
  path_msg.header.frame_id = "base_link";
  path_msg.poses.reserve(result.local_path_body.size());
  for (const auto & pt : result.local_path_body) {
    geometry_msgs::msg::PoseStamped pose;
    pose.header = path_msg.header;
    pose.pose.position.x = pt.x;
    pose.pose.position.y = pt.y;
    pose.pose.orientation.w = 1.0;
    path_msg.poses.push_back(pose);
  }
  local_path_pub_->publish(path_msg);

  if (!result.path_valid) {
    std_msgs::msg::UInt8 req;
    req.data = 1;  // obstacle_close -> AVOID (see safety_manager's state_request contract)
    state_request_pub_->publish(req);
  }
}

void LocalPlannerNode::publishGlobalPath()
{
  const auto & waypoints = raceline_.waypoints();
  if (waypoints.empty()) {return;}

  nav_msgs::msg::Path path_msg;
  path_msg.header.stamp = now();
  path_msg.header.frame_id = "map";
  path_msg.poses.reserve(waypoints.size());
  for (const auto & wp : waypoints) {
    geometry_msgs::msg::PoseStamped pose;
    pose.header = path_msg.header;
    pose.pose.position.x = wp.x;
    pose.pose.position.y = wp.y;
    pose.pose.orientation.w = 1.0;
    path_msg.poses.push_back(pose);
  }
  global_path_pub_->publish(path_msg);
}

}  // namespace planner
