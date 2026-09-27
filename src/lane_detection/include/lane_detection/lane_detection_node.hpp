#pragma once

#include "lane_detection/lane_detector.hpp"
#include "nav_msgs/msg/path.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "std_msgs/msg/float32.hpp"

namespace lane_detection
{

class LaneDetectionNode : public rclcpp::Node
{
public:
  LaneDetectionNode();

private:
  void onImage(const sensor_msgs::msg::Image::ConstSharedPtr & msg);

  std::unique_ptr<LaneDetector> detector_;
  LaneDetectorParams params_;

  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr centerline_pub_;
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr curvature_pub_;
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr offset_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr debug_image_pub_;

  std::string vehicle_frame_id_;
  bool publish_debug_image_{false};
};

}  // namespace lane_detection
