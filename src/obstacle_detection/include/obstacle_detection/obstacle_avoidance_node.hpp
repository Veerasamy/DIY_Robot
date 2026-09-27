#pragma once

#include <mutex>

#include <opencv2/core.hpp>

#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "obstacle_detection/vfh_planner.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/float32.hpp"
#include "std_msgs/msg/u_int8.hpp"

namespace obstacle_detection
{

class ObstacleAvoidanceNode : public rclcpp::Node
{
public:
  ObstacleAvoidanceNode();

private:
  void onLaserScan(const sensor_msgs::msg::LaserScan::ConstSharedPtr & msg);
  void onDepthImage(const sensor_msgs::msg::Image::ConstSharedPtr & msg);
  void onOdom(const nav_msgs::msg::Odometry::ConstSharedPtr & msg);
  void onControlTimer();
  void publishCostmap(const std::vector<ObstaclePoint> & points, const rclcpp::Time & stamp);

  // Wall detection (Section F follow-up): a single VFH valley can still
  // route through a gap that's too narrow to actually be passable in
  // practice -- explicitly check the front sector plus both side sectors so
  // a dead-end/wall (all three blocked at once) always forces a full stop
  // rather than relying solely on the histogram-based valley search.
  struct SectorClearance { double front_m; double left_m; double right_m; };
  SectorClearance computeSectorClearance(const std::vector<ObstaclePoint> & points) const;
  // Negative obstacle (track edge / cliff / table drop) detection: unlike
  // computeSectorClearance (which only looks at points ABOVE the expected
  // ground plane), this scans the near-ground row band for pixels where the
  // ground unexpectedly reads farther away (or returns nothing) than a flat
  // floor would -- meaning it has dropped away. Returns +inf if no drop
  // found within max_range_m.
  double computeCliffDistance(const cv::Mat & depth) const;

  std::vector<ObstaclePoint> depthImageToPoints(const sensor_msgs::msg::Image & msg) const;

  std::unique_ptr<VfhPlanner> planner_;

  std::mutex data_mutex_;
  std::vector<ObstaclePoint> lidar_points_;
  std::vector<ObstaclePoint> depth_points_;
  double current_speed_mps_{0.0};

  // Depth-camera-to-2D projection parameters (Section F: depth analysis).
  // Simplified pinhole + planar-tilt approximation — sufficient for coarse
  // obstacle gating at VFH's resolution. For higher-fidelity 3D transforms,
  // replace with a tf2 lookup against a calibrated base_link->camera static
  // transform (documented as a follow-up refinement in the package README).
  double camera_hfov_rad_{1.361};   // ZED 2i 2.1mm lens, ~78 deg horizontal at HD720
  double camera_vfov_rad_{1.204};   // ~69 deg vertical at HD720
  double camera_mount_height_m_{0.195};  // measured: camera+LiDAR mounted 19-20cm above ground
  double camera_tilt_rad_{0.0};     // positive = tilted downward from horizontal
  double ground_clearance_min_m_{0.03};  // measured car ground clearance is 4cm -- stay below it so anything at/above that height is still flagged as an obstacle
  double obstacle_height_max_m_{0.25};   // measured car height is 190-202mm -- ignore only returns clearly above that (true overhangs), plus margin
  int depth_sample_stride_px_{8};        // subsample columns/rows for speed

  double braking_decel_mps2_{6.0};       // conservative tire-limited deceleration estimate
  double braking_margin_m_{0.3};

  // Wall detection thresholds
  double wall_check_distance_m_{1.2};      // front clearance below this triggers a side check
  double wall_side_clear_min_m_{0.5};      // side clearance below this counts as "also blocked"
  double wall_front_half_angle_rad_{0.26}; // +/-15 deg forward cone
  double wall_side_min_angle_rad_{0.35};   // side sectors start here (~20 deg off-axis)
  double wall_side_max_angle_rad_{1.5708}; // ... out to +/-90 deg

  // Cliff / negative-obstacle detection thresholds
  double cliff_drop_max_m_{0.15};       // ground reading beyond expected+this -> "missing" (dropped away)
  double cliff_check_distance_m_{1.5};  // cliff distance below this -> BRAKE, same as a wall
  double last_cliff_distance_m_{1e6};

  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr depth_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr avoidance_cmd_pub_;
  rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr costmap_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr state_request_pub_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr wall_detected_pub_;
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr front_clearance_pub_;
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr left_clearance_pub_;
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr right_clearance_pub_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr cliff_detected_pub_;
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr cliff_distance_pub_;
  rclcpp::TimerBase::SharedPtr control_timer_;
};

}  // namespace obstacle_detection
