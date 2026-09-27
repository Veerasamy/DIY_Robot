#include "lane_detection/lane_detection_node.hpp"

#if __has_include(<cv_bridge/cv_bridge.hpp>)
#include <cv_bridge/cv_bridge.hpp>  // Jazzy+
#else
#include <cv_bridge/cv_bridge.h>  // Humble
#endif

#include "geometry_msgs/msg/pose_stamped.hpp"

namespace lane_detection
{

LaneDetectionNode::LaneDetectionNode()
: Node("lane_detection")
{
  vehicle_frame_id_ = declare_parameter<std::string>("vehicle_frame_id", "base_link");
  publish_debug_image_ = declare_parameter<bool>("publish_debug_image", false);

  params_.lane_width_m = declare_parameter<double>("lane_width_m", 0.9144);
  params_.meters_per_pixel_x = declare_parameter<double>("meters_per_pixel_x", 0.9144 / (0.6 * 700.0));
  params_.meters_per_pixel_y = declare_parameter<double>("meters_per_pixel_y", 9.0 / 900.0);
  params_.sobel_thresh_min = declare_parameter<int>("sobel_thresh_min", 20);
  params_.sobel_thresh_max = declare_parameter<int>("sobel_thresh_max", 100);
  params_.s_channel_thresh_min = declare_parameter<int>("s_channel_thresh_min", 100);
  params_.s_channel_thresh_max = declare_parameter<int>("s_channel_thresh_max", 255);
  params_.n_windows = declare_parameter<int>("n_windows", 9);
  params_.window_margin = declare_parameter<int>("window_margin", 80);
  params_.min_pixels_to_recenter = declare_parameter<int>("min_pixels_to_recenter", 50);
  params_.generate_debug_image = publish_debug_image_;

  detector_ = std::make_unique<LaneDetector>(params_);

  image_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "/zed/rgb/image_raw", rclcpp::SensorDataQoS(),
    std::bind(&LaneDetectionNode::onImage, this, std::placeholders::_1));

  centerline_pub_ = create_publisher<nav_msgs::msg::Path>("/lane/centerline", rclcpp::QoS(10));
  curvature_pub_ = create_publisher<std_msgs::msg::Float32>("/lane/curvature_radius_m", rclcpp::QoS(10));
  offset_pub_ = create_publisher<std_msgs::msg::Float32>("/lane/center_offset_m", rclcpp::QoS(10));
  if (publish_debug_image_) {
    debug_image_pub_ = create_publisher<sensor_msgs::msg::Image>(
      "/lane/debug_image", rclcpp::SensorDataQoS());
  }

  RCLCPP_INFO(get_logger(), "lane_detection started (debug_image=%s)",
    publish_debug_image_ ? "on" : "off");
}

void LaneDetectionNode::onImage(const sensor_msgs::msg::Image::ConstSharedPtr & msg)
{
  cv_bridge::CvImageConstPtr cv_ptr;
  try {
    cv_ptr = cv_bridge::toCvShare(msg, "bgr8");
  } catch (const cv_bridge::Exception & e) {
    RCLCPP_ERROR(get_logger(), "cv_bridge exception: %s", e.what());
    return;
  }

  LaneResult result = detector_->process(cv_ptr->image);

  nav_msgs::msg::Path path;
  path.header.stamp = msg->header.stamp;
  path.header.frame_id = vehicle_frame_id_;
  path.poses.reserve(result.centerline_m.size());
  for (const auto & pt : result.centerline_m) {
    geometry_msgs::msg::PoseStamped pose;
    pose.header = path.header;
    pose.pose.position.x = pt.x;  // forward
    pose.pose.position.y = pt.y;  // lateral
    pose.pose.orientation.w = 1.0;
    path.poses.push_back(pose);
  }
  centerline_pub_->publish(path);

  std_msgs::msg::Float32 curvature_msg;
  curvature_msg.data = static_cast<float>(result.curvature_radius_m);
  curvature_pub_->publish(curvature_msg);

  std_msgs::msg::Float32 offset_msg;
  offset_msg.data = static_cast<float>(result.center_offset_m);
  offset_pub_->publish(offset_msg);

  if (publish_debug_image_ && !result.debug_image.empty()) {
    auto debug_msg = cv_bridge::CvImage(msg->header, "bgr8", result.debug_image).toImageMsg();
    debug_image_pub_->publish(*debug_msg);
  }
}

}  // namespace lane_detection
