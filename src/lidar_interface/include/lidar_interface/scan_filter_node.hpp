// Thin wrapper/config around the community `rplidar_ros` driver (see
// ARCHITECTURE.md section 7 — this package intentionally does not
// reimplement the RPLiDAR USB/serial protocol). Subscribes to the vendored
// driver's raw scan topic and republishes a cleaned `/scan` matching the
// topic contract `obstacle_detection` expects: mounting-bracket blind-spot
// masking, frame_id normalization, and clamping the reported range_min/max
// to the physically valid sensor envelope in case the driver's defaults
// don't match the exact RPLiDAR model in use.
#pragma once

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

namespace lidar_interface
{

class ScanFilterNode : public rclcpp::Node
{
public:
  ScanFilterNode();

private:
  void onScan(const sensor_msgs::msg::LaserScan::ConstSharedPtr & msg);

  double range_min_m_{0.15};   // RPLiDAR A2/A3 minimum valid range — confirm for your exact model
  double range_max_m_{12.0};   // RPLiDAR A2/A3 maximum valid range — confirm for your exact model
  double blind_spot_start_rad_{0.0};  // start of the mounting-bracket occlusion sector
  double blind_spot_end_rad_{0.0};    // end of the mounting-bracket occlusion sector (start==end disables)
  std::string frame_id_override_;     // empty = keep the driver's frame_id

  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
  rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr scan_pub_;
};

}  // namespace lidar_interface
