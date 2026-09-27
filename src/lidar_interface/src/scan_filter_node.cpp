#include "lidar_interface/scan_filter_node.hpp"

#include <cmath>
#include <limits>

namespace lidar_interface
{

ScanFilterNode::ScanFilterNode()
: Node("lidar_interface")
{
  range_min_m_ = declare_parameter<double>("range_min_m", 0.15);
  range_max_m_ = declare_parameter<double>("range_max_m", 12.0);
  blind_spot_start_rad_ = declare_parameter<double>("blind_spot_start_rad", 0.0);
  blind_spot_end_rad_ = declare_parameter<double>("blind_spot_end_rad", 0.0);
  frame_id_override_ = declare_parameter<std::string>("frame_id_override", "");

  std::string input_topic = declare_parameter<std::string>("input_topic", "/lidar/scan_raw");
  std::string output_topic = declare_parameter<std::string>("output_topic", "/scan");

  scan_sub_ = create_subscription<sensor_msgs::msg::LaserScan>(
    input_topic, rclcpp::SensorDataQoS(),
    std::bind(&ScanFilterNode::onScan, this, std::placeholders::_1));
  scan_pub_ = create_publisher<sensor_msgs::msg::LaserScan>(output_topic, rclcpp::SensorDataQoS());

  RCLCPP_INFO(
    get_logger(), "lidar_interface started, %s -> %s", input_topic.c_str(), output_topic.c_str());
}

void ScanFilterNode::onScan(const sensor_msgs::msg::LaserScan::ConstSharedPtr & msg)
{
  sensor_msgs::msg::LaserScan out = *msg;

  if (!frame_id_override_.empty()) {
    out.header.frame_id = frame_id_override_;
  }

  // Clamp the message's own reported envelope to the physically valid
  // sensor range rather than trusting the driver's defaults, since
  // downstream consumers (obstacle_detection's VFH) filter strictly against
  // range_min/range_max.
  out.range_min = static_cast<float>(range_min_m_);
  out.range_max = static_cast<float>(range_max_m_);

  bool blind_spot_enabled = blind_spot_start_rad_ != blind_spot_end_rad_;

  for (size_t i = 0; i < out.ranges.size(); ++i) {
    float & r = out.ranges[i];
    if (!std::isfinite(r) || r < out.range_min || r > out.range_max) {
      r = std::numeric_limits<float>::infinity();  // REP-117: invalid-but-in-range == inf
      continue;
    }

    if (blind_spot_enabled) {
      double angle = out.angle_min + static_cast<double>(i) * out.angle_increment;
      // Normalize into [-pi, pi) to compare against the configured sector.
      while (angle > M_PI) {angle -= 2.0 * M_PI;}
      while (angle < -M_PI) {angle += 2.0 * M_PI;}
      bool in_blind_spot = blind_spot_start_rad_ <= blind_spot_end_rad_ ?
        (angle >= blind_spot_start_rad_ && angle <= blind_spot_end_rad_) :
        (angle >= blind_spot_start_rad_ || angle <= blind_spot_end_rad_);  // sector wraps +-pi
      if (in_blind_spot) {
        r = std::numeric_limits<float>::infinity();
      }
    }
  }

  scan_pub_->publish(out);
}

}  // namespace lidar_interface
